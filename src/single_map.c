#include "interface.h"
#include "tests.h"
#include <stdio.h>
#include <string.h>

struct mr_in_kv sm_in_kv_lst[MAX_DATA_SIZE];

struct mr_in_kv sm_kv_store[MAX_DATA_SIZE];
size_t sm_call_count = 0;

void sm_map(const struct mr_in_kv *in_kv) {
  size_t index = sm_call_count++;

  if (index >= MAX_DATA_SIZE) {
    return;
  }

  strncpy(sm_kv_store[index].key, in_kv->key, MAX_KEY_SIZE);
  strncpy(sm_kv_store[index].value, in_kv->value, MAX_VALUE_SIZE);
}

void sm_reduce(const struct mr_out_kv *inter_kv) {}

int sm_cmp() {
  for (size_t i = 0; i < MAX_DATA_SIZE; i++) {
    if (strncmp(sm_kv_store[i].key, sm_in_kv_lst[i].key, MAX_KEY_SIZE) != 0 ||
        strncmp(sm_kv_store[i].value, sm_in_kv_lst[i].value, MAX_VALUE_SIZE) !=
            0) {
      return -1;
    }
  }
  return 0;
}

bool single_map() {
  sm_call_count = 0;
  for (size_t i = 0; i < MAX_DATA_SIZE; i++) {
    snprintf(sm_in_kv_lst[i].key, MAX_KEY_SIZE, "%zu", i);
    snprintf(sm_in_kv_lst[i].value, MAX_VALUE_SIZE, "%zu", i);
  }

  struct mr_input sm_input = {sm_in_kv_lst, MAX_DATA_SIZE};
  struct mr_output sm_output;

  bool res = mr_exec(&sm_input, sm_map, 1, sm_reduce, 1, &sm_output) == 0 &&
             sm_call_count == MAX_DATA_SIZE && sm_cmp() == 0;
  free_output(&sm_output);
  TEST(res, 10);
  return res;
}
