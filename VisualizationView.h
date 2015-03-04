/*
 * VisualizationView.h
 *
 *  Created on: 25.04.2014
 *      Author: philipp
 */

#ifndef VISUALIZATIONVIEW_H_
#define VISUALIZATIONVIEW_H_

struct visualizationParameterList;
class VisualizationView;

#include "ControlPanel.h"
#include "TopologyCalculator.h"

#include <qt5/QtWidgets/QtWidgets>
#include <qt5/QtGui/QtGui>
//#include <QMainWindow>
//#include <QLabel>
#include <QTextStream>
#include <stdio.h>
#include <stdlib.h>

struct visualizationParameterList {
    QString mode;
    QString amod;
    QString networkFile = "none";
    QString activityFile = "none";
    int size;
    int t;
    bool activityState;
    int outsize;
    int insize;
    bool positionState;
    bool labelsShown;
    bool nlabelsShown;
    bool topologyAligned;
};


class VisualizationView : public QWidget
{
    Q_OBJECT

  public:
    VisualizationView(QMainWindow* parent = 0);
    ~VisualizationView();

    void readWeightsFromFile(QString);  //reads the (size x size)# of weights into weightMatrix
    void readActivitiesFromFile(QString, int startLine, int endLine);
    void readEsnFromFile(QString);
    void setMode(QString mode);
    void setActivityTimerIsOn(bool);
    void writeRandomWeightsToFile(int sizeN); //writes sizeN x sizeN matrix with random weights to file weights<sizeN>.txt
    void writeRandomActivitiesToFile(int numberNeurons, int timesteps); //writes for #timesteps random activities for #numberNeurons neurons.

    void hearkenTheStream(FILE* stream);
    void interpreteCommandLine(QString commandline);
    void interpreteWeightLine(QString weightLine);

    double readLeftDouble(QString* ); //cuts the QString after double has been read
    bool isNaN(double);
    void setVvindex(int index);
    QString cutLeftParameter(QString);  //cuts off the first parameter of spaced seperated QString paramlist and returns the rest
    QString leftParameter(QString); //returns most left param of space speerated QString params

    int vvindex;
    static int amount;  //contains the amount of active VVs


  private:


    double* weightMatrix;
    double* inMatrix;
    double* outMatrix;
    double* esnMatrix;
    int size; //size of weightMatrix
    int inSize; //number of in-neurons
    int outSize;  //number of out-neurons
    int currentTime; //internal int for current time of activity run
    int maxTime;  //indicates the maximum time to be displayed (to prevent array overrun)
    int toNeuron;  //indicates the current TO neuron (for requireWeightLine increment)
    double* xPositionOfNeuron;
    double* yPositionOfNeuron;
    double drawArea;
    bool debug;

    char inchar;  //temporarilly saves the pipe's stream incoming character
    QString mode;
    QString amod; //activity mode: eg color, radius (TODO: sound?)
    QString commandType;  //contains first 4 letters of commandline
    QString command;

    QLabel* synapseLabels;
    QLabel* neuronLabels;

    bool positionsFixed;
    bool topologyAligned;
    bool synapseLabelsShown;
    bool neuronLabelsShown;
    bool activityTimerIsOn;
    int ndiameter;  //default diameter of neurons in pixel

    FILE* instream;
    QString instring;
    QTextStream* inQStream;
    QTextStream* outQStream;

    ControlPanel* cp;
    TopologyCalculator* topCalc;
    visualizationParameterList controlledParameters;
    QMainWindow* parent;
    QVector<QVector<double> > activities;
    QVector< int > sizeOfLayer; //for ff mode


    void initiateInAndOut();  //sets inMatrix and outMatrix to size x size matrices filled with zeros
    void readSizeFromFile(QString); //reads size from file

    QString writeWeights(); //writes weightMatrix in a QString


    int readLinelengthOfFile(QString);

    void drawInAndOut(QPainter*);
    void drawNeuronNetworkFromWeightMatrix(QPainter*, QString mode = "linear");  //select mode: linear, random
    void drawGrid(QPainter*);
    void drawSynapseArc(QPainter*, int firstNeuron, int secondNeuron, bool drawWithLabel = false);  //first defines the row, second defines the column of corresponding weight
    void drawSelfConnect(QPainter*, int neuronIndex); //, bool drawWithLabel = false);
    void paintEvent(QPaintEvent*);
    void timerEvent(QTimerEvent*);
    void resetValues();
    void printPositions();
    void clearActivities();
    void determineFfPositions();
    int indexOfNeuron(int layer, int neuron);
    void determineLayerSizes(QString command);

  public slots:
    void fileToRead(QString file);
    void modeToApply(QString mode);
    void synapseLabelsToShow();
    void neuronLabelsToShow();
    void activityTimerToRun();
    void activityToRead(QString file);
    void refreshParameters();
    void controlPanelToShow();
    void readThePipe();
    //void hearkenTheStdin();

  signals:
    void requireWeightLine(int vvindex);



};



#endif /* VISUALIZATIONVIEW_H_ */
