#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <omp.h>
#include <mpi.h>
#include"mpi_epidemics.h"

int MAX_X, MAX_Y, N_ELEM, NTHREADS;
int **matrix;
int *array;
char *input_file;

person_t *copy_array(person_t *source_array){
    person_t* new_array = malloc(N_ELEM * sizeof(person_t));
    if (!new_array){
        perror("Insufficient memory!");
        MPI_Abort(MPI_COMM_WORLD, -3);
    }

    memcpy(new_array, source_array, N_ELEM * sizeof(person_t));

    return new_array;
}

person_t *read_input(){
    FILE *in = fopen(input_file, "r");
    if(in == NULL){
        perror("Failed opening input file.");
        MPI_Abort(MPI_COMM_WORLD, -1);
    }

    fscanf(in, "%d %d\n%d", &MAX_X, &MAX_Y, &N_ELEM);

    person_t *people = (person_t*)malloc(sizeof(person_t) * N_ELEM);
    if(people == NULL){
        perror("Insufficient memory!");
        MPI_Abort(MPI_COMM_WORLD, -3);
    }
    int dummy = 0;
    for(int i = 0; i < N_ELEM; i++){
        fscanf(in, "%d %d %d %d %d %d", &dummy, &people[i].X, &people[i].Y, (int*)&people[i].current_status, (int*)&people[i].direction, &people[i].amplitude);
        people[i].future_status = people[i].current_status;
        people[i].status_duration = 0;
        people[i].infection_counter = 0;
    }

    if(fclose(in) != 0){
        perror("Failed closing input file.");
        MPI_Abort(MPI_COMM_WORLD, -2);
    }
    return people;
}

char* generate_new_filename(const char* original_filename, const char* addition){
    const char* dot_position = strrchr(original_filename, '.');
    if (!dot_position){
        dot_position = original_filename + strlen(original_filename);
    }

    size_t base_length = dot_position - original_filename;
    size_t addition_length = strlen(addition);
    size_t extension_length = strlen(dot_position);
    size_t out_suffix_length = strlen("_out");

    size_t new_length = base_length + 1 + addition_length + out_suffix_length + extension_length + 1;

    char* new_filename = (char*)malloc(new_length);
    if (!new_filename){
        perror("Insufficient memory!");
        MPI_Abort(MPI_COMM_WORLD, -3);
    }

    memset(new_filename, 0, new_length);

    strncpy(new_filename, original_filename, base_length);
    strncat(new_filename, "_", new_length - strlen(new_filename) - 1);
    strncat(new_filename, addition, new_length - strlen(new_filename) - 1);
    strncat(new_filename, "_out", new_length - strlen(new_filename) - 1);
    strncat(new_filename, dot_position, new_length - strlen(new_filename) - 1);

    return new_filename;
}



void display_people(person_t *people, char *version){
    char *output_file = generate_new_filename(input_file, version);
    FILE *out = fopen(output_file, "w");
    if(out == NULL){
        perror("Failed opening output file.");
        MPI_Abort(MPI_COMM_WORLD, -1);
    }

    for(int i = 0; i < N_ELEM; i++){
        char *status;
        switch(people[i].current_status){
            case SUSCEPTIBLE:
                status = "SUSCEPTIBLE";
                break;
            case INFECTED:
                status = "INFECTED";
                break;
            case IMMUNE: 
                status = "IMMUNE";
                break;
        }
        fprintf(out, "Person %d; X= %d  Y= %d; Status: %s; Infection counter= %d\n",i+1,people[i].X,people[i].Y,status,people[i].infection_counter);
    }

    if(fclose(out) != 0){
        perror("Failed closing output file.");
        MPI_Abort(MPI_COMM_WORLD, -2);
    }
    free(output_file);
}

void updatePerson(person_t *person){
    switch(person->direction){
        case N:
            person->Y -= person->amplitude;
            if(person->Y < 0){
                person->Y = -person->Y;
                person->direction = S;
            }
            break;
        case V:
            person->X -= person->amplitude;
            if(person->X < 0){
                person->X = -person->X;
                person->direction = E;
            }
            break;
        case S:
            person->Y += person->amplitude;
            if(person->Y >= MAX_Y){
                person->Y = 2*MAX_Y - person->Y - 2;
                person->direction = N;
            }
            break;
        case E:
            person->X += person->amplitude;
            if(person->X >= MAX_X){
                person->X = 2*MAX_X - person->X - 2;
                person->direction = V;
            }
            break;
    }

    switch(person->current_status){
        case INFECTED:
            person->status_duration++;
            if(person->status_duration >= INFECTED_DURATION){
                person->status_duration = 0;
                person->future_status = IMMUNE;
                person->current_status = IMMUNE;
            }
            break;
        case IMMUNE:
            person->status_duration++;
            if(person->status_duration >= IMMUNE_DURATION){
                person->status_duration = 0;
                person->future_status = SUSCEPTIBLE;
                person->current_status = SUSCEPTIBLE;
            }
            break;
        case SUSCEPTIBLE:
            if(person->future_status == INFECTED){
                person->infection_counter++;
                person->current_status = INFECTED;
            }
            break;
    }
    
}

int equals(person_t a, person_t b){
    return (a.X == b.X) && (a.Y == b.Y) && (a.current_status == b.current_status) && (a.infection_counter == b.infection_counter);
}

void compare_results(person_t *serial, person_t *parallel, char *version){
    if(memcmp(serial, parallel, N_ELEM * sizeof(person_t)) == 0){
        printf("Version %s produces the same result as the serial version\n", version);
    }
    else{
        printf("Result for version %s differs from the serial version result at lines:", version);
        for(int i = 0; i < N_ELEM; i++){
            if(equals(serial[i], parallel[i]) == 0) printf(" %d", i+1);
        }
        printf("\n");
        perror("Serial and parallel version produce different results\n");
        MPI_Abort(MPI_COMM_WORLD, -5);
    }
}

void serial_iteration(person_t *people){
    for(int i = 0; i < MAX_X; i++){
        for(int j = 0; j < MAX_Y; j++) matrix[i][j] = 0;
    }

    for(int i =0; i < N_ELEM; i++){
        if(people[i].current_status == INFECTED){
            matrix[people[i].X][people[i].Y] = 1;
        }
    }

    for(int i = 0; i < N_ELEM; i++){
        if((people[i].current_status == SUSCEPTIBLE) && (matrix[people[i].X][people[i].Y] != 0)){
            people[i].future_status = INFECTED;
        }
        updatePerson(&people[i]);
    }
}

double serial_epidemics(person_t *people, int simulation_time){
    double start, elapsed;
    matrix = malloc(sizeof(int*)*MAX_X);
    if(matrix == NULL){
        perror("Insufficient memory!");
        exit(-3);
    }
    for(int i = 0; i < MAX_X; i++){
        matrix[i] = malloc(sizeof(int)*MAX_Y);
        if(matrix[i] == NULL){
            perror("Insufficient memory!");
            MPI_Abort(MPI_COMM_WORLD, -3);
        }
    }

    start = MPI_Wtime();
    for(int i = 0; i < simulation_time; i++){
        #ifdef DEBUG
        print_people(people);
        #endif

        serial_iteration(people);
    }
    elapsed = MPI_Wtime() - start;

#ifdef SAVE_RESULTS
    display_people(people, "serial");
#endif

    for(int i = 0; i < MAX_X; i++) free(matrix[i]);
    free(matrix);

    return elapsed;
}

MPI_Datatype create_MPI_PERSON(){
    const int nitems = 8;
    int block_lengths[] = {1, 1, 1, 1, 1, 1, 1, 1};

    MPI_Datatype types[] = {MPI_INT, MPI_INT, MPI_INT, MPI_INT, MPI_INT, MPI_INT, MPI_INT, MPI_INT};

    MPI_Aint offsets[nitems];
    offsets[0] = offsetof(person_t, X);
    offsets[1] = offsetof(person_t, Y);
    offsets[2] = offsetof(person_t, amplitude);
    offsets[3] = offsetof(person_t, infection_counter);
    offsets[4] = offsetof(person_t, status_duration);
    offsets[5] = offsetof(person_t, current_status);
    offsets[6] = offsetof(person_t, future_status);
    offsets[7] = offsetof(person_t, direction);

    MPI_Datatype MPI_PERSON;
    MPI_Type_create_struct(nitems, block_lengths, offsets, types, &MPI_PERSON);

    MPI_Type_commit(&MPI_PERSON);

    return MPI_PERSON;
}

void parallel_iteration(person_t *people, int LOCAL_N){
    memset(array, 0, MAX_X * MAX_Y * sizeof(int));

    for(int i =0; i < LOCAL_N; i++){
        if(people[i].current_status == INFECTED){
            array[people[i].X * MAX_Y + people[i].Y] = 1;
        }
    }

    MPI_Allreduce(MPI_IN_PLACE, array, MAX_X * MAX_Y, MPI_INT, MPI_LOR, MPI_COMM_WORLD);

    for(int i = 0; i < LOCAL_N; i++){
        if((people[i].current_status == SUSCEPTIBLE) && (array[people[i].X * MAX_Y + people[i].Y] != 0)){
            people[i].future_status = INFECTED;
        }
        updatePerson(&people[i]);
    }
}

void parallel_iteration_threads(person_t *people, int LOCAL_N){
    memset(array, 0, MAX_X * MAX_Y * sizeof(int));
    #pragma omp parallel for num_threads(NTHREADS) schedule(dynamic, 15)
        for(int i =0; i < LOCAL_N; i++){
            if(people[i].current_status == INFECTED){
                array[people[i].X * MAX_Y + people[i].Y] = 1;
            }
        }
    

    MPI_Allreduce(MPI_IN_PLACE, array, MAX_X * MAX_Y, MPI_INT, MPI_LOR, MPI_COMM_WORLD);

    #pragma omp parallel for num_threads(NTHREADS) schedule(dynamic, 15)
        for(int i = 0; i < LOCAL_N; i++){
            if((people[i].current_status == SUSCEPTIBLE) && (array[people[i].X * MAX_Y + people[i].Y] != 0)){
                people[i].future_status = INFECTED;
            }
            updatePerson(&people[i]);
        }
}

double parallel_epidemics(person_t *people, int simulation_time, int rank, void (*iteration)(person_t *, int), char *version){
    double start, elapsed = 0;
    int size;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    int LOCAL_N[size], displ[size];
    if((N_ELEM % size) == 0){
        for(int i=0; i<size; i++){
            displ[i] = (N_ELEM/size)*i;
            LOCAL_N[i] = N_ELEM/size;
        }
    }
    else{
        for(int i=0; i<size; i++){
            displ[i] = (N_ELEM/size +1)*i;
            if(i == (size -1)) LOCAL_N[i] = N_ELEM % (N_ELEM/size +1);
            else LOCAL_N[i] = N_ELEM/size +1;
        }
    }

    array = malloc(sizeof(int) * MAX_X * MAX_Y);
    person_t *pop_chunk = (person_t*)malloc(sizeof(person_t) * LOCAL_N[rank]);
    if(array == NULL || pop_chunk == NULL){
        perror("Insufficient memory!");
        MPI_Abort(MPI_COMM_WORLD, -3);
    }

    MPI_Datatype MPI_PERSON = create_MPI_PERSON();
    MPI_Barrier(MPI_COMM_WORLD);

    start = MPI_Wtime();
    MPI_Scatterv(people, LOCAL_N, displ, MPI_PERSON, pop_chunk, LOCAL_N[rank], MPI_PERSON, 0, MPI_COMM_WORLD);
    for(int i = 0; i < simulation_time; i++){
        iteration(pop_chunk, LOCAL_N[rank]);
    }
    MPI_Gatherv(pop_chunk, LOCAL_N[rank], MPI_PERSON, people, LOCAL_N, displ, MPI_PERSON, 0, MPI_COMM_WORLD);
    elapsed = MPI_Wtime() - start;

    #ifdef SAVE_RESULTS
    if(rank == 0){  
        display_people(people, version);
    }
    #endif

    free(array);
    free(pop_chunk);
    MPI_Type_free(&MPI_PERSON);

    return elapsed;
}
