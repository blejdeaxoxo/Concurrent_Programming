#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<omp.h>
#include"epidemics_utils.h"

int main(){
    double serial, omp1, omp2;
    person_t *p1, *p2, *p3, *dummy;
    FILE *out1 = fopen("speedups_omp1.txt","w");
    FILE *out2 = fopen("speedups_omp2.txt","w");
    FILE *out3 = fopen("relative_speedups.txt","w");
    if(out1==NULL || out2==NULL || out3==NULL){
        perror("Failed opening output file.");
        exit(-1);
    }

    int time[5] = {50, 100, 150, 200, 500};
    int thread_count[3] = {2, 4, 8};
    char inputs[5][25] = {
        "epidemics10K.txt",
        "epidemics20K.txt",
        "epidemics50K.txt",
        "epidemics100K.txt",
        "epidemics500K.txt"
    };

    for(int i = 0; i < 5; i++){
        input_file = inputs[i];
        dummy = read_input();
        for(int j = 0; j < 5; j++){
            p1 = copy_array(dummy);
            serial = serial_epidemics(p1, time[j]);
            for(int k = 0; k < 3; k++){
                p2 = copy_array(dummy);
                p3 = copy_array(dummy);

                omp1 = parallel_epidemics_for(p2, time[j], thread_count[k], 1, 15);
                omp2 = parallel_epidemics_partition(p3, time[j], thread_count[k]);

                fprintf(out1,"%g ",serial/omp1);
                fprintf(out2,"%g ",serial/omp2);
                fprintf(out3,"%g ",omp1/omp2);

                free(p2);
                free(p3);
            }
            free(p1);
            fflush(out1);
            fflush(out2);
            fflush(out3);
        }
        free(dummy);
    }

    if((fclose(out1)+fclose(out2)+fclose(out3)) != 0){
        perror("Failed closing output file.");
        exit(-2);
    }

    return 0;
}