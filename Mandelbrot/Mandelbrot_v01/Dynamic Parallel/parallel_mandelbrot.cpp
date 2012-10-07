/*
* Parallel implementation of the Maldelbrot set
* Author: Omar U. Florez, Utah State University
*/

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "mpi.h"


#define X_RES  512
#define Y_RES  512

#define DATA_TAG  1
#define TERMINATE_TAG  2
#define RESULT_TAG  3



struct complex
{
	float real;
	float imag;
};

int cal_pixel(complex c)
{
	
	int count, max_iter;
	complex z;

    	float temp, length;
	max_iter = 256;
	z.real = 0;
	z.imag = 0;
	count = 0;

	do
	{
		temp = z.real * z.real - z.imag * z.imag + c.real;
		z.imag = 2*z.real*z.imag + c.imag;
		z.real = temp;
		length = z.real * z.real + z.imag * z.imag;
		count++;

		//printf("count: %d, length: %f\n", count, length);

	}while((length<4.0) && (count<max_iter));

	return count;

}

void print_canvas(int canvas[][Y_RES])
{
	printf("P3\n");
	printf("%d %d\n", X_RES, Y_RES); 	//width and height in pixels
	printf("255\n");		//maximum color value
	
	/*x-axis: real  y-axis: imaginary*/
        for(int i=0; i<X_RES; i++)
        {
                for(int j=0; j<Y_RES; j++)
                {
                	printf("%d ", canvas[j][i]); 	
                }	
		printf("\n"); 	       
	}
}

void master()
{
	int number_processors;
	int count = X_RES; //the number of rows in the image
	int row = 0;
	MPI_Status status;
	MPI_Comm_size(MPI_COMM_WORLD, &number_processors);
	
	
	int canvas[X_RES][Y_RES];
	int color[Y_RES]; 	
	
	//sending tasks to all the processors
	for(int i=1; i<number_processors; i++)
	{
		//send row and slave_id
		MPI_Send(&row, 1, MPI_INT, i, DATA_TAG, MPI_COMM_WORLD);			
		count--;
		row++;
		printf("%d\n", count);
	}
	
	
	printf("%d\n", count);
	//when slaves are idle, then keep sending		
	do
	{
		//color contains the values for the row colors
		MPI_Recv(color, Y_RES, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		//row is row id in the canvas
		MPI_Recv(&row, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);			
		//slave_id is the proccesor that is idle now			
		//count++;
		
		if(row<X_RES)
		{
			for(int i=0; i<Y_RES; i++)
			{
				canvas[row][i] = color[i];
			}

			MPI_Send(&row, 1, MPI_INT, status.MPI_SOURCE, DATA_TAG, MPI_COMM_WORLD);
			count--; //minus one row processed			
			row++;
			printf("%d\n", count);
		}
		else
		{
			for(int i=0; i<number_processors; i++)
			{
				MPI_Send(0, 0, MPI_INT, i, TERMINATE_TAG, MPI_COMM_WORLD);
				
			}
		}
	}while(count>0);
	
	print_canvas(canvas);
}

void slave()
{
	float real_min=-2, imag_min = -2;
	float real_max=2, imag_max = 2;
	float scale_real = (real_max - real_min)/X_RES;
	float scale_imag = (imag_max - imag_min)/Y_RES;		  
	
	int canvas [X_RES][Y_RES];
	int color[Y_RES]; 	//it will be iteratively updated by a slave processor
				//Y_RES: number of columns in tha array
	int row;
	MPI_Status status;

	//color contains the values for the row colors
	MPI_Recv(&row, X_RES, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
			
	while(status.MPI_TAG==DATA_TAG)
	{
		//let's compute a row of the canvas
		int i = row;
		//c.real = j;
	        //c.imag = i;	         
	        //Y_RES lies on the x-axis!!       
		for(int j=0; j<Y_RES; i++)
	        {	                
			complex c;	                	
			c.real = real_min + ((float) j*scale_real);	
			c.imag = imag_min + ((float) i*scale_imag);
	            	color[j] = cal_pixel(c);    
		}	
		MPI_Send(color, Y_RES, MPI_INT, 0, RESULT_TAG, MPI_COMM_WORLD);
		MPI_Send(&row, 1, MPI_INT, 0, RESULT_TAG, MPI_COMM_WORLD);
		MPI_Recv(&row, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
	}	
	
	if(status.MPI_TAG==TERMINATE_TAG)		
	{
		return;
	}

}

int main( int argc, char *argv[] )
{
	int rank;	
	//////////////////////////////////////////////////////
	
	MPI_Init(&argc, &argv );
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	
	if(rank==0) //MASTER
	{	
		master();
	}
	else //SLAVE
	{
		slave();
	}

	MPI_Finalize();
	return 0;
}


