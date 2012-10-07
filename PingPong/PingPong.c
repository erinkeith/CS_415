 /*
  "Hello World" MPI Test Program
 */
 #include <mpi.h>
 #include <stdio.h>
 #include <string.h>
 
 #define BUFSIZE 128
 #define TAG 0
 
 int main(int argc, char *argv[])
 {
   int numprocs;
   int myid;
   int i;
   MPI_Status stat;
 
   MPI_Init(&argc,&argv); /* all MPI programs start with MPI_Init; all 'N' processes exist thereafter */
   MPI_Comm_size(MPI_COMM_WORLD,&numprocs); /* find out how big the SPMD world is */
   MPI_Comm_rank(MPI_COMM_WORLD,&myid); /* and this processes' rank is */
 
   /* At this point, all programs are running equivalently, the rank distinguishes
      the roles of the programs in the SPMD model, with rank 0 often used specially... */
   if(myid == 0)
   {
     double latetime, starttime, endtime;

     starttime = MPI_Wtime();

     MPI_Send(&i, 1, MPI_INT, 1, TAG, MPI_COMM_WORLD);

     MPI_Recv(&i, 1, MPI_INT, 1, TAG, MPI_COMM_WORLD, &stat);

     endtime = MPI_Wtime();
     latetime = endtime - starttime;      

     printf("Latency time in seconds: %f \n", latetime);

   }
   else
   {
     double starttime, endtime;
     /* receive from rank 0: */
     MPI_Recv(&i, 1, MPI_INT, 0, TAG, MPI_COMM_WORLD, &stat);
     
     
     /* send to rank 0: */
     MPI_Send(&i, 1, MPI_INT, 0, TAG, MPI_COMM_WORLD);

 }
 
   MPI_Finalize(); /* MPI Programs end with MPI Finalize; this is a weak synchronization point */
   return 0;
}
