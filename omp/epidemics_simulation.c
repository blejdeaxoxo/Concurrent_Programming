#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<omp.h>
#include"epidemics_utils.h"
#define SAVE_RESULTS

void Usage(char *prog_name){
   fprintf(stderr, "Usage: %s <TOTAL_SIMULATION_TIME> <InputFileName> <THREAD_COUNT>\n", prog_name);
   exit(-4);
}

int main(int argc, char **argv){
    if(argc != 4) Usage(argv[0]);
    int TOTAL_SIMULATION_TIME = atoi(argv[1]);
    input_file = argv[2];
    int NTHREADS = atoi(argv[3]);

    person_t *p1 = read_input();
    person_t *p2 = copy_array(p1);
    person_t *p3 = copy_array(p1);

    double serial, omp1, omp2;
    serial = serial_epidemics(p1,TOTAL_SIMULATION_TIME);
    omp1 = parallel_epidemics_for(p2,TOTAL_SIMULATION_TIME,NTHREADS,1,15);
    omp2 = parallel_epidemics_partition(p3,TOTAL_SIMULATION_TIME,NTHREADS);

    compare_results(p1, p2, "omp1");
    compare_results(p1, p3, "omp2");

    printf("Total simulation time= %d   Number of threads= %d   Number of people= %d\n", TOTAL_SIMULATION_TIME, NTHREADS, N_ELEM);
    printf("Serial time= %g\n",serial);
    printf("Parallel time with for= %g\n",omp1);
    printf("Speedup= %g\n", serial/omp1);
    printf("Parallel time with partition= %g\n",omp2);
    printf("Speedup= %g\n", serial/omp2);
    printf("\nRelative speedup= %g\n", omp1/omp2);

    free(p1);
    free(p2);
    free(p3);

    return 0;
}