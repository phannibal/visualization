/*
 * TopologyCalculator.h
 *
 *  Created on: 13.06.2014
 *      Author: philipp
 */

#ifndef TOPOLOGYCALCULATOR_H_
#define TOPOLOGYCALCULATOR_H_

#include <math.h>
#include <stdlib.h>
#include <iostream>

class TopologyCalculator {
  public:
    TopologyCalculator(double*,double*, double*, int);  //receives positions
    ~TopologyCalculator();

    void integrateSingleStep_Euler();
    double* get_xpositions();
    double* get_ypositions();
    void normPositions();

  private:
    double* w_ic;  //interconnectivity
    double* weights;  //weight matrix stored seperately
    double* position;  //positions of neuron i at x ([0]) or y ([1])
    double* force; //current force on neuron i from neuron j at  x ([0]) or y ([1])
    double* sumForce;
    double* velocity;  //velocity of neuron i
    double* mass;
    double rx;
    double ry;
    double r; //radius, for temporal calcs
    double v; //speed, for temporal calcs
    double gamma; //friction parameter

    double t;
    double dt;

    int size; //amount of handled neurons
    int s2; //square of size

    //functions:
    double f1(int, int, int);


};


#endif /* TOPOLOGYCALCULATOR_H_ */
