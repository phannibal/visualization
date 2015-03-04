/*
 * main.cpp
 *
 *  Created on: 24.03.2014
 *      Author: Philipp Hannibal
 * Description: Program to visualize matrices that represent ANNs
 */

#include "QVisualization.h"
//#include "QNeuronView.h"
#include "ControlPanel.h"
//#include "PipeInterpreter.h"



#include <QtGui>
#include <stdlib.h>
#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
//#include <QApplication>
//#include <QWidget>
#include <qt5/QtWidgets/QtWidgets>
#include <qt5/QtGui/QtGui>
//#include <QLayout>
//#include <QFileDialog>
//#include <QLabel>




int main(int argc, char** argv) {
  QApplication app(argc,argv);
  //QWidget mainWindow;
  QVisualization window;
  //QVisualization* vis2 = new QVisualization(&mainWindow);
  if(argc>1) {
    std::cout << argv[1] << "=============== Eingabe argv[1]" << std::endl;
    std::string inputword = argv[1];
    if(inputword == "-pipe") {
      window.startPipeListening();
      std::cout << "Pipe activated." << std::endl;
    } else {
      //window.visWindow->setMode(argv[1]);
    }
  } else {
    //window.visWindow->setMode("random");
  }
  if(argc>2) {
    //window.visWindow->readWeightsFromFile(argv[2]);
  }

  //window.visWindow->writeRandomWeightsToFile(36);
  //window.visWindow->writeRandomActivitiesToFile(36,20);
  //window.visWindow->readWeightsFromFile("weights.txt");
  //window.visWindow->setMode("map");


  //window.visWindow->writeRandomWeightsToFile(5);
  //window.visWindow->writeRandomActivitiesToFile(5,15);

  //QVBoxLayout vlayout;
  //vlayout.addWidget(vis2);
  //mainWindow.setLayout(&vlayout);
  //mainWindow.setWindowTitle("Main");
  //mainWindow.show();
  //mainWindow.move(700,600);
  //vis2->writeRandomWeightsToFile(10);
  //mainWindow.resize(500,300);
  //QFileDialog* fileDialog = new QFileDialog(&window);
  //fileDialog->show();

  window.resize(800,800);
  //window.visWindow->resize(100,100);
  //QLabel* window.visWindow->label2 = new QLabel("trial and error");
  //window.visWindow->label2->show();
  window.move(400,600);
  window.setWindowTitle("Visualization");
  window.show();

  //Stream with visview's timer:
  //activates in constructor of VV
  //may produce problems with multiple different VVs

  /*
   * Stream version with external pipe

  char in;
  int linebreaker=0;
  FILE* instream;
  instream = popen("../pipe/./pipe","r");
  window.visWindow->hearkenTheStream(instream);
   */
  /*
  while(!feof(instream)) {

    in = fgetc(instream);
    std::cout << in;
    if(linebreaker%20 == 0) {
      std::cout << std::endl;
    }
    linebreaker++;
  }
  pclose(instream);
  */






  return app.exec();
}



