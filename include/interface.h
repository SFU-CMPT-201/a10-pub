#pragma once

#include <stddef.h>

#define MAX_KEY_SIZE 16
#define MAX_VALUE_SIZE 16

// Used for input
struct mr_input {
  struct mr_in_kv *kv_lst; // input key-value pairs (array)
  size_t count;            // number of input key-value pairs
};

// Used for input key-value pairs
struct mr_in_kv {
  char key[MAX_KEY_SIZE];     // input key string
  char value[MAX_VALUE_SIZE]; // input value string
};

// Used for intermediate and final key-value pairs
struct mr_out_kv {
  char key[MAX_KEY_SIZE];        // output key string
  char (*value)[MAX_VALUE_SIZE]; // output value strings (array)
  size_t count;                  // number of output value strings
};

// Used for final output
struct mr_output {
  struct mr_out_kv *kv_lst; // final output (array)
  size_t count;             // number of final key-value pairs
};

// Executes the map-reduce framework
// Blocks until the map-reduce framework is done
// Returns 0 on success, -1 on failure
int mr_exec(const struct mr_input *input,         // input key-value pairs
            void (*map)(const struct mr_in_kv *), // map function
            size_t mapper_count,                  // number of mappers (threads)
            void (*reduce)(const struct mr_out_kv *), // reduce function
            size_t reducer_count,    // number of reducers (threads)
            struct mr_output *output // pointer to a final output buffer
);

// Called from the map function for the intermediate output
// To emit one intermediate key-value pair
// Can be called multiple times within the same map function
// Can be called multiple times for the same key
// Returns 0 on success, -1 on failure
int mr_emit_i(const char *key, const char *value);

// Called from the reduce function for the final output
// To emit one final key-value pair
// Can be called multiple times within the same reduce function
// Can be called multiple times for the same key
// The final output is the union of all the emitted key-value pairs
// Returns 0 on success, -1 on failure
int mr_emit_f(const char *key, const char *value);
