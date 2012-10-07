#include <iostream>
#include <mpi.h>
using namespace std;

void master();
void slave();

int main( int argc, char *argv[] )
{
	int myId;
	MPI_Init( &argc, &argv );
	MPI_Comm_rank( MPI_COMM_WORLD, &myId );

	if( myId == 0 )
	{
		master();
	}

	else
	{
		slave();
	}

	MPI_Finalize();
	return 0;
}

void master()
{
	cout<<"MASTER"<<endl;
}

void slave()
{
	cout<<"SLAVE"<<endl;
}
