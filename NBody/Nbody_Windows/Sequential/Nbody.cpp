#include <iostream>
#include <cmath>
#include <time.h>
#include <Magick++.h>

using namespace std;
using namespace Magick;

const int RUN_TIME = 4000;
const int HEIGHT = 10000;
const int WIDTH = 10000;
const int MAX_MASS = 1000;
const double G = 6.673;

// a body has x position, y position, and mass
struct body
{
 int x;
 int y;
 double mass;
};

// to dynamically declare array of bodies
body* nBodies;
int numBodies;

// list of images for gif animation
list<Image> animation;

// initialize array of bodies to random attributes
// and velocities
void initBodies( pair< double, double >[] );

// calculates total force on bodies
// and returns x force and y force
pair< double, double >calcForce( int );

// calculates distance between two bodies
double calcDistance( int, int );

// makes one image to push onto list
void makeImage( int );

// empty functions for parallel planning
void master();
void slave();

int main( int argc, char** argv )
{
 // check for correct number of args
 if( argc < 1 ) 
     {
	  cout << "Need number of bodies" << endl;
	  exit(0);
     }
 // set number of bodies to processor argument
 numBodies = atoi( argv[0] );
 int t, i;
 // first is x, second is y
 pair< double, double >force, velocity[numBodies], newVel[numBodies];
 pair< int, int >newPos[numBodies];

 // initialize bodies
 initBodies( newVel );

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

    // make image and push onto list
	makeImage( t );
 }

 // for ||

    // slave

    // master

 // write animation
 writeImages( imageList.begin(), imageList.end(), "nBodies.gif" );

 // shut down
 delete[] nBodies;
 nBodies = NULL;

 return 0;
}


void initBodies( pair <double, double> newVel[] )
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

 // for numBodies-1 (for index)
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


void makeImage( int )
{
 // radius of circle to be drawn onto image
 double radius;
 int i;

 // temporary image for new frame
 Image temp( Geometry( HEIGHT/10, WIDTH/10 ), Color( "black" ));

 // for every body
 for( i = 0; i < numBodies; i++ ) 
 {
  // set color of Body
  temp.fillColor( "white" );

  // scale radius
  radius = nBodies[i].mass/20;

  // draw body!
  temp.draw( DrawableCircle( nBodies[i].x/10, nBodies[i].y/10, 
              nBodies[i].x/10+radius, nBodies[i].y/10+radius ));
 }

 // push image onto list
 animation.push_back( temp );
}
