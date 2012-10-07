#include <iostream>
using namespace std;

int** matA;
int** matB;
int** matC;

void loadMatrix( int**, int, int, int );
void viewMatrix( int**, int, int );

int main()
{
 int m=200, n=200, p=200;
 int i, j, k;
 clock_t start, end;

 matA = new int*[n];
 matB = new int*[m];
 matC = new int*[n];

 loadMatrix( matA, n, m, 2 );
 loadMatrix( matB, m, p, 1 );
 loadMatrix( matC, n, p, 0 );

 start = clock();

 for( i=0; i<n; i++ )
 {
  for( j=0; j<p; j++ )
  {
   for( k=0; k<m; k++ )
   {
    matC[i][j] += matA[i][k]*matB[k][j];
   }
  }
 }

// viewMatrix( matA, n, m );
// viewMatrix( matB, m, p );
// viewMatrix( matC, n, p );

 end = clock();

 cout<<"Sequential: "<<((double)(end-start)/(double)CLOCKS_PER_SEC)<<endl;

 system("pause");

 return 0;
}

void loadMatrix( int** matrix, int rows, int columns, int value )
{
 int i, j;

 for ( i=0; i<rows; i++ )
 {
  matrix[i] = new int[columns];
  for( j=0; j<columns; j++ )
  {
   matrix[i][j] = value;
  }
 }
}

void viewMatrix( int** matrix, int rows, int columns )
{
 int i, j;

 for ( i=0; i<rows; i++ )
 {
  for( j=0; j<columns; j++ )
  {
   cout<<matrix[i][j]<<" ";
  }
  cout<<endl;
 }
}
