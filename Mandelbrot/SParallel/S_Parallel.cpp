#include <iostream>
#include <fstream>
#include <mpi.h>
using namespace std;

int const MAX_ITER = 256;

int const HEIGHT = 1100;
int const WIDTH = 1100;

double const REAL_MIN = -2;
double const IMAG_MIN = -2;
double const REAL_MAX = 2;
double const IMAG_MAX = 2;

double const SCALE_REAL = ( REAL_MAX - REAL_MIN )/( WIDTH - 1 );
double const SCALE_IMAG = ( IMAG_MAX - IMAG_MIN )/( HEIGHT - 1);

int const DATA_TAG = 0;
int const DEL_TAG = 1;

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
void writeImage( int image[][WIDTH] );
void master( int image[][WIDTH] );
void slave();

int main( int argc, char *argv[] )
{
 int image [HEIGHT][WIDTH];
 int myId;
 
 MPI_Init( &argc, &argv );
 MPI_Comm_rank( MPI_COMM_WORLD,&myId );

 if( myId == 0 )
     master( image );
 else
     slave();

// writeImage( image );

 MPI_Finalize();
 return 0;
}



void master( int image[][WIDTH] )
{
 int numProcs, rowRange, numPix, procID;
 int i, j, k, l, rowStart, rowEnd;
 double start, end;

 start = MPI_Wtime();

 MPI_Status stat;
 MPI_Comm_size( MPI_COMM_WORLD, &numProcs );

                   // numSlaves
 rowRange =( HEIGHT/( numProcs-1 )) +1;
 int colors[rowRange][WIDTH];
 numPix = rowRange * WIDTH;

 for( k=0; k<numProcs-1; k++ )
 {
  MPI_Recv( &colors, numPix, MPI_INT, MPI_ANY_SOURCE, 
            DATA_TAG, MPI_COMM_WORLD, &stat);

  procID = stat.MPI_SOURCE;
  
  // set proper row end so it doesn't go out of bounds
  if( procID == numProcs-1 )
   rowEnd = HEIGHT;
  else
   rowEnd = rowRange * procID;

  rowStart =( procID-1 )* rowRange;

  for( l=0, i=rowStart; i<rowEnd; i++, l++ )
  {
   for( j=0; j<WIDTH; j++ )
   {
    image[i][j] = colors[l][j];
   }
  }
 }
 end = MPI_Wtime();

 // display run time
 cout <<HEIGHT<<"x"<<WIDTH<<" Runtime: "<<( end - start ) <<endl;
}


void slave()
{
 int numProcs, numSlaves, numColors, myID;
 int i, j, k, rowRange,  rowStart, rowEnd;
 complex c;

 MPI_Status stat;
 MPI_Comm_size( MPI_COMM_WORLD, &numProcs );
 MPI_Comm_rank(MPI_COMM_WORLD,&myID);

 numSlaves = numProcs-1;
 rowRange =( HEIGHT / numSlaves )+1;
 int colors[rowRange][WIDTH];
 numColors = rowRange * WIDTH;
 
 // set proper row end so it doesn't go out of bounds
 if( myID == numSlaves )
  rowEnd = HEIGHT;
 else
  rowEnd = rowRange * myID;

 rowStart = ( myID-1 )* rowRange;

 for( k=0, i=rowStart; i<rowEnd; i++, k++  )
 {
  c.imag = IMAG_MIN + ( i * SCALE_IMAG );
  for( j=0; j<WIDTH; j++ )
  {
   c.real = REAL_MIN + ( j * SCALE_REAL );
   colors[k][j] = calcPixel( c );
  }
 } 
 MPI_Send(colors, numColors, MPI_INT, 0, DATA_TAG, MPI_COMM_WORLD);
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
    // the next Z.real becomes a^2 - b^2 + c
    temp = Z.real * Z.real - Z.imag * Z.imag + c.real;

    // the next Z.imag becomes 2abi+ci
    Z.imag = 2 * Z.real * Z.imag + c.imag;

    Z.real = temp;

    // determine the magnitude of Z 
    lengthSqd = Z.real * Z.real + Z.imag * Z.imag;

    count++;
 } while(( lengthSqd < 4.0 ) && ( count < MAX_ITER ));

 return count;
}



void writeImage (int image[][WIDTH])
{
 int x, y;
 ofstream fout;
 char fname[15] = "panda.PGM";

 fout.open( fname );

 if (!fout) {
   cout << "Can't open file: " << fname << endl;
   return;
 }

 fout << "P2" << endl;
 fout << WIDTH << " " << HEIGHT << endl;
 fout << 255 << endl;

 for(x=0; x<WIDTH; x++)
 {
  for(y=0; y<HEIGHT; y++)
  {
   fout<<image[x][y]<<" ";
  }
  fout<<endl;
 }

 if (fout.fail()) {
   cout << "Can't write image " << fname << endl;
   return;
 }

 fout.close();
}
