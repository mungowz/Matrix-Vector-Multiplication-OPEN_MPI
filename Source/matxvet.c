/*
    MIMD-DM
 Leggiamo da un file le righe, le colonne, la matrice, il vettore b
 mod(N,q) != 0
 Calcolo di nloc
 Distribuzione delle righe a ciascun processore con MPI_Scatter
    1 fase:
 Calcolo in parallelo
    2 fase:
nessuna collezione dei risultati
Implementazione dell'algoritmo parallelo (np processori)
per il calcolo del prodotto tra una Matrice A di dimensione NxM e
un vettore b di dimensione M, adottando la I Strategia.
L'algoritmo è sviluppato in ambiente MPI_DOCKER.
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>
#include <limits.h>
#include "mpi.h"
#define MAXBUF 1024
#define MasterProc 0

// read rows
// read cols
// read elements from text file
void generateMatrix(int rows, int cols) {

    FILE *text = fopen("inputFile.txt", "w+");

    fseek(text, 0, SEEK_SET);
    fprintf(text, "%d\n", rows);
    fprintf(text, "%d\n", cols);

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            fprintf(text, "%d\n", (int) rand()%UCHAR_MAX);
        }
    }
    for (int i = 0; i < cols; i++) {
        fprintf(text, "%d\n", (int) rand()%UCHAR_MAX);
    }
    fclose(text);
}

int *readMatrix(FILE *text, int *rows, int *cols)
{
    fseek(text, 0, SEEK_SET);
    fscanf(text, "%d\n", rows);
    fscanf(text, "%d\n", cols);
    int *A = (int*)malloc((*rows) * (*cols) * sizeof(int));

    for (int i = 0; i < *rows; i++) {
        for (int j = 0; j < *cols; j++) {
                fscanf(text, "%d", &A[*rows * i + j]);
        }
    }
    return A;
}
int *readVector(FILE *text, int cols){
     // lettura del vettore b
    int *x = (int*) malloc(cols * sizeof(int));
    for (int i = 0; i < cols; i++) {
    fscanf(text, "%d", &x[i]);
    }
    return x;
}


void createGrid(MPI_Comm *grid, int processorID, int numberOfProcessor, int rows, int cols) {
    int reorder = 0, *period, dim;
    // column vector
    int dims[2] = {numberOfProcessor, 1};
    MPI_Dims_create(dim, 2, dims);
    period = (int*) calloc (dim, sizeof(int));
    period[0] = period [1] = 1;
    printf("before MPI_cart_create\n");
    MPI_Cart_create(MPI_COMM_WORLD,2,dims,period,reorder,grid);
    return;
}

void matXvet_local(MPI_Comm* grid, int processorID, int numberOfProcessor, int nloc, int cols, int* b) {


}
void printMat(int *A, int *x,int rows,int cols){
      printf("\nMatrice\n");
    for(int i = 0; i<rows; i++){
       for(int j = 0; j<cols; j++){
            printf("%d\t",*(A+rows*i+j));
        }
        printf("\n");
       }
    printf("\nVettore\n");
    for(int j = 0; j<cols; j++){
       printf("%d\n", *(x+j));
   }
}

// ./matxvet <textfilename>
int main(int argc, char *argv[])
{
    srand((unsigned int) 0);
    int rows, cols;
    int byteRead;
    int *x,*A;
    char buffer[MAXBUF];
    int processorID, numberOfProcessor;
    MPI_Status status;
    int nloc, q, p, mod;
    int *xloc;
    // Ax = y
    if(argc < 3){
        printf("Usage %s <rows> <cols>\n",argv[0]);
        exit(-1);
    } else {
	printf("**********MATXVET\n");
    }
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &processorID);
    MPI_Comm_size(MPI_COMM_WORLD, &numberOfProcessor);   
    
    if (processorID == MasterProc) {

        // read from textfile
        FILE *text = fopen("inputFile.txt", "r");

        generateMatrix(atoi(argv[1]), atoi(argv[2]));
	printf("Matrix generated\n");

        A =readMatrix(text,&rows,&cols);
	printf("Matrix read\n");

        x = readVector(text,cols);
	printf("Vector read\n");
        fclose(text);
        //printMat(A,x,rows,cols);
    }
    //createGrid(&grid, processorID, numberOfProcessor, rows, cols);
/*    int reorder = 1; 
    int dim;
    int dims[2] = {0,0};
    int period[2] = {1,1};
    MPI_Dims_create(dim, 2, dims);
    MPI_Comm grid; 
    MPI_Cart_create(MPI_COMM_WORLD, 2, dims, period, reorder, &grid);
    printf("Grid created!\n");
    MPI_Comm_rank(grid, &processorID);
    int my_coords[2];
    MPI_Cart_coords(grid, processorID, 2, my_coords);

    // Print my location in the 2D torus.
    printf("[MPI process %d] I am located at (%d, %d).\n", processorID, my_coords[0],my_coords[1]);
    exit(0);

*/
    // DISTRIBUZIONE DEI DATI
  //  sleep(2);
  //  MPI_Barrier(grid);	
    MPI_Barrier(MPI_COMM_WORLD);
    int rc;
    // invio dimensioni matrice
  
    //MPI_Bcast(&rows, 1, MPI_INT, 0, grid);
    MPI_Bcast(&rows, 1, MPI_INT, 0, MPI_COMM_WORLD);    
   // MPI_Bcast(&cols,1,MPI_INT,0,grid);
    MPI_Bcast(&cols, 1, MPI_INT, 0, MPI_COMM_WORLD);
    printf("Matrix dims broadcasted\n");
    // mod(M,p) != 0 -> OK
    // mod(N,q) != 0 -> ridistribuire il resto delle righe

    //Calcolo dimensioni locali
    if ((mod = rows%numberOfProcessor) == 0)
        nloc = rows/numberOfProcessor;
    else {
        for (int step = 0; step < mod; step++)
              nloc++;
        }
    xloc = (int*) malloc(nloc*sizeof(int));


   // MPI_Scatter(A, nloc, MPI_INT, xloc, nloc, MPI_INT, 0,grid);
    MPI_Scatter(A, nloc, MPI_INT, xloc, nloc, MPI_INT, 0,MPI_COMM_WORLD);
    printf("Rows sent from p0 to others\n");
    MPI_Barrier(MPI_COMM_WORLD);
   // MPI_Bcast(x,cols,MPI_INT,0,grid);
     MPI_Bcast(x,cols,MPI_INT,0,MPI_COMM_WORLD);
    printf("X vector sent from p0 to others\n");

     

    // CALCOLO LOCALE
   // matXvet_local(&grid, processorID, numberOfProcessor, nloc, cols/p, xloc, x);
    MPI_Finalize();
    return 0;
}
