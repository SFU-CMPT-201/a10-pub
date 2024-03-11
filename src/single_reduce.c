#include "interface.h"
#include "tests.h"
#include <stdio.h>
#include <string.h>

#define MOD 8

struct mr_in_kv sr_in_kv_lst[MAX_DATA_SIZE];

size_t sr_call_count = 0;
struct sr_inter_kv {
  char key[MAX_KEY_SIZE];
  char value[MAX_DATA_SIZE / MOD][MAX_VALUE_SIZE];
};
struct sr_inter_kv sr_kv_store[MOD];

void sr_map(const struct mr_in_kv *in_kv) {
  mr_emit_i(in_kv->key, in_kv->value);
}

void sr_reduce(const struct mr_out_kv *inter_kv) {
  size_t index = sr_call_count++;

  if (index >= MOD || inter_kv->count != MAX_DATA_SIZE / MOD) {
    return;
  }

  strncpy(sr_kv_store[index].key, inter_kv->key, MAX_KEY_SIZE);
  for (size_t i = 0; i < inter_kv->count; i++) {
    strncpy(sr_kv_store[index].value[i], inter_kv->value[i], MAX_VALUE_SIZE);
  }
}

int sr_cmp() {
  for (size_t i = 0; i < MAX_DATA_SIZE; i++) {
    if (strncmp(sr_kv_store[i % MOD].key, sr_in_kv_lst[i].key, MAX_KEY_SIZE) !=
        0) {
      return -1;
    }

    if (strncmp(sr_kv_store[i % MOD].value[i / MOD], sr_in_kv_lst[i].value,
                MAX_VALUE_SIZE) != 0) {
      return -1;
    }
  }
  return 0;
}

bool single_reduce() {
  sr_call_count = 0;

  for (size_t i = 0; i < MAX_DATA_SIZE; i++) {
    snprintf(sr_in_kv_lst[i].key, MAX_KEY_SIZE, "%zu", i % MOD);
    snprintf(sr_in_kv_lst[i].value, MAX_VALUE_SIZE, "%zu", i);
  }

  struct mr_input sr_input = {sr_in_kv_lst, MAX_DATA_SIZE};
  struct mr_output sr_output;

  bool res = mr_exec(&sr_input, sr_map, 1, sr_reduce, 1, &sr_output) == 0 &&
             sr_call_count == MOD && sr_cmp() == 0;
  free_output(&sr_output);
  TEST(res, 10);

  return res;
}
