#include <iostream>
#include <math.h>
#include <sys/time.h>
#include <mpi.h>
#include <stdlib.h>
using namespace std;

const int RUN_TIME = 4000;
const int HEIGHT = 10000;
const int WIDTH = 10000;
const int MAX_MASS = 1000;
const double G = 6.673;
const int DATA_TAG = 0;
const int NUM_BODS = 50;

void initBodies( int[], int[] );
void display( int[], int[] );
pair< double, double >calcForce( int, int[], int[] );
double calcDistance( int, int, int[] );

int main( int argc, char *argv[] )
{
	int myId, t, i, numProcs, procID;
	bool initFlag = true;
    	int masses[NUM_BODS], positions[NUM_BODS*2], newPos[2];
    	pair< double, double >force, velocity, newVel;
    	velocity.first = 0;
    	velocity.second = 0;
	double start, end;

	MPI_Init( &argc, &argv );
	MPI_Status stat;
	MPI_Comm_rank( MPI_COMM_WORLD, &myId );
	MPI_Comm_size( MPI_COMM_WORLD, &numProcs );
	
	start = MPI_Wtime();	

	for( t = 0; t < RUN_TIME; t++ )
	{
////////MASTER PROCESS////////
		if( myId == 0 )
		{
			// if this is the first time the master has run
			if( initFlag )
			{
				// initialize positions and masses
				initBodies( positions, masses );

				// for each processor, send a mass
				for( int i = 1; i < numProcs; i++ )
					MPI_Send( masses, NUM_BODS, MPI_INT, i, DATA_TAG, MPI_COMM_WORLD );

				initFlag = false;
			}
            		
			// pass positions to first slave
            		MPI_Send( positions, NUM_BODS*2, MPI_INT, 1, DATA_TAG, MPI_COMM_WORLD );

            		// for each slave
            		for( i = 1; i < numProcs; i++ )
            		{
                		// receive each new position
				MPI_Recv( &newPos, 2, MPI_INT, MPI_ANY_SOURCE, DATA_TAG, MPI_COMM_WORLD, &stat );             

                		procID = stat.MPI_SOURCE;

                		// save in positions array
                		positions[2*procID-2] = newPos[0];
                		positions[2*procID-1] = newPos[1];
            		}		
		}
////////SLAVE PROCESS////////
		else
		{
			if( initFlag )
			{
				// receive mass
				MPI_Recv( &masses, NUM_BODS, MPI_INT, 0, DATA_TAG, MPI_COMM_WORLD, &stat );

				initFlag = false;
			}

            		// receive positions from previous process
			MPI_Recv( &positions, NUM_BODS*2, MPI_INT, myId-1, DATA_TAG, MPI_COMM_WORLD, &stat );

            		// if this is not the last process
            		// send positions to next process
            		if( myId != numProcs-1 )
                		MPI_Send( positions, NUM_BODS*2, MPI_INT, myId+1, DATA_TAG, MPI_COMM_WORLD );

            		// calculate force on this body
            		force = calcForce( myId, positions, masses );

            		// calculate new velocity for this body
            		newVel.first = velocity.first + force.first / masses[myId-1];
            		newVel.second = velocity.second + force.second / masses[myId-1];

            		// calculate new positions
            		newPos[0] = positions[2*myId-2] + newVel.first;
            		newPos[1] = positions[2*myId-1] + newVel.second;

			// save new velocities
			velocity.first = newVel.first;
			velocity.second = newVel.second;

			// check for out of bounds and reset
			if( newPos[0] < 0 )
				newPos[0] = 0;
			else if( newPos[0] > HEIGHT )
				newPos[0] = HEIGHT;
			
			if( newPos[1] < 0 )
				newPos[1] = 0;
			else if( newPos[1] > HEIGHT )
				newPos[1] = HEIGHT;

            		// send new position to master
            		MPI_Send( newPos, 2, MPI_INT, 0, DATA_TAG, MPI_COMM_WORLD );
		}
	}
	end = MPI_Wtime();

	if( myId == 0 )
		cout<<"Parallel, "<<RUN_TIME<<": "<<(end-start)<<endl;

	MPI_Finalize();
	return 0;
}



void initBodies( int positions[], int masses[] )
{
	int i;

	// initialize random seed
	srand( time( NULL ));

	for( i = 0; i < 20; i++ )
	{
		positions[i] = rand() % WIDTH;
	}

	for( i = 0; i < 10; i++ )
		masses[i] = rand() % MAX_MASS + 1;

//	display( positions, masses );
}



void display( int positions[], int masses[] )
{
	for( int i = 0; i < 20; i++ )
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


pair< double, double >calcForce( int index, int positions[], int masses[] )
{
 int i;
 double distance;
 pair< double, double >force;

 force.first = 0;
 force.second = 0;

 // for numBodies-1 (exclude index)
 for( i = 1; i < NUM_BODS+1; i++ )
 {
    // exclude index
    if( i != index )
    {
     // get distance( index, i )
     distance = calcDistance( index, i, positions );

     // calculate x force 
     force.first += ( G * masses[index-1] * masses[i-1] * ( positions[2*i-2] - positions[2*index-2] ))/( distance*distance*distance );

     // calculate y force
     force.second += ( G * masses[index-1] * masses[i-1] * ( positions[2*i-1] - positions[2*index-1] ))/( distance*distance*distance );
    }
 }
 return force;
}


double calcDistance( int i1, int i2, int positions[] )
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
