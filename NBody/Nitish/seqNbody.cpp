#include <list>
#include <Magick++.h>
#include <iostream>
#include <string.h>
#include <stdlib.h>

using namespace std;
using namespace Magick;

// Define global cosntants
int ImageHeight = 0;
int ImageWidth = 0;
int RunTime = 0;
const int NBodies = 10;
const int TimeStep = 1;

//Create an STL list of Magick::Image objects
list<Image> imageList;0

int main( int argc, char** argv ) {

	// check for correct number of args
	if( argc < 3 ) {
		cout << "Not enough arguments!" << endl;
		exit(0);
	}

    InitializeMagick(*argv);
	
	// set image size and run time
	ImageHeight = atoi( argv[1] );
	ImageWidth = atoi( argv[2] );
	RunTime = atoi( argv[3] );

	// use temporary image container
	Image *tempImages[RunTime];

	// allocate images equal to run time and initalize to white background
	for( int n = 0; n < RunTime; n++ ) {		
		tempImages[n] = new Image( Geometry(ImageHeight,ImageWidth), Color(MaxRGB, MaxRGB, MaxRGB, 0));
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
		pixel_cache[n] = tempImages[n]->getPixels(0, 0, ImageHeight, ImageWidth);
		pixel[n] = pixel_cache[n] + row * ImageWidth + column;
		*pixel[n] = Color( "red" );
		row += 2;
		// sync and push images to main list
		tempImages[n]->syncPixels();
	   imageList.push_back(*(tempImages[n]) );	
	}

	// easy way to write all images
   writeImages( imageList.begin(), imageList.end(), "animation.gif" );
	 return 0;
}
