// Async Traveling Salesperson with Pth

// Compile: gcc -g -Wall -o atsp_pth atsp_pth.c -lm -lpthread
// Execute: ./atsp_pth <number of threads> <seed>

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "timer.h"
#include <limits.h>
#include <string.h>

#define MAX_THREADS 1024
#define COLS 1000
#define ROWS 1000
#define MAX_LINE_LEN 12000

int travel_matrix[COLS][ROWS];
int *global_best_tour;
int global_best_tour_value;
long thread_count;
unsigned int seed = 256;
pthread_rwlock_t rwlock_travel_matrix = PTHREAD_RWLOCK_INITIALIZER;
pthread_rwlock_t rwlock_best_tour = PTHREAD_RWLOCK_INITIALIZER;

double *non_comm_end_time;

typedef struct
{
  long rank;
  double start_time;
} pth_arg;

void Get_args(int argc, char *argv[]);
void *Estimate_pi(void *rank);
void Usage(char *prog_name);
void *Find_best_tour(void *arguments);
void Find_tour(int *test_tour, int *tour_value, int city_start);

int main(int argc, char *argv[])
{
  long thread;
  double start, finish, elapsed;
  pthread_t *thread_handles;

  GET_TIME(start);

  Get_args(argc, argv);

  non_comm_end_time = malloc(thread_count * sizeof(double));
  if (non_comm_end_time == NULL)
  {
    printf("non_comm_end_time failed to allocate\n");
    exit(1); // Handle memory allocation failure
  }

  thread_handles = (pthread_t *)malloc(thread_count * sizeof(pthread_t));
  if (thread_handles == NULL)
  {
    printf("thread_handles failed to allocate\n");
    exit(1); // Handle memory allocation failure
  }

  pth_arg *arguments = malloc(thread_count * sizeof(pth_arg));

  if (arguments == NULL)
  {
    printf("Pth argument failed to allocate\n");
    exit(1); // Handle memory allocation failure
  }

  global_best_tour_value = INT_MAX;
  global_best_tour = malloc(ROWS * sizeof(int));
  if (global_best_tour == NULL)
  {
    printf("global_best_tour failed to allocate\n");
    exit(1); // Handle memory allocation failure
  }

  // Read in matrix
  FILE *matrix_file = fopen("./DistanceMatrix1000_v2.csv", "r");
  if (!matrix_file)
  {
    printf("Failed to load matrix file.\n");
    exit(1);
  }

  char temp_line[MAX_LINE_LEN];
  pthread_rwlock_wrlock(&rwlock_travel_matrix);
  for (int row = 0; row < ROWS; row++)
  {
    fgets(temp_line, sizeof(temp_line), matrix_file);

    char *token;
    int temp_entry;
    token = strtok(temp_line, ",");

    for (int col = 0; col < COLS; col++)
    {
      temp_entry = strtol(token, NULL, 10);
      if (temp_entry > 0 && temp_entry < 1000)
      {
        travel_matrix[row][col] = temp_entry;
      }
      else
        printf("Row %d Col %d: Error, entry = %d.\n",
               row, col, temp_entry);

      token = strtok(NULL, ",");
    }
  }
  pthread_rwlock_unlock(&rwlock_travel_matrix);

  for (thread = 0; thread < thread_count; thread++)
  {
    arguments[thread].rank = thread;
    arguments[thread].start_time = start;
    pthread_create(&thread_handles[thread], NULL,
                   Find_best_tour, (void *)&arguments[thread]);
  }

  for (thread = 0; thread < thread_count; thread++)
    pthread_join(thread_handles[thread], NULL);

  GET_TIME(finish);
  elapsed = finish - start;

  printf("Cities of best tour found:");
  for (int i = 0; i < ROWS + 1; i++)
  {
    printf(" %d", global_best_tour[i]);
  }
  printf("\n");
  printf("Number of cities traversed: %d\n", ROWS + 1);
  printf("Best tour value: %d\n", global_best_tour_value);
  printf("Number of threads: %ld\n", thread_count);
  printf("Elapsed time = %e seconds\n", elapsed);
  for (int i = 0; i < thread_count; i++)
  {
    printf("Thread %d post-loop time %e\n",
           i, non_comm_end_time[i] - start);
  }

  free(thread_handles);
  free(arguments);
  free(non_comm_end_time);
  return 0;
}

void *Find_best_tour(void *arguments)
{
  pth_arg *args = (pth_arg *)arguments;
  long my_rank = (long)args->rank;
  unsigned int my_seed = seed + my_rank;
  double my_working_time;

  int *my_best_tour = malloc(ROWS * sizeof(int));
  int my_best_tour_value = INT_MAX;

  do
  {
    int city_start = rand_r(&my_seed) % ROWS;
    int *test_tour = malloc((ROWS + 1) * sizeof(int));
    int tour_value = 0;
    Find_tour(test_tour, &tour_value, city_start);

    if (tour_value < my_best_tour_value)
    {
      free(my_best_tour);
      my_best_tour = test_tour;
      my_best_tour_value = tour_value;
    }
    else
      free(test_tour);

    GET_TIME(my_working_time);
  } while (my_working_time - args->start_time < 60.0);

  non_comm_end_time[my_rank] = my_working_time;

  // grab write lock
  pthread_rwlock_wrlock(&rwlock_best_tour);
  if (my_best_tour_value < global_best_tour_value)
  {
    free(global_best_tour);
    global_best_tour_value = my_best_tour_value;
    global_best_tour = my_best_tour;
  }
  else
  {
    free(my_best_tour);
  }
  pthread_rwlock_unlock(&rwlock_best_tour);

  return NULL;
} /* Find_best_tour */

void Find_tour(int *test_tour, int *tour_value, int city_start)
{
  test_tour[0] = city_start;

  int visited[COLS] = {0};
  visited[city_start] = 1;

  *tour_value = 0;

  for (int row = 1; row < ROWS; row++)
  {
    int previous = test_tour[row - 1];
    int best = -1;
    int min_dist = INT_MAX;

    pthread_rwlock_rdlock(&rwlock_travel_matrix);
    for (int col = 0; col < COLS; col++)
    {
      if (!visited[col] && travel_matrix[previous][col] < min_dist)
      {
        min_dist = travel_matrix[previous][col];
        best = col;
      }
    }
    pthread_rwlock_unlock(&rwlock_travel_matrix);

    test_tour[row] = best;
    visited[best] = 1;
    *tour_value += min_dist;
  }
  pthread_rwlock_rdlock(&rwlock_travel_matrix);
  *tour_value += travel_matrix[test_tour[ROWS - 1]][city_start];
  pthread_rwlock_unlock(&rwlock_travel_matrix);
  test_tour[ROWS] = city_start;

} /* Find_tour */

/*------------------------------------------------------------------
 * Function:    Get_args
 * Purpose:     Get the command line args
 * In args:     argc, argv
 * Globals out: thread_count, seed
 */
void Get_args(int argc, char *argv[])
{
  if (argc != 3)
    Usage(argv[0]);
  thread_count = strtol(argv[1], NULL, 10);
  if (thread_count <= 0 || thread_count > MAX_THREADS)
    Usage(argv[0]);
  seed = strtol(argv[2], NULL, 10);
  if (seed < 0)
    Usage(argv[0]);

} /* Get_args */

/*------------------------------------------------------------------
 * Function:  Usage
 * Purpose:   Print a message explaining how to run the program
 * In arg:    prog_name
 */
void Usage(char *prog_name)
{
  fprintf(stderr, "usage: %s <number of threads> <seed>\n", prog_name);
  fprintf(stderr, "   seed is the starting seed for randomness and should be >= 0\n");
  exit(0);
} /* Usage */