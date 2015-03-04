/*
 * VisualizationView.cpp
 *
 *  Created on: 25.04.2014
 *      Author: philipp
 */

#define QSTR_EQUAL 0

#include "VisualizationView.h"
#include "ControlPanel.h"
//#include "TopologyCalculator.h"

#include <qt5/QtWidgets/QtWidgets>
#include <qt5/QtGui/QtGui>
//#include <QWidget>
//#include <QMainWindow>
//#include <QApplication>
//#include <QLabel>
#include <QRect>
//#include <QStatusBar>
#include <QTextStream>
//#include <QToolBar>
//#include <QAction>
#include <QFile>
#include <QString>
#include <QPainter>
//#include <QFileDialog>
//#include <QInputDialog>
//#include <QLabel>
#include <QTime>
#include <QTimer>

#include <cmath>
#include <stdlib.h>
#include <iostream>
#include <math.h>


int VisualizationView::amount=0;
/******************************************************************
 * CONSTRUCTOR
 ************************************************************/
VisualizationView::VisualizationView(QMainWindow* parent)
  : QWidget(parent)
{
  //QString weightsFileName("weights10.txt");
  //readWeightsFromFile(weightsFileName);
  VisualizationView::amount++;
  std::cout << amount << " VVs are active." << std::endl;
  QString defaultMode = "linear";
  inSize = 0;
  outSize = 0;
  size = 0;
  activities.resize(1); //1 hypothetical timestep
  activities[0].resize(size+1); //1 hypothetical neuron (size is set to 0 above)
  sizeOfLayer.clear();

  topologyAligned = false;
  positionsFixed = false; //neurons' positions might be changed on first initiation of necessarity
  synapseLabelsShown = false; //hide labels
  neuronLabelsShown = false;  //no neuron Labels
  activityTimerIsOn = false;  //no activity blinking
  drawArea = 0.7; //percentage of area filled with network relevant drawings
  currentTime = 0;
  maxTime = 1;
  toNeuron = 0;
  debug = true; //true: activates debugging outstream lines

  this->parent = parent;
  setMode(defaultMode);
  amod = "radius";
  ndiameter = 10;

  synapseLabels = new QLabel[size*size];
  neuronLabels = new QLabel[size];
  //TODO: add something like: neuronLabels = new QLabel[size+insize+outsize];
  cp = new ControlPanel(this);  //cp of this VV instance (networkish global) control panel

  /* testing purpose:
  QLabel* testLabel = new QLabel("Test Label");
  testLabel->setParent(this);
  testLabel->setGeometry(QRect(200,30,100,20));
  //this->resize(800,800);
  */
  parent->statusBar()->showMessage("VisualizationView initiated.");

  QTextStream terminal(stdout);
  //ControlPanel* cp = new ControlPanel(this);
  terminal << "Refresh Parameters..." << endl;
  refreshParameters();
  terminal << "Update control panel..." << endl;

  cp->update(controlledParameters); //TODO: this line is buggy

  startTimer(500); //starts every 0.5 seconds the VisualizationView::timerEvent(QTimerEvent* e) function

  /*
  inQStream = new QTextStream(stdin, QIODevice::ReadOnly);
  outQStream = new QTextStream(stdout);
  QTimer* pipeTimer = new QTimer(this);
  connect(pipeTimer, SIGNAL(timeout()), this, SLOT(hearkenTheStdin()));//SLOT(readThePipe()));
  pipeTimer->start(100);
  inchar = '1';
  */
}

VisualizationView::~VisualizationView() {
  cp->close();
  //pclose(instream);
  //cp->quit();
  VisualizationView::amount--;
}


//Reading functions:
/* readSizeFromFile(name of file)
 * sets <size> to n, when the file represents a n x n matrix.
 */
void VisualizationView::readSizeFromFile(QString filename) { //assumes no IN/OUT neurons
  QString firstLine;
  QFile data(filename);
  int tempSize = 1;
  if( data.open(QFile::ReadOnly)) {
    QTextStream read(&data);
    firstLine = read.readLine();
    foreach (QChar qc, firstLine) {
      if(qc.isSpace()) {
        tempSize++;
      }
    }
    size = tempSize;
    inSize = 0;
    outSize = 0;
  }
}

int VisualizationView::readLinelengthOfFile(QString filename) {
  QFile data(filename);
  int nlines = 0;
  if(data.open(QFile::ReadOnly)) {
    QTextStream countstream(&data);
    QString buffer = countstream.readLine();
    for(nlines = 1; !buffer.isNull(); nlines++) {
      buffer=countstream.readLine();
    }
    data.close();
  }
  return nlines;
}

/*  PUBLIC readWeightsFromFile(name of file)
 *  sets weightMatrix[] to a size * size long array, containing the weights of the file
 */
void VisualizationView::readWeightsFromFile(QString filename) {
  if( mode == "esn" ) {
    readEsnFromFile(filename);
  } else {

    QTextStream terminal(stdout);
    readSizeFromFile(filename);
    //initiateInAndOut();
    //terminal << size << "\n";
    weightMatrix = new double[size*size];
    QString wline;
    QFile data(filename);
    if(data.open(QFile::ReadOnly)) {
      QTextStream read(&data);
      int i=0;
      do {  //walk linewise through file
        wline = read.readLine();  //read line
        //terminal << wline << endl;  //check reading
        int j=0;  //count # of read floats
        QString temp; //that are previously read as QString
        for ( int c=0;c<=wline.length();c++) { //walk char-wise through line
          if(!wline[c].isSpace() && !wline[c].isNull()) {
            temp.append(wline[c]);
            //terminal << wline[c] << " read to temp" <<endl;
          } else {
            if(!temp.isEmpty()) {
              weightMatrix[size*i+j] = temp.toFloat();
              //terminal << temp <<" written to weights[][]"<< endl;
              temp.clear();
              j++;
            }
          }
        }
        i++;
      } while ( !wline.isNull());//end reading file
      controlledParameters.networkFile = filename;
    }
    clearActivities();
    terminal << writeWeights(); //debug purpose: display read weights
    // missing destructor before redefining: ~synapseLabels;
    synapseLabels = new QLabel[size*size];
    neuronLabels = new QLabel[size];
  }
  //parent->statusBar()->showMessage("The data of file "+filename+" is now visualized.");
}

/*
 * esn reading
 */
void VisualizationView::readEsnFromFile(QString filename) {
  mode = "esn";
  QFile esnFile(filename);
  if(esnFile.open(QFile::ReadOnly)) {
    QTextStream inStream(&esnFile);

    //first line should contain: SIZE<space>INSIZE<space>OUTSIZE as integer
    //read first line:
    inStream >> size;
    inStream >> inSize;
    inStream >> outSize;

    //read rest of file
    int sout = size + outSize;
    int sin = size + inSize;
    esnMatrix = new double[sout*sin];
    for(int i = 0; i < sout; i++) {
      inStream.readLine();
      for(int j = 0; j<sin; j++) {
        inStream >> esnMatrix[sin*i+j];
      }
    }
    controlledParameters.networkFile = filename;
  }
}


/*
 * activity reading
 */
void VisualizationView::readActivitiesFromFile(QString filename, int startLine, int endLine) {
  //reset timer values:
  setActivityTimerIsOn(false);
  currentTime = 0;
  maxTime = 1;

  //detect size, that is: number of neurons
  int currentSize = size;
  readSizeFromFile(filename);
  QTextStream terminal(stdout); //debug purpose

  if(size == currentSize) { //if proper amount of neuron's activities is given do stuff
    //adjust 2d array "activities":
    activities.resize(endLine);
    for(int t = 0; t < endLine; t++) {
      activities[t].resize(size);
    }
    maxTime = endLine;
    //read file:
    QFile activityFile(filename);
    if(activityFile.open(QFile::ReadOnly)) {
      QTextStream inStream(&activityFile);
      for(int t = 0; t<endLine; t++) {
        for(int i = 0; i<size; i++) {
          inStream >> activities[t][i];
          if( isNaN( (double) activities[t][i]) ) {
            activities[t][i] = 0;
          }
        }
        inStream.readLine();
      }
      //for debug purpose: write in terminal read data:
      terminal << endl << "Read activities:" << endl;
      for(int t = 0; t<endLine; t++) {
        for(int i = 0; i<size; i++) {
          terminal << activities[t][i] << " ";
        }
        terminal << endl;
      }
      //replace all NaNs with 0:
      /*for(int t = 0; t<endLine; t++) {
        for(int i = 0; t< size; i++) {

        }
      }*/

    }//end file reading

    //start activity display via
    setActivityTimerIsOn(false);
    parent->statusBar()->showMessage("Activity file successfully read (" + filename + ").");
    controlledParameters.activityFile = filename;
    //which causes timerEvent to trigger repainting and time incrementing

  } else {  //if sizes of network and activities do not match, do nothing
    size = currentSize;
    setActivityTimerIsOn(false);
    parent->statusBar()->showMessage("Network size mismatch.");
  }
}

/*
 * readLeftDouble returns the first (most left) double of a string and cuts the string to the remaining content
 */
double VisualizationView::readLeftDouble(QString* s) {
  double r=0.0; //in spe returnable double
  QString temp_qstring;
  int c;
  for(c = 0; !((*s)[c].isSpace()) && !((*s)[c].isNull());c++) {  //runs through s' characters via s[c]
      temp_qstring.append( (*s)[c]);
  }
  r = temp_qstring.toFloat();
  //delete temp_qstring;
  (*s).remove(0,c+1);
  return r;
}

/*
 * initiateInAndOut()
 */
void VisualizationView::initiateInAndOut() {
  inMatrix = new double[size*size];
  outMatrix = new double[size*size];
  for(int i = 0; i < size; i ++) {
    for(int j = 0; j < size; j++) {
      inMatrix[i*size+j] = outMatrix[i*size+j] = 0.0;  //all entries are zero of both matrices
    }
  }
}


/*  writeWeights()
 *  returns the current weightMatrix values as matrix string.
 */
QString VisualizationView::writeWeights() {
  //write weights from weightMatrix[]
  QString returnString="";
  QString temp;
  for(int i=0;i<size;i++) {
    for(int j=0;j<size;j++) {
      returnString += temp.setNum(weightMatrix[size*i+j]);
      returnString += " ";
    }
    returnString.chop(1);
    returnString += "\n";
  }
  return returnString;
}

/*  =================
 * paintEvent(some event)
 * is called on this.show() and on other times
 * draws the current weightMatrix in chosen mode
 * ==================
 */
void VisualizationView::paintEvent(QPaintEvent* event) {
  Q_UNUSED(event);
  QPainter qp(this);
  drawNeuronNetworkFromWeightMatrix(&qp, mode);
  drawGrid(&qp);
  /*QRect* rect = new QRect(10,10,20,20);
  QImage image;
  image.load("pic.png");
  qp.drawImage(*rect,image);
  image drawing*/
  if(debug) std::cout << "A paint event happened.\n"; //debug purpose

  //refresh control panel on each draw event:
  refreshParameters();
  cp->update(controlledParameters);

}

/*
 * timer Event for permanent activity refreshing
 */
void VisualizationView::timerEvent(QTimerEvent* e) {
  Q_UNUSED(e);

  //Activity Timer
  if(activityTimerIsOn) {
    currentTime++;
    currentTime = currentTime % maxTime;
    repaint();
  }

}

/*  drawNeuronNetworkFromWeightMatrix
 *  uses current weightMatrix to visualize it
 */
void VisualizationView::drawNeuronNetworkFromWeightMatrix(QPainter* painter, QString mode) {
  //Painter and Pen default settings:
  QPen pen(Qt::black, 1, Qt::SolidLine);
  painter->setPen(pen);
  painter->setBrush(QBrush("#0000FF"));
  painter->setOpacity(0.5);
  painter->setRenderHint(QPainter::Antialiasing);
  //read window width and height:
  int h = height();
  int w = width();

  //for each synapse one label:


  //draw network depending on mode
  //linear mode drawing
  if( mode == "linear") {
    //set coordinates zero to the center of window:
    painter->translate(QPoint(w/2,h/2));

    //draw horizontal line of circles
    for(int i=0;i<size;i++) {
      painter->drawEllipse(-w/(2*size)*(size-1)+w/size*i,0,10,10);

      //draw neuron labels:
      if(neuronLabelsShown) {
        neuronLabels[i].setParent(this);
        neuronLabels[i].setGeometry(QRect(-w/(2*size)*(size-1)+w/size*i,-30,30,20));
        neuronLabels[i].setText(QString("#%1").arg(i));
      }
    }
    //draw connections
    //connections go to up left or bottom right when OUT | comes from top right or bottom left when IN
    int direction=1;
    for( int i = 0;i<size;i++) {
      for( int j = 0; j<size; j++){
        if( weightMatrix[size*i+j]!= 0) {
          pen.setWidthF(2.0*(weightMatrix[size*i+j]>0? weightMatrix[size*i+j]:-weightMatrix[size*i+j]));//(double)abs((double)weightMatrix[size*i+j]));
          direction = i<j ? 1 : -1;
          weightMatrix[size*i+j]<0? pen.setColor(Qt::red) : pen.setColor(Qt::black);
          painter->setPen(pen);
          painter->drawArc(-w/(2*size)*(size-1)+w/size*i+5,-28-5*abs(j-i),w/size*(j-i),60+10*abs(j-i),0,direction*16*180);
          //draw label and circle at end:
          /*
          pen.setWidth(5);
          painter->setPen(pen);
          painter->drawPoint(-w/(2*size)*(size-1)+w/size*i+5+3*direction,-5*direction+5);
          */
          if(synapseLabelsShown) {
            //std::cout<< "Display a label...\n";
            synapseLabels[size*i+j].setParent(this);
            synapseLabels[size*i+j].setGeometry(QRect(w/(2*size)*(size-1)+w/size*i+5,28+5*abs(j-i),50,20));
            synapseLabels[size*i+j].setText(QString("%1").arg(weightMatrix[size*i+j]));
            //synapseLabels[size*i+j].show();
          }

        }
      }
    }
    painter->translate(QPoint(-w/2,-h/2));
  }

  //random or ESN or top or ff mode drawing:
  if( mode == "random" || mode == "esn" || mode == "top" || mode == "ff") {

    //calculate feed forward positions:
    if(mode =="ff") {
      if(!positionsFixed) {
        determineFfPositions();
        positionsFixed = true;
      }
    }


    //generate random positions, if not done yet:
    if(!positionsFixed) {
      xPositionOfNeuron = new double[size];
      yPositionOfNeuron = new double[size];
      srand(500);//time(NULL));
      for(int i = 0; i < size ; i++) {
        xPositionOfNeuron[i] = (double) rand() / (double)RAND_MAX;
        yPositionOfNeuron[i] = (double) rand() / (double)RAND_MAX;
      }
      if(mode != "top") {
        positionsFixed = true;  //TODO check the necessarity of this line
      }
    }

    //calculate topology, if top mode is chosen:

      if(mode =="top" && !topologyAligned) {
        topCalc = new TopologyCalculator(xPositionOfNeuron, yPositionOfNeuron, weightMatrix, size);

        //integrate forces on previously randomly positions:
        int step_MAX = 10000;
        for(int step = 0; step < step_MAX; step ++) {
          topCalc->integrateSingleStep_Euler();
        }
        //topCalc->normPositions();

        //printPositions();

        xPositionOfNeuron = topCalc->get_xpositions();
        yPositionOfNeuron = topCalc->get_ypositions();
        //printPositions();


        topologyAligned = true;
        positionsFixed = true;

      }



    //in case of ESN: draw in and out matrix in background (first)
    if(mode == "esn") {
      drawInAndOut(painter);
    }

    //draw synaptic connections:
    for(int i = 0; i < size ; i++) {
      for(int j = 0; j < size ; j++) {
        if(weightMatrix[size*i+j]!=0) {
          drawSynapseArc(painter, i, j, synapseLabelsShown);
        }
      }
    }

    //draw activity dependent circles at neuron positions
    //respect the AMOD option
    if(amod == "color") {
      double temp_squaredActivity = 0;
      QColor brushcolor(0,0,255);
      double diameter = ndiameter;
      for(int i = 0; i < size; i++) {
        temp_squaredActivity = pow((double) activities[currentTime][i],2);
        brushcolor = QColor((int)(255*temp_squaredActivity), 0 , (int)(255*(1-temp_squaredActivity)));
        painter->setBrush(QBrush(brushcolor));

        painter->drawEllipse(drawArea*(w*(xPositionOfNeuron[i])-0.707106*diameter)+0.5*(1-drawArea)*w,drawArea*(h*(yPositionOfNeuron[i])-0.707106*diameter)+0.5*(1-drawArea)*h,diameter,diameter);
        //add neuron labels
        if(neuronLabelsShown) {
          neuronLabels[i].setParent(this);
          neuronLabels[i].setGeometry(drawArea*(w*(xPositionOfNeuron[i])-0.707106*diameter)+0.5*(1-drawArea)*w,drawArea*(h*(yPositionOfNeuron[i])-0.707106*diameter)+0.5*(1-drawArea)*h-(20+0.5*diameter),30,20);
          neuronLabels[i].setText(QString("#%1").arg(i));
        }
      }

    } else { if(amod == "opacity") {
      QColor brushcolor(255,0,0);
      painter->setBrush(QBrush(brushcolor));
      double diameter = ndiameter;
      double opacityfactor = 0;
      for(int i = 0; i < size; i++) {
        opacityfactor = pow((double) activities[currentTime][i],2);
        painter->setOpacity(opacityfactor);
        painter->drawEllipse(drawArea*(w*(xPositionOfNeuron[i])-0.707106*diameter)+0.5*(1-drawArea)*w,drawArea*(h*(yPositionOfNeuron[i])-0.707106*diameter)+0.5*(1-drawArea)*h,diameter,diameter);
        //add neuron labels
        if(neuronLabelsShown) {
            neuronLabels[i].setParent(this);
            neuronLabels[i].setGeometry(drawArea*(w*(xPositionOfNeuron[i])-0.707106*diameter)+0.5*(1-drawArea)*w,drawArea*(h*(yPositionOfNeuron[i])-0.707106*diameter)+0.5*(1-drawArea)*h-(20+0.5*diameter),30,20);
            neuronLabels[i].setText(QString("#%1").arg(i));
        }
      }
      painter->setOpacity(1);
    } else { if(amod == "radius" || true) {
      double diameter = 0;
      for(int i = 0; i < size ; i++) {
        diameter = ndiameter * pow((double) activities[currentTime][i],2)+1.0;
        painter->drawEllipse(drawArea*(w*(xPositionOfNeuron[i])-0.707106*diameter)+0.5*(1-drawArea)*w,drawArea*(h*(yPositionOfNeuron[i])-0.707106*diameter)+0.5*(1-drawArea)*h,diameter,diameter);
        //add neuron labels
        if(neuronLabelsShown) {
          neuronLabels[i].setParent(this);
          neuronLabels[i].setGeometry(drawArea*(w*(xPositionOfNeuron[i])-0.707106*diameter)+0.5*(1-drawArea)*w,drawArea*(h*(yPositionOfNeuron[i])-0.707106*diameter)+0.5*(1-drawArea)*h-(20+0.5*diameter),30,20);
          neuronLabels[i].setText(QString("#%1").arg(i));
        }
      }
      if(amod != "radius") {
        parent->statusBar()->showMessage("Warning: no proper activity display mode (AMOD) has been selected, using radius by default.");
      }
    }}}  //end else-if



  } //end random, esn, top, ff


  //painter->drawArc(0,0,100,50,0,16*180);
  //map drawing
  if( mode == "map") {
    xPositionOfNeuron = new double[size];
    yPositionOfNeuron = new double[size];
    int sizeroot = (int) sqrt(size);
    for(int i = 0; i < size; i++) {
      xPositionOfNeuron[i] = (i % sizeroot) * (double) 1/(double) (sizeroot+1);
      yPositionOfNeuron[i] = (i / sizeroot) * (double) 1/(double) (sizeroot+1);
    }


    //draw neurons:
    //double diameter = 10.0; //TODO: check if this is necessary
    QColor brushcolor(0,0,255);  //recolor via: brushcolor = QColor(0,255,0);

    double mapSize = drawArea * (w < h? w : h); //squared field with mapSize of drawArea*100% of smaller edge of window
    double neuronPixelSize = (double) 1/(double) (sizeroot+1);
    painter->setOpacity(0.8);
    painter->setBrush(QBrush(brushcolor));
    double temp_squaredActivity = 0;
    for(int i = 0; i < size; i++ ) {
      //painter->drawRect(drawArea*(w*(xPositionOfNeuron[i])-0.5*diameter)+0.5*(1-drawArea)*w,drawArea*(h*(yPositionOfNeuron[i])-0.5*diameter)+0.5*(1-drawArea)*h, diameter, diameter);
      temp_squaredActivity = pow((double) activities[currentTime][i],2);
      brushcolor = QColor((int)(255*temp_squaredActivity), 0 , (int)(255*(1-temp_squaredActivity)));
      painter->setBrush(QBrush(brushcolor));

      painter->drawRect(mapSize*xPositionOfNeuron[i]+0.5*(1-drawArea)*w, mapSize*yPositionOfNeuron[i]+0.5*(1-drawArea)*h, neuronPixelSize*mapSize, neuronPixelSize*mapSize);
    }

    //draw legend:
    painter->setOpacity(1);
    for(int k = 0; k<255; k++) {
      brushcolor = QColor(k,0,255-k);
      pen.setColor(brushcolor);
      painter->setPen(pen);
      painter->setBrush(QBrush(brushcolor));

      double xStart, xEnd, yStart, yEnd;
      yStart = yEnd = h*drawArea*(1.0-(double)k/255.0) + 0.5*(1-drawArea)*h;
      xStart = 0.5*w*(1+drawArea);
      xEnd = xStart + (1-drawArea)*0.5*w;
      //painter->drawLine(0.5*w*(1+drawArea),h*drawArea*(1.0-255.0*(double)k/255.0) + 0.5*(1-drawArea)*h,0.5*w*(1+drawArea)+0.1*0.5*(1-drawArea),h*drawArea*(1.0-255.0*(double)k/255.0) + 0.5*(1-drawArea)*h);//0.25*(1-drawArea)*w,drawArea*h/255.0);
      painter->drawLine(xStart,yStart,xEnd,yEnd);
    }


  }//endif mode = map



}

/* drawInAndOut(painter*)
 *
 */
void VisualizationView::drawInAndOut(QPainter* painter) {

  QPen pen(Qt::black, 1, Qt::DotLine);
  painter->setPen(pen);
  //draw in and out neurons:
  int w = width();
  int h = height();
  int sout = size + outSize;
  int sin = size + inSize;

  pen.setStyle(Qt::DashLine);//DashLine
  pen.setColor(Qt::green);
  painter->setPen(pen);
  //painter->setOpacity(0.5);
  //draw in and out connections:
  //first: IN
  for(int i = 0; i < size; i++) { //i indicates TO
    for(int j = size; j < sin; j++) { //j indicates FROM
      if(esnMatrix[sin*i+j] != 0) {
        painter->drawLine((1-drawArea)*0.25*w,
            0.5*(1-drawArea)*h+drawArea*h/inSize*(j-size),
            drawArea*xPositionOfNeuron[i]*w+0.5*(1-drawArea)*w,
            drawArea*yPositionOfNeuron[i]*h+0.5*(1-drawArea)*h);
      }
    }
  }
  //second: OUT
  for(int i = size; i< sout; i++) {
    for(int j = 0; j < size; j++) {
      if(esnMatrix[sin*i+j] != 0) {
        painter->drawLine(drawArea*xPositionOfNeuron[j]*w+0.5*(1-drawArea)*w,
            drawArea*yPositionOfNeuron[j]*h+0.5*(1-drawArea)*h,
            (1-drawArea)*0.75*w+drawArea*w,
            0.5*(1-drawArea)*h+drawArea*h/outSize*(i-size));
      }
    }
  }

  //draw in and out neurons

  pen.setStyle(Qt::SolidLine);
  pen.setColor(Qt::black);
  painter->setPen(pen);
  for(int i = 0; i < inSize; i++) {
    painter->drawEllipse((1-drawArea)*0.25*w-5,
        0.5*(1-drawArea)*h+drawArea*h/inSize*i-5,
        10,10);
  }
  for(int i = 0; i< outSize; i++) {
    painter->drawEllipse((1-drawArea)*0.75*w+drawArea*w-5,
        0.5*(1-drawArea)*h+drawArea*h/outSize*i-5,
        10,10);
  }
}

/*drawGrid
 * draws a dotted square grid onto the window with 10px grid constant
 */
void VisualizationView::drawGrid(QPainter* painter) {
  int h = height();
  int w = width();
  painter->translate(QPoint(w/2,h/2));
  QPen pen(Qt::black, 1, Qt::SolidLine);
  painter->setPen(pen);
  for(int i = -w/2;i<w/2;i=i+10) {
    for(int j = -h/2;j<h/2;j=j+10) {
      painter->drawPoint(i,j);
    }
  }
}

/*  drawSynapseArc(painter pointer, index of 1. neuron, index of 2. neuron)
 * INTENDED: to draw arc from 1. to 2.
 * DOES: draw line from 1. to 2.
 */
void VisualizationView::drawSynapseArc(QPainter* painter, int i, int j, bool drawWithLabel) {
    //i is TO and j is FROM
  double swidth = weightMatrix[size*i+j];  //save weight of synapse
  //set up the pen:
  QPen pen(Qt::black, 0, Qt::SolidLine);  //initiate pen
  pen.setWidthF(2.0*fabs(swidth));//(swidth>0? swidth : -swidth)); //scale width of pen with weight
  //change color depending on sign of weight:
  QColor brushcolor;
  //painter->setOpacity(0.9);
  if(swidth < 0) {
      pen.setColor(Qt::red);
      brushcolor = QColor(255,0,0);
  } else {
      pen.setColor(Qt::black);
      brushcolor = QColor(0,0,0);
  }

  //int direction = i<j? 1 : -1;  //drawing direction of arc (mathematical sense)
  painter->setBrush(QBrush(brushcolor));
  painter->setPen(pen);
  int w = width();
  int h = height();
  //for shorter notation:
  double xposi = xPositionOfNeuron[i];
  double yposi = yPositionOfNeuron[i];
  double xposj = xPositionOfNeuron[j];
  double yposj = yPositionOfNeuron[j];
  /* arc positioning... doesn't work at the moment:
  *double deltax = xposj - xposi;
  *double deltay = yposj - yposi;

  *double length = pow(w*w*deltax*deltax+h*h*deltay*deltay,0.5);
  *double alpha = atan((double) deltay/(double) deltax);
  *std::cout << alpha << " " << length << "\n";
  *painter->rotate(360/(2*M_PI)* alpha);
  *painter->drawArc(w*(cos(alpha)*xposi+sin(alpha)*yposi),h*(-sin(alpha)*xposi+cos(alpha)*yposi),length, 42, 0, direction*16*180);
  *painter->rotate(-360/(2*M_PI)* alpha);
  */
  double xstart,ystart,xend,yend;
  xstart = drawArea*w*xposi+0.5*(1-drawArea)*w;
  ystart = drawArea*h*yposi+0.5*(1-drawArea)*h;
  xend = drawArea*w*xposj+0.5*(1-drawArea)*w;
  yend = drawArea*h*yposj+0.5*(1-drawArea)*h;

  if(i == j) {
    /*double startAngle = -150;
    double drawAngle = 300;
    painter->drawArc(xstart+15,ystart,30,16*startAngle,16*drawAngle);
    */
    drawSelfConnect(painter, i);  //, drawWithLabel);
  } else {
    painter->drawLine(drawArea*w*xposi+0.5*(1-drawArea)*w,drawArea*h*yposi+0.5*(1-drawArea)*h,drawArea*w*xposj+0.5*(1-drawArea)*w,drawArea*h*yposj+0.5*(1-drawArea)*h);
    //add small circle at end of connection:
    double weightsize = 0.5* ndiameter * sqrt(fabs(swidth));
    double angle = atan( (yposi-yposj) / (xposi-xposj));
    if(xposj>xposi){ angle += M_PI;}
    //double dist = sqrt( pow(xend - xstart,2) + pow(yend - ystart,2) );
    //double xshift = cos(angle)*dist;
    //double yshift = sin(angle)*dist;
    painter->setOpacity(0.8);
    painter->drawEllipse(drawArea*w*xposi+0.5*(1-drawArea)*w -0.5*weightsize - cos(angle)*weightsize,drawArea*h*yposi+0.5*(1-drawArea)*h -0.5*weightsize - sin(angle)*weightsize,weightsize,weightsize);
    painter->setOpacity(0.5);
  }

  if(drawWithLabel) {
    synapseLabels[size*i+j].setGeometry( QRect(xstart+0.3*(xend-xstart),ystart+0.3*(yend-ystart),50,20) );
    synapseLabels[size*i+j].setText(QString("%1").arg(swidth));
  }

  //reset stuff:
  brushcolor = QColor(0,0,255);
  painter->setBrush(QBrush(brushcolor));
}

void VisualizationView::drawSelfConnect(QPainter* painter, int i) {//,bool drawWithLabel) {
  double xpos, ypos, startAngle, drawAngle;
  double w = width();
  double h = height();
  xpos = w*(xPositionOfNeuron[i]*drawArea+0.5*(1-drawArea));
  ypos = h*(yPositionOfNeuron[i]*drawArea+0.5*(1-drawArea));
  startAngle = -150;
  drawAngle = 300;
  painter->drawArc(xpos,ypos-15,30,30,16*startAngle,16*drawAngle);
}

/*  PUBLIC setMode(string of new mode)
 *  sets mode to the new chosen mode
 */
void VisualizationView::setMode(QString newMode) {
  mode = newMode;
  QString statusAlert = "Selected display mode: "+mode+".";
  parent->statusBar()->showMessage(statusAlert);

  //reset values:
  resetValues();  //resets boolean values
}

/*
 * PUBLIC setActivityTimerIsOn, switches activityTimerIsOn (bool) to selected state
 */
void VisualizationView::setActivityTimerIsOn(bool state) {
  activityTimerIsOn = state;
}
/*
 *PUBLIC writeRandomWeightsToFile(size of desired matrix)
 */
void VisualizationView::writeRandomWeightsToFile(int N) {
  srand(time(NULL));
  double randNumber;
  int zero = 0;
  QString stringN;
  stringN = stringN.setNum(N);
  QString nameOfFile = "weights" + stringN + ".txt";
  QFile data(nameOfFile);
  if(data.open(QFile::WriteOnly)) {
    QTextStream out(&data);
    for(int i=0; i<N; i++) {
      for(int j=0; j<N; j++) {
        randNumber = 1-2*(double) rand() / (double)RAND_MAX;
        //diagonals and 10% are zero
        if( i == j || (randNumber < 0.1 && randNumber > -0.1)) {
          out << zero;
        } else {
          out << randNumber;
        }
        //space between numbers, none if at end
        if( j < N-1) {
          out << " ";
        } else {
          out << endl;
        }
      }
    }
    readWeightsFromFile(nameOfFile);  //testing purpose
  }
}

/*
 * writeRandomActivitiesToFile(size,times)
 * function to generate sample activities
 */
void VisualizationView::writeRandomActivitiesToFile(int numberNeurons, int numberTimesteps) {
  //srand(time(NULL));
  //double randNumber;

  //generate file:
  QString stringNumberNeurons;
  stringNumberNeurons = stringNumberNeurons.setNum(numberNeurons);
  QString stringNumberTimesteps;
  stringNumberTimesteps = stringNumberTimesteps.setNum(numberTimesteps);
  QString nameOfFile = "activities_of" + stringNumberNeurons + "neurons_for" + stringNumberTimesteps + "frames.txt";
  QFile data(nameOfFile);
  if(data.open(QFile::WriteOnly)) {
    QTextStream out(&data);
    for(int timestep=0; timestep<numberTimesteps; timestep++){
      for(int i = 0; i < numberNeurons; i++) {
        //randNumber = 1 - 2*(double) rand() / (double) RAND_MAX;
        out << cos(i*timestep*0.1);
        if( i < numberNeurons - 1) {
          out << " ";
        } else {
          out << endl;
        }
      }//end of one row
    }//end of file writing loop
  } //end of file writing

  readActivitiesFromFile(nameOfFile, 0, numberTimesteps); //debug purpose: test call of activity display
}
/*
 * others
 */
void VisualizationView::resetValues() {
  positionsFixed = false;
  topologyAligned = false;
  activityTimerIsOn = false;
  //neuronLabelsShown = false;
}

void VisualizationView::printPositions() {
  QTextStream terminal(stdout);
  for(int i = 0; i<size; i++) {
    terminal << "x:" << xPositionOfNeuron[i] << " y:" << yPositionOfNeuron[i] << endl;
  }
}

void VisualizationView::clearActivities() {
  activities.resize(1);
  activities[0].resize(size);
  for(int t= 0; t<1; t++) {
    for(int i = 0; i<size; i++) {
      activities[t][i] = 1;
    }
  }
  controlledParameters.activityFile = "N/A";
  maxTime = 1;
}

/*  =======================
 * SLOTS
 *  =======================
 */

void VisualizationView::fileToRead(QString file) {
  positionsFixed = false;
  if(synapseLabelsShown) {
    synapseLabelsToShow();
  }
  if(neuronLabelsShown) {
      neuronLabelsToShow();
  }
  readWeightsFromFile(file);
  repaint();
}

void VisualizationView::activityToRead(QString file) {
  //TODO function to determine line number
  parent->statusBar()->showMessage("activityToRead(QString) called.");
  int numberOfLines = 15; //TODO: soft code this!
  numberOfLines = readLinelengthOfFile(file);
  readActivitiesFromFile(file, 0, numberOfLines);
}

void VisualizationView::modeToApply(QString aMode) {
  setMode(aMode);
  parent->statusBar()->showMessage("Selected '" + aMode + "' as display mode.");
  repaint();
}

void VisualizationView::synapseLabelsToShow() {
  if(synapseLabelsShown) {
    for(int i = 0; i < size; i++) {
      for(int j = 0; j< size; j++) {
        synapseLabels[size*i+j].setParent(this);
        //synapseLabels[size*i+j].clear();
        //synapseLabels[size*i+j].hide();
        //synapseLabels[size*i+j].setText("");
        synapseLabels[size*i+j].setVisible(false);

      }
    }
    synapseLabelsShown = false;
    parent->statusBar()->showMessage("Synapse labels are hid.");
  } else {
    for(int i = 0; i < size; i++) {
      for(int j = 0; j< size; j++) {
        synapseLabels[size*i+j].setParent(this);
        synapseLabels[size*i+j].setVisible(true);
        synapseLabels[size*i+j].show();
      }
    }
    synapseLabelsShown = true;
    parent->statusBar()->showMessage("Synapse labels are shown now.");
  }
  repaint();
}

void VisualizationView::neuronLabelsToShow() {
    if(neuronLabelsShown) {//hide
        for(int i = 0; i < size; i++) {
            neuronLabels[i].setParent(this);
            neuronLabels[i].setVisible(false);
        }
    } else {//show
        for(int i = 0; i < size; i++) {
            neuronLabels[i].setParent(this);
            neuronLabels[i].setVisible(false);
            neuronLabels[i].show();
        }
    }
    neuronLabelsShown = !neuronLabelsShown;
    repaint();
}

void VisualizationView::activityTimerToRun() {  //switches activityTimer on / off
  activityTimerIsOn = activityTimerIsOn ? false : true ;
  QString sstate = activityTimerIsOn ? "activated." : "paused.";
  parent->statusBar()->showMessage("Activity display " + sstate);
}

void VisualizationView::controlPanelToShow() {
  cp->setIndexedTitle(vvindex);
  cp->show();
}

void VisualizationView::refreshParameters() {
  controlledParameters.mode = mode;
  controlledParameters.size = size;
  controlledParameters.t = currentTime;
  controlledParameters.positionState = positionsFixed;
  controlledParameters.insize = inSize;
  controlledParameters.outsize = outSize;
  controlledParameters.activityState = activityTimerIsOn;
  controlledParameters.labelsShown = synapseLabelsShown;
  controlledParameters.topologyAligned = topologyAligned;
  controlledParameters.amod = amod;
  controlledParameters.nlabelsShown = neuronLabelsShown;
}

void VisualizationView::readThePipe() {
  //std::cout << "VV: readThePipe was called." << std::endl;

  if(feof(instream)) {
    std::cout << "VV: the stream is over! =========================" << std::endl;
  }
  /*
  if(inchar != (char) 0) {
    std::cout << "VV: there should be a character! ooooooooooooooooooooooooooooooooo" << std::endl;
  }
  */
  std::cout << "VV(readThePipe):";
  while( !feof(instream) && inchar != '\n') { // '\0' represents empty char, also (char) 0 would
    inchar = fgetc(instream);
    std::cout <<  inchar ;
  }
  //std::cout << std::endl << "VV: readThePipe has finished!" << std::endl;

}

/* shifted to QVisualization.cpp:
void VisualizationView::hearkenTheStdin() {
  *outQStream << "Hearken to the stdin stream:" << endl;
  instring = inQStream->readLine();
  if(!instring.isNull()) {  //isNull() is operator to check if something is written into inQStream
    *outQStream << "Received command line:" << endl;
    *outQStream << instring << endl;
    *outQStream << "Taking action..." << endl;
    interpreteCommandLine(instring);
  }
}
*/

/*
 * PIPE FUNCTIONS + readThePipe, hearkenTheStdin above ( are a slots)
 */

void VisualizationView::hearkenTheStream(FILE* stream) {
  instream = stream;
}

/*
 * COMMAND LINE INTERPRETATION HERE
*/
void VisualizationView::interpreteCommandLine(QString commandLine) {
  debug = false;  //no debug out put for this function, debug=true at end
  //*outQStream << "Interpreting " << commandLine << " ..." << endl;
  QTextStream terminal(stdout);
  command = commandLine;
  if(debug) terminal << "Interpreting stuff\n";
  commandType = commandLine.left(4);
  command.remove(0,5);  //should remove the commandType from command Line
  if(debug) terminal << "commandType = " << commandType << ";left command = " << command << endl;

  //Take action depending on commandType:
  bool commandExecuted = false;
  //Change display mode:
  if (QString::compare(commandType, "MODE")  == QSTR_EQUAL && !commandExecuted ) {
    if(debug) terminal << "Taking action to change mode..." << endl;
    QString temp_mode = leftParameter(command);
    if(temp_mode == "ff") {
      determineLayerSizes(cutLeftParameter(command));

      this->modeToApply(temp_mode);
    } else {
      this->modeToApply(command);
    }
    commandExecuted = true;
  }

  //Quit VisualizationView:
  if (QString::compare(commandType, "QUIT")  == QSTR_EQUAL && !commandExecuted ) {
    terminal << "Quitting VisualizationView instance "<< vvindex << endl;
    //this->parent->qApp->quit();
    cp->close();
    this->close();
    commandExecuted = true;
  }

  //Read NeTWork:
  if (QString::compare(commandType, "RNTW")  == QSTR_EQUAL && !commandExecuted ) {
    if(debug) terminal << "VV: interprete RNTW, ";
    if(QString::compare(command.left(4),"file") == QSTR_EQUAL) {  //read from file
      if(debug) terminal << "VV: interprete file: with file named: ";
      command.remove(0,5);  //removes "file " from command
      if(debug) terminal << command << endl;
      this->fileToRead(command);  //TODO: overwork for 1 common reading function, this one is not suitable for ESN
      commandExecuted = true;
    }
    if(QString::compare(command.left(4),"data") == QSTR_EQUAL) {  //read from following data stream
      command.remove(0,5);  //removes "data " from command
      int testint = (int) readLeftDouble(&command);
      if(debug) terminal << testint << " was read" << endl;
      //replace weightMatrix with to-be-read-weightmatrix:
      //maybe add: if(testint.isDouble()) {...
      size = testint;
      //Reset matrix, labels and activities:
      weightMatrix = new double[size*size]; //check, if deletion of weightMatrix is necessary via ~weightMatrix
      synapseLabels = new QLabel[size*size];
      neuronLabels = new QLabel[size];
      clearActivities();
      for(int i = 0; i<size; i++) {
        //call a call for QV::deliverWeightLine, to be followed by VV::intepreteWeightLine
        toNeuron = i; //set toNeuron, that interpreteWeightLine knows, which neuron is the TO indicated one
        emit requireWeightLine(vvindex);  //cast to QVis slot deliverWeightLine(vvindex)

      }
      repaint();
      if(debug) terminal << "A new network has been read via pipe command:" << endl;
      if(debug) terminal << writeWeights();
      controlledParameters.networkFile = "streamed data";
      commandExecuted = true;
    }
  }

  //Write NeTWork
  if (QString::compare(commandType, "WNTW") == QSTR_EQUAL && !commandExecuted ) {

  }

  //read single ACTivity Line
  if (QString::compare(commandType, "ACTL")  == QSTR_EQUAL && !commandExecuted ) {  //TODO: on QUIT() this produces some "aborted(core dumped), invalid pointer" error
      QVector<double> temp_qvector;
      activities.insert(activities.size(), temp_qvector );
      maxTime++;
      activities[maxTime-1].resize(size);
      double temp_d;
      for(int i = 0; i < size; i++) {
        temp_d = readLeftDouble(&command);
        activities[maxTime-1][i] = isNaN(temp_d) ? 0 : temp_d;
        if(debug) terminal << command << endl;
      }
      currentTime++;
      repaint();
      commandExecuted = true;
    }
  //set activity display mode amod:
  if(QString::compare(commandType, "AMOD") == QSTR_EQUAL && !commandExecuted ) {
    if(command.length() > 0) {
      amod = leftParameter(command);
      if(debug) terminal << "new amod: " << amod << endl;
      command = cutLeftParameter(command);
      if(debug) terminal << "cut command: "<< command << endl;
      ndiameter = (int) readLeftDouble(&command);
      if(debug) terminal << "diameter: " << ndiameter << endl;
      repaint();
    }

    commandExecuted = true;
  }

  //Read ACTivity from file
  if (QString::compare(commandType, "RACT")  == QSTR_EQUAL && !commandExecuted ) {
    this->activityToRead(command);
    commandExecuted = true;
  }

  //Write ACTivity
  if (QString::compare(commandType, "WACT")  == QSTR_EQUAL && !commandExecuted ) {

  }

  //RUN Activities (and pauses if running already)
  if (QString::compare(commandType, "RUNA")  == QSTR_EQUAL && !commandExecuted ) {
    this->activityTimerToRun();
    commandExecuted = true;
  }

  //BReaK Activity run
  if (QString::compare(commandType, "BRKA")  == QSTR_EQUAL && !commandExecuted ) {
    if(activityTimerIsOn) this->activityTimerToRun();
    currentTime = 0;
    repaint();
    commandExecuted = true;
  }

  //control Synapse LaBeLs
  if (QString::compare(commandType, "SLBL")  == QSTR_EQUAL && !commandExecuted ) {
    if(QString::compare(command,"on") == QSTR_EQUAL  ) {
      if(!synapseLabelsShown) {
        this->synapseLabelsToShow();
        commandExecuted = true;
      }
    } else {
      if(QString::compare(command,"off") == QSTR_EQUAL) {
        if(synapseLabelsShown) {
          this->synapseLabelsToShow();
        }
        commandExecuted = true;
      } else {  //if neither on nor off is given: switch current option
        this->synapseLabelsToShow();
        commandExecuted = true;
      }
    }

  }
  //control Neuron LaBeLs
  if (QString::compare(commandType, "NLBL")  == QSTR_EQUAL && !commandExecuted ) {
    if(QString::compare(command,"on") == QSTR_EQUAL  ) {
      if(!neuronLabelsShown) {
        this->neuronLabelsToShow();

      }
      commandExecuted = true;
    } else {
      if(QString::compare(command,"off") == QSTR_EQUAL) {
        if(neuronLabelsShown) {
          this->neuronLabelsToShow();
        }
        commandExecuted = true;
      } else {  //if neither on nor off is given: switch current option
        this->neuronLabelsToShow();
        commandExecuted = true;
      }
    }

  }


  //RePaiNT the network
  if(QString::compare(commandType, "RPNT") == QSTR_EQUAL && !commandExecuted ) {
    repaint();
    commandExecuted = true;
  }

  //ReSET some parameters
  if (QString::compare(commandType, "RSET")  == QSTR_EQUAL && !commandExecuted ) {
    this->resetValues();
  }

  //default:
  if (!commandExecuted) {
    if(debug) terminal << "No action taken on: " << commandLine << endl;
  }
   //END OF if-else cascade ( commandType )
  debug = true;
}


void VisualizationView::interpreteWeightLine(QString weightLine) {
  QTextStream terminal(stdout);
  QString* inputLine = new QString(weightLine);
  double temp_weight;
  for(int j = 0; j< size; j++) {  //increments FROM neuron
    temp_weight = this->readLeftDouble(inputLine);
    weightMatrix[size*toNeuron + j] = temp_weight;  //incremention of toNeuron is done outside of this function.
  }

  //if(debug) terminal << weightLine << endl;
  //if(debug) terminal << "was (theoretically) interpreted." << endl;
}

/*
 * MATH and UTILITIES
 */

bool VisualizationView::isNaN(double checkthis) {
  if(checkthis != checkthis) {
    return true;
  } else {
    return false;
  }
}

void VisualizationView::setVvindex(int index) {
  vvindex = index;
  cp->setIndexedTitle(vvindex);
}

QString VisualizationView::cutLeftParameter(QString in) {
  QString out = in;
  bool searching = true;
  for(int i = 0; i<in.length() && searching; i++) {
    if(in.at(i).isSpace()) {
      out  = in.remove(0,i+1);
      searching = false;
    }
  }
  return out;
}

QString VisualizationView::leftParameter(QString in) {
  QString out;
  out.clear();
  for(int i = 0; i<in.length() && !(in.at(i).isSpace()); i++) {
    out.append(in.at(i));
  }
  return out;
}

void VisualizationView::determineFfPositions() {
  double xstep = 0.5/(double) sizeOfLayer.size();
  double ystep;
  int i;
  for(int l = 0; l < sizeOfLayer.size(); l++) {  //loop over layers l
    ystep = 0.5/(double) sizeOfLayer[l];
    for(int k = 0; k < sizeOfLayer[l]; k++) { //loop over neurons k in layer l
      i = indexOfNeuron(l,k);
      xPositionOfNeuron[i] = (1+2*l)*xstep;
      yPositionOfNeuron[i] = (1+2*k)*ystep;
    }
  }
}


int VisualizationView::indexOfNeuron(int layer, int neuron) {
  int index = 0;
  for(int l = 0; l < layer; l++) {
    index += sizeOfLayer[l];
  }
  index += neuron ;

  return index;
}

void VisualizationView::determineLayerSizes(QString lc) { //lc is layer command, space seperated integer list
  int distr_neurons = 0;  //counting distributed neurons
  QString lcc = lc; //layer command copy
  sizeOfLayer.clear();
  //sizeOfLayer[0] = 0;
  int temp_layersize;
  while(!lcc.isEmpty() && distr_neurons < size) {
    temp_layersize = (int) readLeftDouble(&lcc);
    if(distr_neurons + temp_layersize > size) {
      temp_layersize = size-distr_neurons;
    }
    sizeOfLayer.append(temp_layersize);
    distr_neurons += temp_layersize;
  }
  if(distr_neurons < size ) {
    sizeOfLayer.append(size-distr_neurons);
  }
}




