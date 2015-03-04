/*
 * TopologyCalculator.cpp
 *
 *  Created on: 13.06.2014
 *      Author: philipp
 */

#include "TopologyCalculator.h"

#include <math.h>
#include <stdlib.h>
#include <iostream>
#include <QTextStream>



TopologyCalculator::TopologyCalculator(double* xpositions, double* ypositions, double* weights, int size) {
  //set array lengths
  position = new double[2*size];  //access via x-component: position[OF], y-component: position[size+OF]. OF = index OF neuron (i,j,k...)
  mass = new double[size];
  this->weights = weights;
  force = new double[2*size*size];
  sumForce = new double[2*size];
  velocity = new double[2*size];
  w_ic = new double[size*size];
  this->size = size;
  s2 = size * size;
  gamma = 0.1;

  //set time scale:
  t = 0.0;
  dt = 0.01;

  QTextStream terminal(stdout);

  //store positions:
  terminal << "Copy position pointers..." << endl;
  for(int i = 0; i<size; i++) {
    position[i] = xpositions[i];
    position[size+i] = ypositions[i];
    terminal << position[i] << " und " << position[size+i] << endl;
  }

  terminal << "Set forces and velocities to zero..." << endl;
  //initiate zero force, zero velocity:
  for(int i = 0; i<size; i++) {
    for(int j = 0; j<=i; j++) {
      force[i*size+j] = force[size*size+(i*size+j)] = force[j*size+i] = force[size*size+(j*size+i)] = 0.0;
    }
    velocity[i]  = velocity[size+i] = 0.0;
  }

  terminal << "Calculate interconnectivity:" << endl;
  //calculate interconnectivity (|wij*wji|+|wij|+|wji|):
  double tempw = 0;
  double tempw2= 0;
  double tempw3= 0;
  for(int i = 0; i<size; i++) {
    for(int j = 0; j<=i; j++) {
      tempw = weights[i*size+j]*weights[j*size+i];
      tempw2=weights[i*size+j];
      tempw2 = tempw2 < 0 ? -tempw2 : tempw2;
      tempw3=weights[j*size+i];
      tempw3 = tempw3 < 0 ? -tempw3 : tempw3;
      w_ic[i*size+j] = w_ic[j*size+i] = (tempw < 0 ? -tempw : tempw) + tempw2 + tempw3;
      terminal << w_ic[i*size+j] << " for " << i << " and " << j << endl;
    }
  }

  //calculate neuron's inertia ("mass")
  terminal << "Calculate inertiae:" << endl;
  double tempm = 0;
  for(int i = 0; i<size; i++) {
    mass[i] = 1.0;
    for(int j=0; j<size; j++) {
      tempm = weights[i*size+j];
      mass[i] += tempm<0? -tempm : tempm;
    }
    mass[i] = mass[i]/(double) size;
    terminal << mass[i] << endl;
  }

}

TopologyCalculator::~TopologyCalculator() {

}

void TopologyCalculator::integrateSingleStep_Euler() {
  //clear summed forces:
  for(int i = 0; i< size; i ++) {
    sumForce[i] = sumForce[size+i] = 0.0;
  }

  //calculate force, followed by acceleration
  for(int i = 0; i<size; i++) {

    for(int j = 0; j<i; j++) {//no force on itself (j<i !!)
      //x-direction:
      force[i*size+j] = f1(i,j,0);
      sumForce[i] += force[i*size+j];
      force[j*size+i] = -force[i*size+j];//symmetric force
      sumForce[j] += force[j*size+i];

      //y-direction:
      force[size*size+(i*size+j)] = f1(i,j,1);
      sumForce[size+i] += force[size*size+(i*size+j)];
      force[size*size+(j*size+i)] = -force[size*size+(i*size+j)];
      sumForce[size+j] += force[size*size+(j*size+i)];
    }
  }
  //calculate velocities (with speed damping / friction : f* = f1 -gamma v, gamma see constructor
  for(int i = 0; i<size; i++) {
    velocity[i] += dt * (sumForce[i]-gamma *velocity[i])/mass[i];// delta v = delta t * acceleration (a=F/m)
    velocity[size+i] += dt * (sumForce[size+i]-gamma*velocity[size+i])/mass[i];
  }

  //calculate locations
  for(int i = 0; i<size; i++) {
    for(int dim = 0; dim<2; dim++) {
      position[dim*size+i] += dt * velocity[dim*size+i];

      //stiff boundary:
      /*
      if(position[dim*size+i]>1) position[dim*size+i] = 1;
      if(position[dim*size+i]<0) position[dim*size+i] = 0;
      */
      /* PERIODIC BOUNDARY */
      while(position[dim*size+i]>1) {  //periodic boundary
        position[dim*size+i] -= 1;
      }
      while(position[dim*size+i]<0) {
        position[dim*size+i] += 1;
      }
      /* */

    }
  }

  //increase current time:
  t += dt;

  //write positions:
  get_xpositions();
  get_ypositions();

}
/*
 * force functions
 * f1 = e_r * (1 - w * r)
 */
double TopologyCalculator::f1(int i, int j, int dim) {
  //ensure, that i > j
  /*
  if(j>i) {
    int tempindex = j;
    j = i;
    i = tempindex;
  }
  */
/* PERIODIC BOUNDARY CONDITION */
  rx = position[j] - position[i];
  rx = rx < -0.5 ? 1+rx : rx; //periodic boundary
  rx = rx > 0.5? rx-1 : rx; //periodic boundary

  ry = position[size+j] - position[size+i];
  ry = ry < -0.5 ? 1+ry : ry; //periodic boundary
  ry = ry > 0.5? ry-1: ry; //periodic boundary
/* */

  r = sqrt(rx*rx+ry*ry);

  //TODO: check if it is necessary to add signum here (for i > j):
  //DEFINITION OF TOPOLOGY FORCE HERE: f1 = unity * (1/r - w*r)
  //forceless at: r0 = 1 / sqrt(w_ic)
  if(dim == 0) {
    return -rx/r*(1/r-w_ic[i*size+j]*r);

  } else {
    return -ry/r*(1/r-w_ic[i*size+j]*r);
  }

}
/*
 * others
 */
void TopologyCalculator::normPositions() {//only for non-periodic boundary
  //search for the highest abs of a position: ATTENTION: abs is integer function use fabs for double
  double pos_MAX = 1;
  for(int i = 0; i<size; i++) {
    for(int dim = 0; dim < 2; dim++) {
      pos_MAX = abs(position[dim*size+i])>pos_MAX? abs(position[dim*size+i]) : pos_MAX;
    }
  }
  //norm all position values:
  for(int i = 0; i<size; i++) {
    for(int dim = 0; dim < 2; dim++) {
      position[dim*size+i] = position[dim*size+i] / pos_MAX;
    }
  }

}

/*
 * GET functions
 */
double* TopologyCalculator::get_xpositions() {
  QTextStream terminal(stdout);
  double* retPos = new double[size];
  for(int i = 0; i <size; i++) {
    retPos[i] = position[i];
    //terminal << "(TC:) x of " << i << " is " << position[i] << endl;
  }
  return retPos;
}

double* TopologyCalculator::get_ypositions() {
  QTextStream terminal(stdout);
  double* retPos = new double[size];
  for(int i = 0; i <size; i++) {
    retPos[i] = position[size+i];
    //terminal << "(TC:) y of " << i << " is " << position[size+i] << endl;
  }
  return retPos;
}

