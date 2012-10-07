#include <iostream>
#include <mpi.h>
using namespace std;

int d;

const int MAX = 2000;

int matA[MAX][MAX];
int matB[MAX][MAX];
int matC[MAX][MAX];

int const DATA_TAG = 0;
int const DEL_TAG = 1;

void master();
void slave();
void loadMatrix( int[][MAX], int );
void viewMatrix( int[][MAX] );

int main( int argc, char** argv )
{
 int myId;
 
 MPI_Init( &argc, &argv );
 MPI_Comm_rank( MPI_COMM_WORLD,&myId );

 loadMatrix( matB, 1 );

 //broadcast matB to all the slaves
 MPI_Bcast(&matB, d*d, MPI_INT, 0, MPI_COMM_WORLD);

 d = atoi( argv[1] );

 if( myId == 0 )
     master();
 else
     slave();

// viewMatrix( matA );
// viewMatrix( matB );
 viewMatrix( matC );

 MPI_Finalize();
 return 0;
}

void master()
{
 int numProcs, i, row, numRows=0, rcvd;
 double start, end;
 MPI_Status stat;

 loadMatrix( matA, 2 );
 loadMatrix( matC, 0 );

 start = MPI_Wtime();

 MPI_Comm_size( MPI_COMM_WORLD, &numProcs );

 // send out initial rows
 for( i=1, row=0; i<numProcs; i++, row++ )
 {
  // loop to send out pieces of matA
  MPI_Send( &row, 1, MPI_INT, i, DATA_TAG, MPI_COMM_WORLD );
  MPI_Send( &matA[row][0], d, MPI_INT, i, DATA_TAG, MPI_COMM_WORLD );
 }

 // loop to receive matC
 while( numRows<d )
 {
  MPI_Recv( &rcvd, 1, MPI_INT, MPI_ANY_SOURCE, DATA_TAG, MPI_COMM_WORLD, &stat );
  MPI_Recv( &matC[rcvd][0], d, MPI_INT, MPI_ANY_SOURCE, DATA_TAG, MPI_COMM_WORLD, &stat );

  numRows++;

  // loop to send out pieces of matA
  MPI_Send( &row, 1, MPI_INT, i, DATA_TAG, MPI_COMM_WORLD );
  MPI_Send( &matA[row][0], d, MPI_INT, i, DATA_TAG, MPI_COMM_WORLD );
  row++;
 }

 for( i=1; i<numProcs; i++ )
 {
  MPI_Send( 0, 0, MPI_INT, i, DEL_TAG, MPI_COMM_WORLD );
  MPI_Send( &matA[0][0], d, MPI_INT, i, DEL_TAG, MPI_COMM_WORLD );
 }

 end = MPI_Wtime();

 // display run time
 cout <<d<<" Runtime: "<<( end - start ) <<endl;
}

void slave()
{
 int i, j, col;
 int incoming[d], outgoing[d];
 MPI_Status stat;
 
 MPI_Recv( &col, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &stat);
 MPI_Recv( &incoming, d, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &stat);

 while( stat.MPI_TAG == DATA_TAG )
 {
  // reset outgoing matrix to zero
  for( i=0; i<d; i++ )
  {
   outgoing[i] = 0;
  }

  for( i=0; i<d; i++ )
  {
   for( j=0; j<d; j++ )
   {
    outgoing[i] += incoming[j]*matB[j][col];
   }
  }
  
  MPI_Send( &col, 1, MPI_INT, 0, DATA_TAG, MPI_COMM_WORLD );
  MPI_Send( &outgoing, d, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &stat );
 
  MPI_Recv( &col, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &stat);
  MPI_Recv( &incoming, d, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &stat);
 }

 if( stat.MPI_TAG == DEL_TAG )
  return;
}

void loadMatrix( int matrix[][MAX], int value )
{
 int i, j;

 for ( i=0; i<d; i++ )
 {
  for( j=0; j<d; j++ )
  {
   matrix[i][j] = value;
  }
 }
}

void viewMatrix( int matrix[][MAX] )
{
 int i, j;

 for ( i=0; i<d; i++ )
 {
  for( j=0; j<d; j++ )
  {
   cout<<matrix[i][j]<<" ";
  }
  cout<<endl;
 }
}
