/*
 * ControlPanel.cpp
 *
 *  Created on: 27.06.2014
 *      Author: philipp
 */


#include "ControlPanel.h"

#include <qt5/QtWidgets/QtWidgets>
#include <qt5/QtGui/QtGui>
//#include <QMainWindow>

//#include <QVBoxLayout>
//#include <QHBoxLayout>
//#include <QGridLayout>
//#include <QLabel>
//#include <QPushButton>
#include <QTextStream>
//#include <QInputDialog>
//#include <QFileDialog>

ControlPanel::ControlPanel(VisualizationView* source)
  : QWidget()
{
  //debug purpose:
  QTextStream terminal(stdout);
  debug = false;  //true: debug messages are written to terminal
  //common:
  ncp = 13;
  this->source = source;

  QString* paraNames = new QString[ncp];  //contains: parameter names
  paraNames[0] = "Draw Mode";
  paraNames[1] = "Network File";
  paraNames[2] = "Activity File";
  paraNames[3] = "Time";
  paraNames[4] = "Synapse Labels";
  paraNames[5] = "Size";
  paraNames[6] = "Out size";
  paraNames[7] = "In size";
  paraNames[8] = "Position State";
  paraNames[9] = "Activity Run";
  paraNames[10] = "Topology";
  paraNames[11] = "Activity Code";
  paraNames[12] = "Neuron Labels";


  //positioning:
  this->move(50,50);
  this->resize(200,400);
  this->setWindowTitle("Control Panel");
  //int index = 1;
  //this->setWindowTitle( QString("VV %1 Control Panel").arg(index) );
  this->show();

  //layout
  QGridLayout* grid = new QGridLayout(this);
  grid->setSpacing(1);
  grid->setOriginCorner(Qt::TopLeftCorner); //somewhat buggy


  parameterLabels = new QLabel[2*ncp]; //contains: parameter values as text
  buttons = new QPushButton*[ncp];  //array of pointers to QPushButton objects.

  //add headline:
  QLabel* tempLabel = new QLabel();
  tempLabel->setText("Parameter");
  grid->addWidget(tempLabel,0,0,1,1);

  QLabel* tempLabel2 = new QLabel();
  tempLabel2->setText("Value");
  grid->addWidget(tempLabel2,0,1,1,1);

  QLabel* tempLabel3 = new QLabel();
  tempLabel3->setText("Option");
  grid->addWidget(tempLabel3,0,2,1,1);
  //end of headline adding


  //add names to grid
  for (int k = 0; k<ncp; k++) {
    parameterLabels[k].setText(paraNames[k]);
    grid->addWidget(&(parameterLabels[k]),k+1,0,1,1);
  }

  //add statusses to grid
  for (int k = 0; k<ncp; k++) {
    parameterLabels[ncp+k].setText("N/A");
    grid->addWidget(&(parameterLabels[ncp+k]) ,k+1,1,1,1);
  }

  //add buttons to grid
  for (int k = 0; k<ncp; k++) {
    buttons[k] = new QPushButton("Set/Reset");
    grid->addWidget(buttons[k],k+1,2,1,1);
  }

  //set buttons' functions:
  for (int k = 0; k<ncp; k++) {
    switch(k){
      case 0: { //MODE
        QInputDialog* inputDialog = new QInputDialog(this);
        inputDialog->hide();
        connect(buttons[k], SIGNAL(clicked()), inputDialog, SLOT(show()));
        connect(inputDialog, SIGNAL(textValueSelected(QString)),this->source,SLOT(modeToApply(QString)));
        break;
      }

      case 1: { //network file
        QFileDialog* fileDialog_forNetwork = new QFileDialog(this);
        fileDialog_forNetwork->hide();
        connect(buttons[k],SIGNAL(clicked()),fileDialog_forNetwork,SLOT(show()));
        connect(fileDialog_forNetwork,SIGNAL(fileSelected(QString)),this->source,SLOT(fileToRead(QString)));
        break;
      }

      case 4: { //synapse labels:
        connect( buttons[k],SIGNAL(clicked()), this->source,SLOT(synapseLabelsToShow()));
        break;
      }

      case 9: { //activity run
        connect( buttons[k],SIGNAL(clicked()), this->source,SLOT(activityTimerToRun()));
        break;
      }

      case 12: {    //neuron labels
        connect( buttons[k], SIGNAL(clicked()), this->source, SLOT(neuronLabelsToShow()));
        break;
      }

      default: {
        buttons[k]->hide();
        break;
      }
    }

  }

  setLayout(grid);

  //update();


}

ControlPanel::~ControlPanel() {

}

/* ==========================
 * SLOTS
 */

void ControlPanel::update(visualizationParameterList p) {
  QTextStream terminal(stdout);
  if(debug) terminal << "... delivering value 1 ... " << p.mode << " ... and others ..." << endl;
  parameterLabels[ncp].setText(p.mode);
  //terminal << "... delivering value 2 ... " << endl;
  parameterLabels[ncp+1].setText(p.networkFile);
  //terminal << "... delivering value 3 ... " << endl;
  parameterLabels[ncp+2].setText(p.activityFile);
  //terminal << "... delivering value 4 ... " << endl;
  parameterLabels[ncp+3].setText(QString("%1").arg(p.t));

  //terminal << "... delivering value 5 ... " << endl;
  if(p.labelsShown) {
    parameterLabels[ncp+4].setText("Shown");
  } else {
    parameterLabels[ncp+4].setText("Hidden");
  }

  //terminal << "... delivering value 1 ... " << endl;
  parameterLabels[ncp+5].setText(QString("%1").arg(p.size));
  //terminal << "... delivering value 6 ... " << endl;
  parameterLabels[ncp+6].setText(QString("%1").arg(p.outsize));
  //terminal << "... delivering value 7 ... " << endl;
  parameterLabels[ncp+7].setText(QString("%1").arg(p.insize));

  //terminal << "... delivering value 8 ... " << endl;
  if(p.positionState) {
    parameterLabels[ncp+8].setText("Fixed");
  } else {
    parameterLabels[ncp+8].setText("Loose");
  }

  //terminal << "... delivering value 9 ... " << endl;
  if(p.activityState) {
    parameterLabels[ncp+9].setText("Running");
  } else {
    parameterLabels[ncp+9].setText("Paused");
  }

  //terminal << "... delivering value 10 ... " << endl;
  if(p.topologyAligned) {
    parameterLabels[ncp+10].setText("Aligned");
  } else {
    parameterLabels[ncp+10].setText("undefined");
  }

  parameterLabels[ncp+11].setText(p.amod);

  if(p.nlabelsShown) {
      parameterLabels[ncp+12].setText("Shown");
  } else {
      parameterLabels[ncp+12].setText("Hidden");
  }

}


void ControlPanel::setIndexedTitle(int index) { //refreshes cp title to index and shifts the window
  this->setWindowTitle( QString("VV %1 Control Panel").arg(index) );
  this->move(50+10*index,50+20*index);
  //std::cout << "Moved the window!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
}
