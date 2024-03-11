#pragma once

#include "interface.h"
#include <stdbool.h>

#define MAX_DATA_SIZE 1024
#define MAX_THREADS 32

#define TEST(cond, pts)                                                        \
  do {                                                                         \
    test(__FILE__, __LINE__, cond, pts);                                       \
  } while (0)

void test(char *, size_t, bool, size_t);
bool single_map(void);
bool single_reduce(void);
bool single_map_reduce(void);
bool number_of_mappers(void);
bool number_of_reducers(void);
bool partition_input(void);
bool partition_intermediate(void);
bool full_map_reduce(void);
bool multiple_calls(void);
void free_output(struct mr_output *);
