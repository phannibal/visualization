/*
 * QVisualization.h
 *
 *  Created on: 24.03.2014
 *      Author: philipp
 *      Descr.: main window header file
 */

#ifndef QVISUALIZATION_H_
#define QVISUALIZATION_H_

#include "VisualizationView.h"
//#include <QMainWindow>
//#include <QApplication>
#include <qt5/QtWidgets/QtWidgets>
#include <qt5/QtGui/QtGui>
#include <QString>
#include <QTextStream>
#include <QVector>
//#include <QGridLayout>
//#include <QGroupBox>

class QVisualization : public QMainWindow
{
    Q_OBJECT

  public:
    QVisualization(QWidget* parent = 0);
    ~QVisualization();

    VisualizationView* visWindow;
    QVector< VisualizationView* > visBulk;

    void startPipeListening();



  private:
    bool debug;
    bool piping;
    int max_vvindex;
    QString instring;
    QTextStream* inQStream;
    QTextStream* outQStream;
    QGridLayout* layout;
    QGroupBox* maingroup;

    void interpreteGlobalCommandLine(QString command);
    void deleteNetwork(int vvindex);
    void createNetwork();
    void refreshLayout();
    QVector< bool > isDeleted;

    int receiverVV;

  public slots:
    void hearkenTheStdin();
    void deliverWeightLine(int vvindex);
    void allControlPanelsToShow();
    void networkToCreate();
    void receiveSingleCommandLine(QString cl);

};



#endif /* QVISUALIZATION_H_ */
