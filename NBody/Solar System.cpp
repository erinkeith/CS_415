#include <iostream>
#include <cmath>
#include <time.h>
using namespace std;

// hours in a year 8766
const int RUN_TIME = 8766;

// max radius 590
const int HEIGHT = 1200;
const int WIDTH = 1200;

const double G = .00000000006673;

const int SUN_X = WIDTH/2;
const int SUN_Y = HEIGHT/2;

const int SUN = 0;
const int JUPITER = 1;
const int SATURN = 2;
const int URANUS = 3;
const int NEPTUNE = 4;
const int EARTH = 5;
const int VENUS = 6;
const int MARS = 7;
const int MERCURY = 8;
const int PLUTO = 9;

struct body
{
 double x;
 double y;
 double mass;
};

body* nBodies;
int numBodies;

void initBodies( pair <double, double>[] );
void calcForce( int, double&, double& );
double calcDistance( int, int );
void outputPositions();
void master();
void slave();

int main()
{
 // set number of bodies to processor argument
 numBodies = 10;
 int t, i;
 double xForce, yForce;
 // velocities currently in seconds
 pair <double, double> velocity[numBodies];
 pair <double, double> newVel[numBodies];
 pair <int, int> newPos[numBodies];

 // initialize bodies
 initBodies( velocity );
 outputPositions();

 // for each time step
 for( t = 0; t < RUN_TIME; t++ )
 {
//  outputPositions();

    // for each body
    for( i = 0; i < numBodies; i++ )
    {
       // calculate forces
       calcForce( i, xForce, yForce );

       // calculate new velocities
       newVel[i].first =  velocity[i].first + xForce  / nBodies[i].mass;
       newVel[i].second =  velocity[i].second + yForce  / nBodies[i].mass;

       // calculate new positions
       newPos[i].first = nBodies[i].x + newVel[i].first;
       newPos[i].second = nBodies[i].y + newVel[i].second;
    }
    for( i = 0; i < numBodies; i++ )
    {
       // store new position
       nBodies[i].x = newPos[i].first;
       nBodies[i].y = newPos[i].second;

       // store new velocities
       velocity[i].first = newVel[i].first;
       velocity[i].second = newVel[i].second;
    }
 }
 outputPositions();

 // for ||

    // slave

    // master

    // push image

 // write animation

 // shut down
 delete[] nBodies;
 nBodies = NULL;

 system( "pause" );
 return 0;
}


void initBodies( pair <double, double> velocity[] )
{

 // create new struct
 nBodies = new body[numBodies];

 // SUN
 nBodies[SUN].x = SUN_X;
 nBodies[SUN].y = SUN_Y;
 nBodies[SUN].mass = 1989100000;
 velocity[SUN].first = 0;
 velocity[SUN].second = 0;

 // JUPITER
 nBodies[JUPITER].x = SUN_X+78;
 nBodies[JUPITER].y = SUN_Y;
 nBodies[JUPITER].mass = 1898600;
 velocity[JUPITER].first = 3.62;
 velocity[JUPITER].second = 3.62;

 // SATURN
 nBodies[SATURN].x = SUN_X+143;
 nBodies[SATURN].y = SUN_Y;
 nBodies[SATURN].mass = 568460;
 velocity[SATURN].first = 3.11;
 velocity[SATURN].second = 3.11;

 // URANUS
 nBodies[URANUS].x = SUN_X+287;
 nBodies[URANUS].y = SUN_Y;
 nBodies[URANUS].mass = 86832;
 velocity[URANUS].first = 2.61;
 velocity[URANUS].second = 2.61;

 // NEPTUNE
 nBodies[NEPTUNE].x = SUN_X+451;
 nBodies[NEPTUNE].y = SUN_Y;
 nBodies[NEPTUNE].mass = 102430;
 velocity[NEPTUNE].first = 2.33;
 velocity[NEPTUNE].second = 2.33;

 // EARTH
 nBodies[EARTH].x = SUN_X+15;
 nBodies[EARTH].y = SUN_Y;
 nBodies[EARTH].mass = 5973;
 velocity[EARTH].first = 5.46;
 velocity[EARTH].second = 5.46;

 // VENUS
 nBodies[VENUS].x = SUN_X+11;
 nBodies[VENUS].y = SUN_Y;
 nBodies[VENUS].mass = 4868;
 velocity[VENUS].first = 5.92;
 velocity[VENUS].second = 5.92;

 // MARS
 nBodies[MARS].x = SUN_X+23;
 nBodies[MARS].y = SUN_Y;
 nBodies[MARS].mass = 642;
 velocity[MARS].first = 4.91;
 velocity[MARS].second = 4.91;

 // MERCURY
 nBodies[MERCURY].x = SUN_X+6;
 nBodies[MERCURY].y = SUN_Y;
 nBodies[MERCURY].mass = 330;
 velocity[MERCURY].first = 6.92;
 velocity[MERCURY].second = 6.92;

 // PLUTO
 nBodies[PLUTO].x = SUN_X+591;
 nBodies[PLUTO].y = SUN_Y;
 nBodies[PLUTO].mass = 13;
 velocity[PLUTO].first = 2.16;
 velocity[PLUTO].second = 2.16;

}


void calcForce( int index, double& xForce, double& yForce )
{
 int i;
 double distance;
 xForce=yForce=0;

 // for numBodies-1 (for index)
 for( i = 0; i < numBodies; i++ )
 {
    // exclude index
    if( i != index )
    {
     // get distance( index, i )
     distance = calcDistance( index, i );

     // calculate x force 
     xForce += ( G * nBodies[index].mass * nBodies[i].mass * ( nBodies[index].x - nBodies[i].x ))/( distance*distance*distance );

     // calculate y force
     yForce += ( G * nBodies[index].mass * nBodies[i].mass * ( nBodies[index].y - nBodies[i].y ))/( distance*distance*distance );
    }
 }
}


double calcDistance( int i1, int i2 )
{
 double diff1, diff2, square1, square2;

 // get first difference
 diff1 = nBodies[i2].x - nBodies[i1].x;

 // get second difference
 diff2 = nBodies[i2].y - nBodies[i1].y;

 // get first square
 square1 = diff1 * diff1;

 // get second square
 square2 = diff2 * diff2;

 // return square root
 return sqrt( square1 + square2 );
}

void outputPositions()
{
 // for each body
    // display index: x position, y position, endline
 // for numBodies
 for( int i = 0; i < numBodies; i++ )
 {
  // body number
  cout << i+1 << ": ";

  cout << nBodies[i].x << ", ";

  cout << nBodies[i].y << ", ";

  cout << nBodies[i].mass << endl;
 }
}
