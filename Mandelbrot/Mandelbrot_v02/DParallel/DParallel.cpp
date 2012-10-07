#include <iostream>
#include <fstream>
#include <mpi.h>
using namespace std;

int const MAX_ITER = 256;

int const HEIGHT = 9000;
int const WIDTH = 9000;

double const REAL_MIN = -2;
double const IMAG_MIN = -2;
double const REAL_MAX = 2;
double const IMAG_MAX = 2;

double const SCALE_REAL = ( REAL_MAX - REAL_MIN )/( WIDTH - 1 );
double const SCALE_IMAG = ( IMAG_MAX - IMAG_MIN )/( HEIGHT - 1);

int const DATA_TAG = 0;
int const DEL_TAG = 1;

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
void writeImage( );
void master( );
void slave();

int main( int argc, char *argv[] )
{
 int myId;
 
 MPI_Init( &argc, &argv );
 MPI_Comm_rank( MPI_COMM_WORLD,&myId );
 
 if( myId == 0 )
     master();
 else
     slave();

// writeImage();

 MPI_Finalize();
 return 0;
}



void master()
{
 int numProcs, procID, tempRow;
 int i, row=0, count=HEIGHT;
 int colors[WIDTH];
 double start, end;

 start = MPI_Wtime();

 MPI_Status stat;
 MPI_Comm_size( MPI_COMM_WORLD, &numProcs );

 // lookup table based on processor ID
 // keeps track of row numbers
 int rowLookUp[numProcs];

 // send out initial rows
 for( i=1; i<numProcs; i++ )
 {
  MPI_Send( &row, 1, MPI_INT, i, DATA_TAG, MPI_COMM_WORLD ); 
  rowLookUp[i]=row; 
  row++;
 }

 // save rows and send more as necessary
 do{
  MPI_Recv( &colors, WIDTH, MPI_INT, MPI_ANY_SOURCE, DATA_TAG, MPI_COMM_WORLD, &stat);

  procID = stat.MPI_SOURCE;
  tempRow = rowLookUp[procID];
  
  if( tempRow<HEIGHT  )
  {
   for( i=0; i<WIDTH; i++ )
    image[tempRow][i] = colors[i];
  
   // send another row
   MPI_Send( &row, 1, MPI_INT, procID, DATA_TAG, MPI_COMM_WORLD );
   rowLookUp[procID]=row; 
   row++;
  }
 }while( tempRow<HEIGHT );

 for( i=1; i<numProcs; i++ )
  MPI_Send( 0, 0, MPI_INT, i, DEL_TAG, MPI_COMM_WORLD );

 end = MPI_Wtime();

 // display run time
 cout <<HEIGHT<<"x"<<WIDTH<<" Runtime: "<<( end - start )<<endl;
}


void slave()
{
 int colors[WIDTH], row;
 int i;
 complex c;
 MPI_Status stat;

 MPI_Recv( &row, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &stat );

 while( stat.MPI_TAG == DATA_TAG )
 {
  c.imag = IMAG_MIN + ( row * SCALE_IMAG );
  for( i=0; i<WIDTH; i++ )
  {
   c.real = REAL_MIN + ( i * SCALE_REAL );
   colors[i] = calcPixel( c );
  }
  
  MPI_Send( colors, WIDTH, MPI_INT, 0, DATA_TAG, MPI_COMM_WORLD );
  MPI_Recv( &row, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &stat );
 }

 if( stat.MPI_TAG == DEL_TAG )
  return;
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



void writeImage ()
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

 for(x=0; x<HEIGHT; x++)
 {
  for(y=0; y<WIDTH; y++)
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
