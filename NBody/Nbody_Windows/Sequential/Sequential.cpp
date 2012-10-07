#include <iostream>
#include <cmath>
#include <time.h>
using namespace std;

const int RUN_TIME = 4000;
const int HEIGHT = 10000;
const int WIDTH = 10000;
const int MAX_MASS = 1000;
const double G = 6.673;

struct body
{
 int x;
 int y;
 double mass;
};

body* nBodies;
int numBodies;

void initBodies( pair< double, double >[] );
pair< double, double >calcForce( int );
double calcDistance( int, int );
void outputPositions();
void master();
void slave();

int main()
{
 // set number of bodies to processor argument
 numBodies = 10;
 int t, i;
 // first is x, second is y
 pair< double, double >force, velocity[numBodies], newVel[numBodies];
 pair< int, int >newPos[numBodies];

 // initialize bodies
 initBodies( velocity );
 outputPositions();

 // for each time step
 for( t = 0; t < RUN_TIME; t++ )
 {
    // for each body
    for( i = 0; i < numBodies; i++ )
    {
       // calculate forces
       force = calcForce( i );

       // calculate new velocities
       newVel[i].first = velocity[i].first + force.first / nBodies[i].mass;
       newVel[i].second = velocity[i].second + force.second / nBodies[i].mass;

       // calculate new positions
       newPos[i].first = nBodies[i].x + newVel[i].first;
       newPos[i].second = nBodies[i].y + newVel[i].second;
    }
    for( i = 0; i < numBodies; i++ )
    {
       // store new x position
       if( newPos[i].first < 0 )
        nBodies[i].x = 0;
       
       else if( newPos[i].first > WIDTH )
        nBodies[i].x = WIDTH;        

       else
        nBodies[i].x = newPos[i].first;
       
       // store new y position
       if( newPos[i].second < 0 )
        nBodies[i].y = 0;
       
       else if( newPos[i].second > HEIGHT )
        nBodies[i].y = HEIGHT;  
       
       else
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
 /* initialize random seed: */
 srand ( time(NULL) );

 // create new struct
 nBodies = new body[numBodies];

 // for numBodies
 for( int i = 0; i < numBodies; i++ )
 {
    // randomly generate x position
    nBodies[i].x = rand() % WIDTH;

    // randomly generate y position
    nBodies[i].y = rand() % HEIGHT;

    // randomly generate mass
    nBodies[i].mass = rand() % MAX_MASS + 1;

    velocity[i].first = 0;
    velocity[i].second = 0;
 }
}


pair< double, double >calcForce( int index )
{
 int i;
 double distance;
 pair< double, double >force;

 force.first = 0;
 force.second = 0;

 // for numBodies-1 (exclude index)
 for( i = 0; i < numBodies; i++ )
 {
    // exclude index
    if( i != index )
    {
     // get distance( index, i )
     distance = calcDistance( index, i );

     // calculate x force 
     force.first += ( G * nBodies[index].mass * nBodies[i].mass * ( nBodies[i].x - nBodies[index].x ))/( distance*distance*distance );

     // calculate y force
     force.second += ( G * nBodies[index].mass * nBodies[i].mass * ( nBodies[i].y - nBodies[index].y ))/( distance*distance*distance );
    }
 }
 return force;
}


double calcDistance( int i1, int i2 )
{
 int diff1, diff2, square1, square2;

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
