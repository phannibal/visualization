/*
 * QVisualization.cpp
 *
 *  Created on: 24.03.2014
 *      Author: Philipp Hannibal
 * Description: Code for QVisualization.h
 *              QMainWindow widget.
 *
 */

#ifndef QSTR_EQUAL
#define QSTR_EQUAL 0
#endif

//include necessary libraries:
#include "QVisualization.h"
#include "VisualizationView.h"
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
//#include <QCheckBox>
#include <QTimer>
//#include <QGridLayout>
//#include <QGroupBox>
#include <QVector>
#include <cmath>
#include <stdlib.h>
#include <iostream>
#include <math.h>

/*
 *Define functions declared in QVisualization.
 */

//Constructor:
QVisualization::QVisualization(QWidget* parent)
  : QMainWindow(parent)
{
  debug = true;
  piping = false;
  /* transferred to VisualizationView
  QString weightsFileName("weights2.txt");
  readWeightsFromFile(weightsFileName);
  QString defaultMode = "linear";
  setMode(defaultMode);
  positionsFixed = false;
  drawArea = 0.7;
  */
  layout = new QGridLayout();
  maingroup = new QGroupBox();

  //setup VV bulk:
  max_vvindex = 0;  //this shall not be decreased
  visBulk.resize(max_vvindex+1);
  for(int vvindex = 0; vvindex<=max_vvindex;vvindex++) {
    if(debug) std::cout << "VV 1 initialisieren:..." << std::endl;
    visBulk[vvindex] = new VisualizationView(this);
    visBulk[vvindex]->setVvindex(vvindex);
    if(debug) std::cout << "VV 1 ready!" << std::endl;
  }

  //visBulk[vvindex]->fileToRead etc...


  //initiate central Widget:

  //visWindow = new VisualizationView(this);
  //VisualizationView* visWindow2 = new VisualizationView(this);
  //VisualizationView* visWindow3 = new VisualizationView(this);
  //VisualizationView* visWindow4 = new VisualizationView(this);

  //visWindow2->fileToRead("topology_test_2.txt");

  //layout->addWidget(visWindow,0,0);
  int layoutsize = (int) sqrt(max_vvindex+1);
  if(max_vvindex == 2) layoutsize = 2;  //case of 3networks
  if(debug) std::cout << layoutsize << std::endl;
  int vvLinesAdded =0;
  for(int vvindex = 0; vvindex <= max_vvindex; vvindex++) {

      layout->addWidget( visBulk[vvindex] ,vvLinesAdded,vvindex%layoutsize);
      if(debug) std::cout << vvLinesAdded << "... "<< vvindex%layoutsize << std::endl;

      if(vvindex%layoutsize == layoutsize-1) vvLinesAdded++;
  }
  //layout->addWidget(visWindow2,0,1);
  //layout->addWidget(visWindow3,1,0);
  //layout->addWidget(visWindow4,1,1);

  maingroup->setLayout(layout);
  if(debug) std::cout << "maingroup refreshed" << std::endl;
  setCentralWidget(maingroup);
  if(debug) std::cout << "central widget refreshed" << std::endl;

  //initiate status bar:
  //statusBar()->showMessage(tr("Ready"),1000);

  //setup toolbar (with quit, select network, select mode):

  QToolBar* toolbar = addToolBar( "main toolbar");

  //parent->addToolBar(Qt::LeftToolBarArea, toolbar);
  QAction* quitAct = toolbar->addAction("Quit");
  connect(quitAct,SIGNAL(triggered()),qApp,SLOT(quit()));


  toolbar->addSeparator();
  QAction* showControlPanelsAct = toolbar->addAction("Show All Control Panels");
  connect(showControlPanelsAct, SIGNAL(triggered()),this,SLOT(allControlPanelsToShow()));

  toolbar->addSeparator();
  QAction* createNetworkAct = toolbar->addAction("Create Network");
  connect(createNetworkAct, SIGNAL(triggered()), this, SLOT(networkToCreate()));

  toolbar->addSeparator();
  QAction* commandAct = toolbar->addAction("New Command");
  QInputDialog* inputDialog = new QInputDialog(this);
  connect(commandAct, SIGNAL(triggered()), inputDialog, SLOT(show()));
  connect(inputDialog, SIGNAL(textValueSelected(QString)), this, SLOT(receiveSingleCommandLine(QString)));
 /*
  QAction* selectFileAct = toolbar->addAction("Select Network");
  QAction* selectModeAct = toolbar->addAction("Select Mode");
  QAction* selectActivityAct = toolbar->addAction("Select Activity");
  toolbar->addSeparator();
  QAction* switchSynapseLabelsOnOffAct = toolbar->addAction("Synapse Labels On/Off");
  QAction* switchActivityTimerOnOffAct = toolbar->addAction("Run Activities On/Off");
  QAction* showControlPanelAct = toolbar->addAction("Show Control Panel");

  //file dialog for network select:
  QFileDialog* fileDialog_forNetwork = new QFileDialog(this);
  fileDialog_forNetwork->hide();
  connect(selectFileAct,SIGNAL(triggered()),fileDialog_forNetwork,SLOT(show()));
  connect(fileDialog_forNetwork,SIGNAL(fileSelected(QString)),this->visBulk[0],SLOT(fileToRead(QString)));

  //file dialog for activity select:
  QFileDialog* fileDialog_forActivity = new QFileDialog(this);
  fileDialog_forActivity->hide();
  connect(selectActivityAct,SIGNAL(triggered()),fileDialog_forActivity,SLOT(show()));
  connect(fileDialog_forActivity,SIGNAL(fileSelected(QString)),this->visBulk[0],SLOT(activityToRead(QString)));

  //mode dialog:
  QInputDialog* inputDialog = new QInputDialog(this);
  inputDialog->hide();
  connect(selectModeAct, SIGNAL(triggered()), inputDialog, SLOT(show()));
  connect(inputDialog, SIGNAL(textValueSelected(QString)),this->visBulk[0],SLOT(modeToApply(QString)));

  //switch synapse labels and activity:
  connect(switchSynapseLabelsOnOffAct, SIGNAL(triggered()), this->visBulk[0],SLOT(synapseLabelsToShow()));
  connect(switchActivityTimerOnOffAct, SIGNAL(triggered()), this->visBulk[0],SLOT(activityTimerToRun()));

  //show control panel button
  connect(showControlPanelAct, SIGNAL(triggered()), this->visBulk[0],SLOT(controlPanelToShow()));
*/
  //END OF toolbar setup

  //START OF instream setup


  //END OF instream setup

}



QVisualization::~QVisualization() {

}

/*
 * Public functions:
 */
void QVisualization::startPipeListening() { //cast this function to initiate the timer, reading the in pipe
  inQStream = new QTextStream(stdin, QIODevice::ReadOnly);
  outQStream = new QTextStream(stdout);
  QTimer* pipeTimer = new QTimer(this);
  connect(pipeTimer, SIGNAL(timeout()), this, SLOT(hearkenTheStdin()));

  for(int vvindex = 0; vvindex <= max_vvindex; vvindex++) {
    connect( this->visBulk[vvindex], SIGNAL(requireWeightLine(int)), this, SLOT( deliverWeightLine(int)));
  }

  pipeTimer->start(100);
  piping = true;//?

}

/*
 * PUBLIC SLOTS:
 */

void QVisualization::hearkenTheStdin() {  //this is called every ?100ms
  debug = false;
  //*outQStream << "Hearken to the stdin stream:" << endl;
  instring = inQStream->readLine();
  if(!instring.isNull()) {  //isNull() is operator to check if something is written into inQStream
    //if(debug) *outQStream << "Received command line:" << endl;
    //if(debug) *outQStream << instring << endl;

    //if(QString::compare(instring.left(1),"#") != 0) { //# for comment
      //instring.remove(0,1);
    receiverVV = (int) visBulk[0]->readLeftDouble(&instring);

    if(receiverVV == -1) {  //type "-1 CMMD ..." to access QVis commands
      this->interpreteGlobalCommandLine(instring);
    } else {  //else type "vvindex CMMD ..." to access VV with index vvindex
      if(receiverVV <= max_vvindex) {
        //if(debug) *outQStream << "Posting it to VV..." << endl;
        this->visBulk[receiverVV]->interpreteCommandLine(instring);
      }
    }
    //}
  }
  debug = true;
}

//public slot:
void QVisualization::deliverWeightLine(int vvindex) {
  instring = inQStream->readLine();
  this->visBulk[vvindex]->interpreteWeightLine(instring);
}

//private:
void QVisualization::interpreteGlobalCommandLine(QString commandLine) {
  QString command = commandLine;  //copy of commandLine, modify QString command
  QString commandType = commandLine.left(4);
  command.remove(0,5);

  bool cexe = false;  //to indicate command's execution

  if(QString::compare(commandType,"DELN") == QSTR_EQUAL) {  //DELete Network
    int vvindex = (int) this->visBulk[0]->readLeftDouble(&command);
    if(!this->visBulk[0]->isNaN((double) vvindex)) {
      deleteNetwork(vvindex);
    }
    cexe = true;
  }

  if(QString::compare(commandType,"CRTN") == QSTR_EQUAL) {  //CReaTe Network
    createNetwork();
    cexe = true;
  }

  /*if(QString::compare(commandType,"COPY") == QSTR_EQUAL) {
    int vvindex = (int) this->visBulk[0]->readLeftDouble(&command);
    visBulk.append(visBulk[vvindex]);
    cexe = true;
  }*/ //doesn't work this way


  if(!cexe) {
    //if(debug) *outQStream << "No command executed on line '" << commandLine <<"'." << endl;
  }

}

void QVisualization::deleteNetwork(int vvindex) {
  //ATTENTION: this will shift the previous vvindexes to address a network
  //?Nonetheless the VV internal vvindex stays constant?
  if(vvindex <= max_vvindex && vvindex >=0 && max_vvindex > 0) {  //delete, if there's more than 1 network, and 0<=vvi<=maxvvi
    delete visBulk[vvindex];
    visBulk.remove(vvindex);
    max_vvindex--;
    refreshLayout();
    for(int i = vvindex; i<= max_vvindex; i++) {
      visBulk[i]->setVvindex(i);
    }

    //if(debug) *outQStream << "One network successfully chopped." << endl;
  }
}

void QVisualization::createNetwork() {
  //*outQStream << "Creating network..." << endl;
  visBulk.append(new VisualizationView(this));
  //*outQStream << "connecting created network..." << endl;
  max_vvindex++;
  visBulk[max_vvindex]->setVvindex(max_vvindex);
  connect( this->visBulk[max_vvindex], SIGNAL(requireWeightLine(int)), this, SLOT( deliverWeightLine(int)));

  //if(debug) *outQStream << "New network added. New maximum index is " << max_vvindex << endl;

  refreshLayout();

}

void QVisualization::refreshLayout() {
  int layoutsize = (int) sqrt(max_vvindex+1);
  if(max_vvindex == 2) layoutsize = 2;  //case of 3networks
  if(debug) std::cout << layoutsize << std::endl;
  int vvLinesAdded =0;

  for(int vvindex = 0; vvindex <= max_vvindex; vvindex++) {
    layout->addWidget( visBulk[vvindex] ,vvLinesAdded,vvindex%layoutsize);
    if(debug) std::cout << vvLinesAdded << "... "<< vvindex%layoutsize << std::endl;
    if(vvindex%layoutsize == layoutsize-1) vvLinesAdded++;
  }
}

//public slot:
void QVisualization::allControlPanelsToShow() {
  for(int vvindex = 0; vvindex <= max_vvindex; vvindex++) {
    visBulk[vvindex]->controlPanelToShow();
  }
}

void QVisualization::networkToCreate() {
  this->createNetwork();
}

void QVisualization::receiveSingleCommandLine(QString commandLine) {
  QString clc = commandLine;  //copy of commandLine
  if(!clc.isNull()) {  //isNull() is operator to check if something is written into inQStream
    receiverVV = (int) visBulk[0]->readLeftDouble(&clc);
    if(receiverVV == -1) {  //type "-1 CMMD ..." to access QVis commands
      this->interpreteGlobalCommandLine(clc);
    } else {  //else type "vvindex CMMD ..." to access VV with index vvindex
      if(receiverVV <= max_vvindex) {
        this->visBulk[receiverVV]->interpreteCommandLine(clc);
      }
    }
  }
}







