#include <iostream>
#include <math.h>
#include <sys/time.h>
#include <mpi.h>
#include <stdlib.h>
using namespace std;

const int MAX_MASS = 1000;
const double G = 6.673;
const int X = 0;
const int Y = 1;

int RUN_TIME;
int HEIGHT;
int WIDTH;
int NUM_BODS;

pair< int, int > initBodies( int& );
void display( int[], int[] );
pair< double, double >calcForce( int, int[][2], int[] );
double calcDistance( int, int, int[] );

int main( int argc, char** argv[] )
{
    NUM_BODS = atoi( argv[1] );
    HEIGHT = WIDTH = atoi( argv[2] );
    RUN_TIME = atoi( argv[3] );

	int myId, numProcs, error, mass, t;
	bool initFlag = true;
    int masses[NUM_BODS], positions[NUM_BODS][2];
    pair< int, int > newPos;
    pair< double, double >force, velocity( 0.0, 0.0 ), newVel;
	double start, end;

	error = MPI_Init( &argc, &argv );
    if( error != MPI_SUCCESS )
    {
     cout<<"Intialize didn't work: "<<error<<endl;
     MPI_Abort( MPI_COMM_WORLD, error );
    }

	MPI_Status stat;
	MPI_Comm_rank( MPI_COMM_WORLD, &myId );
	MPI_Comm_size( MPI_COMM_WORLD, &numProcs );
	
	start = MPI_Wtime();	

	for( t = 0; t < RUN_TIME; t++ )
	{
		// if this is the first time the program has run
	    if( initFlag )
		{
			// initialize positions and masses
			newPos = initBodies( mass );

			// send out masses
			MPI_Allgather( mass, 1, MPI_INT, masses, NUM_BODS, MPI_INT, MPI_COMM_WORLD );

			initFlag = false;
		}
            		
		// send out positions
        MPI_Allgather( newPos, 2, MPI_INT, positions, NUM_BODS*2, MPI_INT, MPI_COMM_WORLD );

        // calculate force on this body
        force = calcForce( myId, positions, masses );

        // calculate new velocity for this body
        newVel.first = velocity.first + force.first / masses[myId-1];
        newVel.second = velocity.second + force.second / masses[myId-1];

        // calculate new positions
        newPos.first = positions[myId-1][X] + newVel.first;
        newPos.second = positions[myId-1][Y] + newVel.second;

		// save new velocities
		velocity.first = newVel.first;
		velocity.second = newVel.second;

		// check for out of bounds and reset
		if( newPos.first < 0 )
			newPos.first = 0;
		else if( newPos.first > HEIGHT )
			newPos.first = HEIGHT;
			
		if( newPos.second < 0 )
			newPos.second = 0;
		else if( newPos.second > HEIGHT )
			newPos.second = HEIGHT;
		}
	}
	end = MPI_Wtime();

	if( myId == 0 )
        cout<<( end-start )<<endl;

	MPI_Finalize();
	return 0;
}



pair< int, int > initBodies( int& mass )
{
	pair< int, int > newPos;

	// initialize random seed
	srand( time( NULL ));

    // x position
	newPos.first = rand() % WIDTH;

    // y position
	newPos.second = rand() % HEIGHT;

	mass = rand() % MAX_MASS + 1;

    return newPos;
}



void display( int positions[], int masses[] )
{
	for( int i = 0; i < NUM_BODS; i++ )
	{
		if( i % 2 == 0 )
			cout<<masses[i/2]<<", ";

		cout<<positions[i];

		if( i % 2 == 0 )
			cout<<", ";
		else
			cout<<endl;
	}
}


pair< double, double >calcForce( int index, int positions[][2], int masses[] )
{
 int i;
 double distance;
 pair< double, double >force( 0.0, 0.0 );

 // for numBodies-1 (exclude index)
 for( i = 1; i < NUM_BODS+1; i++ )
 {
    // exclude index
    if( i != index )
    {
     // get distance( index, i )
     distance = calcDistance( index, i, positions );

     // calculate x force 
     force.first += ( G * masses[index-1] * masses[i-1] * ( positions[i-1][X] - positions[index-1][X] ))/( distance*distance*distance );

     // calculate y force
     force.second += ( G * masses[index-1] * masses[i-1] * ( positions[i-1][Y] - positions[index-1][Y] ))/( distance*distance*distance );
    }
 }
 return force;
}


double calcDistance( int i1, int i2, int positions[][2] )
{
 int diff1, diff2, square1, square2;

 // get first difference
 diff1 = positions[2*i2-2] - positions[2*i1-2];

 // get second difference
 diff2 = positions[2*i2-1] - positions[2*i1-1];

 // get first square
 square1 = diff1 * diff1;

 // get second square
 square2 = diff2 * diff2;

 // return square root
 return sqrt( square1 + square2 );
}
