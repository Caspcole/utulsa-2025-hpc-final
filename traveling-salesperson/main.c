// Async Traveling Salesperson with Pth

// gcc -g -Wall -o main main.c -lm -lpthread
// ./main

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
long thread_count;
unsigned int seed = 255;

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

  Get_args(argc, argv);

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

  // Read in matrix
  FILE *matrix_file = fopen("./DistanceMatrix1000_v2.csv", "r");
  if (!matrix_file)
  {
    printf("Failed to load matrix file.\n");
    exit(1);
  }

  // TODO zero out the matrix before inserting data
  // TODO add read-write mutex and lock before inserting data?

  printf("Entering matrix creation\n");
  char temp_line[MAX_LINE_LEN];
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
        printf("Row %d Col %d: Error, entry = %d.\n", row, col, temp_entry);

      token = strtok(NULL, ",");
    }
  }
  printf("First 5 of last line: %d %d %d %d %d\n",
         travel_matrix[ROWS - 1][0],
         travel_matrix[ROWS - 1][1],
         travel_matrix[ROWS - 1][2],
         travel_matrix[ROWS - 1][3],
         travel_matrix[ROWS - 1][4]);

  GET_TIME(start);

  printf("Entering greedy tests\n");

  for (thread = 0; thread < thread_count; thread++)
  {
    arguments[thread].rank = thread;
    arguments[thread].start_time = start;
    pthread_create(&thread_handles[thread], NULL, Find_best_tour, (void *)&arguments[thread]);
  }

  for (thread = 0; thread < thread_count; thread++)
    pthread_join(thread_handles[thread], NULL);

  // pi_estimate = 4 * number_in_circle / ((double)number_of_tosses);
  GET_TIME(finish);
  elapsed = finish - start;

  // elapsed = finish - start;
  printf("Number of threads: %ld\n", thread_count);
  // printf("Multi-threaded estimate of pi = %.15f\n", pi_estimate);
  printf("Elapsed time = %e seconds\n", elapsed);

  free(thread_handles);
  free(arguments);
  return 0;
}

void *Find_best_tour(void *arguments)
{
  pth_arg *args = (pth_arg *)arguments;
  long my_rank = (long)args->rank;
  unsigned int my_seed = seed + my_rank;
  double my_working_time;

  int *my_best_tour = malloc(COLS * sizeof(int));
  int my_best_tour_value = INT_MAX;

  do
  {
    GET_TIME(my_working_time); // use a thread-safe timer

    // convert the tour finding to a function that returns a tour and value

    int city_start = rand_r(&my_seed) % COLS;
    printf("City start: %d\n", city_start);
    int *test_tour = malloc(COLS * sizeof(int));
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

    printf("Test value: %d\n", tour_value);
    printf("Current best value %d: \n", my_best_tour_value);
    for (int i = 0; i < 5; i++)
      printf(" %d", my_best_tour[i]);
    printf("\n");

    printf("Time diff %f\n", my_working_time - args->start_time);
  } while (my_working_time - args->start_time < 0.02);

  // grab write lock
  // compare against global value
  // write to global tour if better value

  free(my_best_tour);
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

    for (int col = 0; col < COLS; col++)
    {
      // grab read lock
      if (!visited[col] && travel_matrix[previous][col] < min_dist)
      {
        min_dist = travel_matrix[previous][col];
        best = col;
      }
      // release read lock
    }
    test_tour[row] = best;
    visited[best] = 1;
    *tour_value += min_dist;
  }

} /* Find_tour */

/*------------------------------------------------------------------
 * Function:    Get_args
 * Purpose:     Get the command line args
 * In args:     argc, argv
 * Globals out: thread_count, number_of_tosses
 */
void Get_args(int argc, char *argv[])
{
  if (argc != 2)
    Usage(argv[0]);
  thread_count = strtol(argv[1], NULL, 10);
  if (thread_count <= 0 || thread_count > MAX_THREADS)
    Usage(argv[0]);

} /* Get_args */

/*------------------------------------------------------------------
 * Function:  Usage
 * Purpose:   Print a message explaining how to run the program
 * In arg:    prog_name
 */
void Usage(char *prog_name)
{
  fprintf(stderr, "usage: %s <number of threads> <n>\n", prog_name);
  fprintf(stderr, "   n is the number of tosses and should be >= 1\n");
  exit(0);
} /* Usage */