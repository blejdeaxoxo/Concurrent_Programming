#include<stdio.h>
#include<stdlib.h>
#define NO 4
#define NI 4
#define NT 2
int main(int argc, char **argv){
    double speedups1[5][5][3];
    double speedups2[5][5][3];

    FILE *in1 = fopen(argv[1],"r");
    FILE *in2 = fopen(argv[2],"r");
    FILE *out = fopen("plot_data2.txt","w");
    if(in1 == NULL || in2 == NULL || out == NULL){
        perror("Failed opening file.");
        exit(-1);
    }
    for(int i = 0; i < 5; i++){
        for(int j = 0; j < 5; j++){
            for(int k = 0; k < 3; k++){
                fscanf(in1,"%lf ", &speedups1[i][j][k]);
                fscanf(in2,"%lf ", &speedups2[i][j][k]);
            }
        }
    }

    fprintf(out,"%g, %g, %g, %g, %g\n",speedups1[0][NI][NT],speedups1[1][NI][NT],speedups1[2][NI][NT],speedups1[3][NI][NT],speedups1[4][NI][NT]);
    fprintf(out,"%g, %g, %g, %g, %g\n",speedups2[0][NI][NT],speedups2[1][NI][NT],speedups2[2][NI][NT],speedups2[3][NI][NT],speedups2[4][NI][NT]);
    fprintf(out,"%g, %g, %g, %g, %g\n",speedups1[NO][0][NT],speedups1[NO][1][NT],speedups1[NO][2][NT],speedups1[NO][3][NT],speedups1[NO][4][NT]);
    fprintf(out,"%g, %g, %g, %g, %g\n",speedups2[NO][0][NT],speedups2[NO][1][NT],speedups2[NO][2][NT],speedups2[NO][3][NT],speedups2[NO][4][NT]);
    fprintf(out,"%g, %g, %g\n",speedups1[NO][NI][0],speedups1[NO][NI][1],speedups1[NO][NI][2]);
    fprintf(out,"%g, %g, %g\n",speedups2[NO][NI][0],speedups2[NO][NI][1],speedups2[NO][NI][2]);
    if(fclose(in1) != 0 || fclose(in2) != 0 || fclose(out) != 0){
        perror("Failed closing input.");
        exit(-2);
    }
    return 0;
}