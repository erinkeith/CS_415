#include <list>
#include <vector>
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <Magick++.h>

using namespace std;
using namespace Magick;

// Define global cosntants
int ImageHeight = 1000;
int ImageWidth = 1000;
int RunTime = 100;
const int NBodies = 10;
const int TimeStep = 1;
const double gravConst = 6.6;
const int massConstant = 300;
bool DEBUG = false;

// Body structure
struct Body {
	int xPos;
	int yPos;
	double mass;
//	int position[RunTime];
};

// Pointers for manipulation
Body *bodyPtr;
Image *imagePtr;

// function prototypes
pair<double, double>  calcForce( int );

double calcDistance( Body, Body );

void initBodies();

void display();

void pushFrame( int );

//Create an STL list of Magick::Image objects
list<Image> imageList;

int main( int argc, char** argv ) {

	// check for correct number of args
	if( argc < 1 ) {
		cout << "Not enough arguments!" << endl;
		exit(0);
	}
	
	// seed for random number generator
	srand( time(NULL) );	

	// set run time and make an image array
	RunTime = atoi( argv[1] );

	// initialize all temporary arguments
	pair<double, double> oldVelocity[NBodies],
								newVelocity[NBodies];
	pair<int, int> newPosition[NBodies];
	pair<double, double> Force( 0.0, 0.0 );

	// initialize all N Bodies
	initBodies();


	// outter loop to iterate through RunTime
	for( int t = 0; t < RunTime; t++ ) {

		cout << "New Iteration..." << endl;		

		// inner loop to iterate through Number of Bodies
		for( int n = 0; n < NBodies; n++ ) {
		
			// calculate force on i'th body
			Force = calcForce( n );

			// calculate velocity					
			
				// calculate x component of velocity vector
					newVelocity[n].first = oldVelocity[n].first + Force.first * ( TimeStep / bodyPtr[n].mass );

				// calculate y component of velocity vector
					newVelocity[n].second = oldVelocity[n].second + Force.second * ( TimeStep / bodyPtr[n].mass );				

			// calculate new position using velocity vector

				// calculate x direction of position
					newPosition[n].first = bodyPtr[n].xPos + newVelocity[n].first * TimeStep;

				// calculate y direction of poition
					newPosition[n].second = bodyPtr[n].yPos + newVelocity[n].second * TimeStep;
				
					cout << newPosition[n].first << " " << newPosition[n].second << endl;
		}

		for( int n = 0; n < NBodies; n++ ) {
	
			// set n'th body position equal to new position
			
				// set x component
					bodyPtr[n].xPos = newPosition[n].first;					

				// set y component
					bodyPtr[n].yPos = newPosition[n].second;

			// set velocity to the new velocity
				
				// set x component
					oldVelocity[n].first = newVelocity[n].first;					

				// set y component
					oldVelocity[n].second = newVelocity[n].second;	

		}

		// push a new frame after each time step
			pushFrame( t );
	}


   writeImages( imageList.begin(), imageList.end(), "animation.gif" );
	 return 0;
}

// function prototypes
void initBodies() {
	
	// initialize number of bodies
		bodyPtr = new Body[NBodies];

	// initialize x and y positions and masses of bodies
		for( int i = 0; i < NBodies; i++ ) {						
			bodyPtr[i].xPos = rand() % ImageWidth;
			bodyPtr[i].yPos = rand() % ImageHeight;
			bodyPtr[i].mass = rand() % massConstant + 1;		

			if(DEBUG) {		
				cout << "Body: " << i;	
				cout << "xPos: " << bodyPtr[i].xPos << " yPos: " << bodyPtr[i].yPos << endl;
				cout << bodyPtr[i].mass << endl;
			}
		}
}

pair<double, double> calcForce( int n ) {

	// initalize temporary variables
		pair<double, double> forceSum(0.0, 0.0), tempForce;
		double tempDist;

	// loop over every other body and calc forces	
		for( int i = 0; i < NBodies; i++ ) {

			// if on current body, skip it
			if ( i == n ) {
				continue;
			}
	
			// calculate forces applied on body n 

				// calculate temp distance between the two bodies
					tempDist = calcDistance( bodyPtr[i], bodyPtr[n] );
 
				// calculate force in the x direction
					tempForce.first = ( gravConst*bodyPtr[i].mass*bodyPtr[n].mass*(bodyPtr[i].xPos - bodyPtr[n].xPos) ) / pow(tempDist, 3);										
				 
				// calculate force in the y direction
					tempForce.second = ( gravConst*bodyPtr[i].mass*bodyPtr[n].mass*(bodyPtr[i].yPos - bodyPtr[n].yPos) ) / pow(tempDist, 3);
			
				// accumulate force 	
					forceSum.first += tempForce.first; 
					forceSum.second += tempForce.second;				
		}

	// return force
	return forceSum;	
}

double calcDistance( Body one, Body two ) {
	// find distance between Body one and Body two

		// sqrt( (x2-x1)^2 + (y2-y1)^2 )
	return sqrt( pow( (two.xPos - one.xPos), 2 ) + pow( (two.yPos - one.yPos), 2 ) ); 
}

void pushFrame( int index ) {

	// initialize temporary variables
		pair<int, int> pos;
		double radius = 0.0;
	
	// create an image to push onto list
		Image image( Geometry( ImageHeight, ImageWidth ), Color( "black" ) );

	// loop over the number of bodies
		for( int i = 0; i < NBodies; i++ ) {

			// set color of Body
				image.fillColor( "white" );

			// get Body position
				pos.first = bodyPtr[i].xPos;
				pos.second = bodyPtr[i].yPos;

			// check if position is out of bounds
				if(pos.first < 0 )
					pos.first = 0;					
				if(pos.second < 0)
					pos.second = 0;

			// scale radius
				radius = bodyPtr[i].mass/20;

			// draw body!
				image.draw( DrawableCircle( pos.first, pos.second, pos.first+radius, pos.second+radius ) );

		}
	// push Image onto list
		imageList.push_back( image );
}

void display() {

	// use temporary image container
/*	Image *tempImages[RunTime];

	// allocate images equal to run time and initalize to white background
	for( int n = 0; n < RunTime; n++ ) {		
		tempImages[n] = new Image( Geometry(100,100), Color(MaxRGB, MaxRGB, MaxRGB, 0));
   	tempImages[n]->animationDelay(10);
		tempImages[n]->modifyImage();
		tempImages[n]->type( TrueColorType );
	}

	// pixel packets are for individual pixel manipulation
	PixelPacket *pixel_cache[RunTime];
	PixelPacket *pixel[RunTime];
	// col and row of pixel to draw/change
	int column = 50;
	int row = 0;

	for( int n = 0; n < RunTime; n++ ) {
		// draw all bodies on image n
		pixel_cache[n] = tempImages[n]->getPixels(0, 0, 100, 100);
		pixel[n] = pixel_cache[n] + row * 100 + column;
		*pixel[n] = Color( "red" );
		row += 2;
		// sync and push images to main list
		tempImages[n]->syncPixels();
	   imageList.push_back(*(tempImages[n]) );	
	}

	// easy way to write all images
   writeImages( imageList.begin(), imageList.end(), "animation.gif" );
*/
}

