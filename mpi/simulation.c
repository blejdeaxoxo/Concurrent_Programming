#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <omp.h>
#include <mpi.h>
#include"mpi_epidemics.h"
#define NLP 8
#define TESTS 20

void Usage(char *prog_name, int rank){
    if(rank == 0){
        fprintf(stderr, "Usage: %s <TOTAL_SIMULATION_TIME> <InputFileName>\n", prog_name);
        MPI_Abort(MPI_COMM_WORLD, -4);
    }
}

int main(int argc, char **argv){
    int rank, size;
    MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    if(argc != 3) Usage(argv[0], rank);
    int TOTAL_SIMULATION_TIME = atoi(argv[1]);
    input_file = argv[2];
    NTHREADS = NLP / size;
    person_t *dummy=NULL, *p1=NULL, *p2=NULL, *p3=NULL;
    double serial=0, mpi1=0, mpi2=0;
    if(rank == 0){
        dummy = read_input();
        for(int i=0; i<TESTS; i++){
            p1 = copy_array(dummy);
            serial += serial_epidemics(p1,TOTAL_SIMULATION_TIME);
        }
        serial = serial/TESTS;
    }
    MPI_Bcast(&MAX_X, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&MAX_Y, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&N_ELEM, 1, MPI_INT, 0, MPI_COMM_WORLD);
    
    for(int i=0; i<TESTS; i++){
        if(rank == 0){
            p2 = copy_array(dummy);
            p3 = copy_array(dummy);
        }

        mpi1 += parallel_epidemics(p2, TOTAL_SIMULATION_TIME, rank, parallel_iteration, "mpi1");
        mpi2 += parallel_epidemics(p3, TOTAL_SIMULATION_TIME, rank, parallel_iteration_threads, "mpi2");
    }
    mpi1 = mpi1/TESTS;
    mpi2 = mpi2/TESTS;

    if(rank == 0){
        compare_results(p1, p2, "mpi1");
        compare_results(p1, p3, "mpi2");
        printf("Total simulation time= %d   Number of people= %d\n", TOTAL_SIMULATION_TIME, N_ELEM);
        printf("Serial time= %g\n",serial);
        printf("Parallel time with for= %g\n",mpi1);
        printf("Speedup= %g\n", serial/mpi1);
        printf("Parallel time with partition= %g\n",mpi2);
        printf("Speedup= %g\n", serial/mpi2);

        free(dummy);
        free(p1);
        free(p2);
        free(p3);
    }
    MPI_Finalize();
    return 0;
}