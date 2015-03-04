/*
 * ControlPanel.h
 *
 *  Created on: 27.06.2014
 *      Author: philipp
 */

#ifndef CONTROLPANEL_H_
#define CONTROLPANEL_H_

class ControlPanel;

#include "VisualizationView.h"

#include <qt5/QtWidgets/QtWidgets>
#include <qt5/QtGui/QtGui>
//#include <QMainWindow>
//#include <QVBoxLayout>
//#include <QHBoxLayout>
//#include <QGridLayout>
//#include <QLabel>
//#include <QPushButton>

class ControlPanel : public QWidget
{
  Q_OBJECT

  public:
    ControlPanel(VisualizationView* source);
    ~ControlPanel();

    void setIndexedTitle(int index);

  private:
    int size;
    double t;
    QString mode;
    QString network;
    QString activity;
    int ncp;
    bool debug;
    QLabel* parameterLabels;
    QPushButton** buttons;
    QString* paraNames;

    VisualizationView* source;

  public slots:
    void update(visualizationParameterList);
  /*
    void updateSize(int);
    void updateT(double);
    void updateMode(QString);
    void updateNetwork(QString);
    void updateActivity(QString);
   */




};


#endif /* CONTROLPANEL_H_ */
