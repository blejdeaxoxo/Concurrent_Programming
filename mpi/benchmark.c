#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <omp.h>
#include <mpi.h>
#include"mpi_epidemics.h"
#define NLP 8
#define NTESTS 80


int main(int argc, char **argv){
    int rank, size;
    MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    int time[5] = {50, 100, 150, 200, 500};
    char inputs[5][25] = {
        "epidemics10K.txt",
        "epidemics20K.txt",
        "epidemics50K.txt",
        "epidemics100K.txt",
        "epidemics500K.txt"
    };
    FILE *out1 =NULL;
    FILE *out2 =NULL;

    if(rank == 0){
        out1 = fopen("speedups_mpi1.txt","w");
        out2 = fopen("speedups_mpi2.txt","w");
        if(out1==NULL || out2==NULL){
            perror("Failed opening output file.");
            MPI_Abort(MPI_COMM_WORLD, -1);
        }
    }

    NTHREADS = NLP / size;
    person_t *dummy=NULL, *p1=NULL, *p2=NULL, *p3=NULL;
    double serial=0, mpi1=0, mpi2=0;
    int TESTS = NTESTS;
    int TOTAL_SIMULATION_TIME = 200;
    for(int i = 0; i<5; i++){
        serial=0, mpi1=0, mpi2=0;
        input_file = inputs[i];
        if(rank == 0){
            dummy = read_input();
            for(int i=0; i<TESTS; i++){
                p1 = copy_array(dummy);
                serial += serial_epidemics(p1,TOTAL_SIMULATION_TIME);
                free(p1);
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
            if(rank == 0){
                free(p2);
                free(p3);
            }
        }
        mpi1 = mpi1/TESTS;
        mpi2 = mpi2/TESTS;

        if(rank == 0){
            free(dummy);
            fprintf(out1,"%g ",serial/mpi1);
            fprintf(out2,"%g ",serial/mpi2);
            fflush(out1);
            fflush(out2);
        }
        TESTS = TESTS/2;
    }
    if(rank == 0){
        fprintf(out1,"\n");
        fprintf(out2,"\n");
    }
    input_file = "epidemics50K.txt";
    TESTS = NTESTS/4;
    if(rank == 0){
        dummy = read_input();
    }
    MPI_Bcast(&MAX_X, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&MAX_Y, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&N_ELEM, 1, MPI_INT, 0, MPI_COMM_WORLD);
        
    for(int i = 0; i<5; i++){
        TOTAL_SIMULATION_TIME = time[i];
        serial=0, mpi1=0, mpi2=0;
        if(rank == 0){
            for(int i=0; i<TESTS; i++){
                p1 = copy_array(dummy);
                serial += serial_epidemics(p1,TOTAL_SIMULATION_TIME);
                free(p1);
            }
            serial = serial/TESTS;
        }
        for(int i=0; i<TESTS; i++){
            if(rank == 0){
                p2 = copy_array(dummy);
                p3 = copy_array(dummy);
            }

            mpi1 += parallel_epidemics(p2, TOTAL_SIMULATION_TIME, rank, parallel_iteration, "mpi1");
            mpi2 += parallel_epidemics(p3, TOTAL_SIMULATION_TIME, rank, parallel_iteration_threads, "mpi2");
            if(rank == 0){
                free(p2);
                free(p3);
            }
        }
        mpi1 = mpi1/TESTS;
        mpi2 = mpi2/TESTS;

        if(rank == 0){
            fprintf(out1,"%g ",serial/mpi1);
            fprintf(out2,"%g ",serial/mpi2);
            fflush(out1);
            fflush(out2);
        }
        
    }

    if(rank == 0){
        free(dummy);
        if((fclose(out1)+fclose(out2)) != 0){
        perror("Failed closing output file.");
        exit(-2);
    }
    }

    MPI_Finalize();
    return 0;
}