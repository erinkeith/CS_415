#include <iostream>
#include <fstream>
#include <time.h>

using namespace std;

int const RUN_TIME_ITER = 100;
int const MAX_ITER = 256;
int const HEIGHT = 10000;
int const WIDTH = 10000;
double const REAL_MIN = -2;
double const IMAG_MIN = -2;
double const REAL_MAX = 2;
double const IMAG_MAX = IMAG_MIN+(REAL_MAX - REAL_MIN)*HEIGHT/WIDTH;
double const SCALE_REAL = ( REAL_MAX - REAL_MIN )/( WIDTH - 1 );
double const SCALE_IMAG = ( IMAG_MAX - IMAG_MIN )/( HEIGHT - 1);

int image [HEIGHT][WIDTH];

// to handle complex numbers a+bi
struct complex
{
 float real;
 float imag;
};

/* uses function to compute 
   Z(n+1) = Z(n)^2 + c
   where c is the complex coord of pixel in plane
   returns number of iterations, bounded by MAX_ITER
*/
int calcPixel( complex c );
void writeImage();

int main()
{
 int x, y;
 complex c;
 clock_t start, end; // for timing

 start = clock();

 for( y=0; y<HEIGHT; y++ )
 {
  c.imag = IMAG_MIN + ( y * SCALE_IMAG);
  for( x=0; x<WIDTH; x++ )
  {
   c.real = REAL_MIN + ( x * SCALE_REAL);
   image[x][y] = calcPixel( c );
  }
 }

 end = clock();

 writeImage();

 // display run time
cout<<HEIGHT<<"x"<<WIDTH<<" Runtime: "
    <<((double)(end-start)/(double)CLOCKS_PER_SEC)/100<<" seconds"<<endl;
 return 0;
}

int calcPixel( complex c )
{
 int count;         // keep track of iterations, becomes pixel value
 complex Z;
 float temp, lengthSqd;

 Z.real = 0;
 Z.imag = 0;
 count = 0;

 do{
    temp = Z.real * Z.real - Z.imag * Z.imag + c.real;

    // the next Z.imag becomes 2abi+ci
    Z.imag = 2 * Z.real * Z.imag + c.imag;

    // the next Z.real becomes a^2 - b^2 + c
    Z.real = temp;

    // determine the magnitude of Z 
    lengthSqd = Z.real * Z.real + Z.imag * Z.imag;

    count++;
 } while(( lengthSqd < 4.0 ) && ( count < MAX_ITER ));

 return count;
}



void writeImage()
{
 int x, y;
 ofstream fout;
 char fname[15] = "panda.PGM";

 fout.open( fname );

 if (!fout) {
   cout << "Can't open file: " << fname << endl;
   exit(1);
 }

 fout << "P2" << endl;
 fout << WIDTH << " " << HEIGHT << endl;
 fout << 255 << endl;

 for(x=0; x<WIDTH; x++)
 {
  for(y=0; y<HEIGHT; y++)
  {
   fout<<image[y][x]<<" ";
  }
  fout<<endl;
 }

 if (fout.fail()) {
   cout << "Can't write image " << fname << endl;
   exit(0);
 }

 fout.close();
}
