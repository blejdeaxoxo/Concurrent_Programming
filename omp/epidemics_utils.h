#ifndef EPIDEMICS_UTILS_H
#define EPIDEMICS_UTILS_H

#define INFECTED_DURATION 4
#define IMMUNE_DURATION 7

extern int MAX_X, MAX_Y, N_ELEM;
extern int **matrix;
extern char *input_file;

typedef enum{
    INFECTED,
    SUSCEPTIBLE,
    IMMUNE
}status_t;

typedef enum{
    N,
    S,
    E,
    V
}direction_t;

typedef struct{
    int X, Y, amplitude, infection_counter, status_duration;
    status_t current_status, future_status;
    direction_t direction;
}person_t;

person_t *copy_array(person_t *source_array);

person_t *read_input();

char* generate_new_filename(const char* original_filename, const char* addition);

void display_people(person_t *people, char *version);

void updatePerson(person_t *person);

void print_people( person_t *people);

int equals(person_t a, person_t b);

void compare_results(person_t *serial, person_t *parallel, char *version);

void serial_iteration(person_t *people);

double serial_epidemics(person_t *people, int simulation_time);

void parallel_iteration_for(person_t *people);

double parallel_epidemics_for(person_t *people, int simulation_time, int NTHREADS, int policy, int chunk_size);

void parallel_iteration_partition(person_t *people, int matrix_chunk, int array_chunk);

double parallel_epidemics_partition(person_t *people, int simulation_time, int NTHREADS);

#endif