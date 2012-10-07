#include<iostream>
using namespace std;

int sum( int );

int main()
{
 int newSum = 0;

 for(int i=0; i<10; i++)
 {
 newSum = sum( newSum );

 cout<<newSum<<endl;
 }
 system( "pause" );
 return 0;
}

int sum( int addend )
{
 return addend+2;
}
