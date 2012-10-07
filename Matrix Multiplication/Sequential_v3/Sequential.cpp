#include <iostream>
using namespace std;

int d;

const int MAX = 2000;

int matA[MAX][MAX];
int matB[MAX][MAX];
int matC[MAX][MAX];

void loadMatrix( int[][MAX], int );
void viewMatrix( int[][MAX] );

int main( int argc, char** argv )
{
 int i, j, k;
 clock_t start, end;

 d = atoi( argv[1] );

 loadMatrix( matA, 2 );
 loadMatrix( matB, 1 );
 loadMatrix( matC, 0 );

 start = clock();

 for( i=0; i<d; i++ )
 {
  for( j=0; j<d; j++ )
  {
   for( k=0; k<d; k++ )
   {
    matC[i][j] += matA[i][k]*matB[k][j];
   }
  }
 }

// viewMatrix( matA );
// viewMatrix( matB );
// viewMatrix( matC );

 end = clock();

 cout<<d<<": "<<((double)(end-start)/(double)CLOCKS_PER_SEC)<<endl;

 return 0;
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
