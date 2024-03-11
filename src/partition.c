#include "interface.h"
#include "tests.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct partition {
  pthread_t id;
  struct mr_in_kv kv_lst[MAX_DATA_SIZE];
  size_t count;
};

bool too_many_partitions = false;

struct partition partitions[MAX_THREADS];
pthread_mutex_t p_lock = PTHREAD_MUTEX_INITIALIZER;

void pin_map(const struct mr_in_kv *in_kv) {
  pthread_t self = pthread_self();

  pthread_mutex_lock(&p_lock);
  size_t i = 0;
  for (i = 0; i < MAX_THREADS; i++) {
    if (partitions[i].count == 0 || pthread_equal(partitions[i].id, self)) {
      partitions[i].id = self;
      strncpy(partitions[i].kv_lst[partitions[i].count].key, in_kv->key,
              MAX_KEY_SIZE);
      strncpy(partitions[i].kv_lst[partitions[i].count].value, in_kv->value,
              MAX_VALUE_SIZE);
      partitions[i].count++;
      break;
    }
  }
  if (i == MAX_THREADS) {
    too_many_partitions = true;
  }
  pthread_mutex_unlock(&p_lock);
}

void pin_reduce(const struct mr_out_kv *inter_kv) {}

int partition_cmp(struct mr_in_kv *in_kv_lst, size_t num_par) {
  size_t partition_size = MAX_DATA_SIZE / num_par;

  for (size_t i = 0; i < num_par; i++) {
    if (partitions[i].count != partition_size) {
      return -1;
    }
  }

  for (size_t i = 0; i < num_par; i++) {
    size_t base = i * partition_size;
    size_t pos = 0;
    for (pos = 0; pos < num_par; pos++) {
      if (strncmp(partitions[pos].kv_lst[0].key, in_kv_lst[base].key,
                  MAX_KEY_SIZE) == 0)
        break;
    }
    if (pos == num_par) {
      return -1;
    }

    for (size_t j = 0; j < partition_size; j++) {
      if (strncmp(partitions[pos].kv_lst[j].key, in_kv_lst[base + j].key,
                  MAX_KEY_SIZE) != 0 ||
          strncmp(partitions[pos].kv_lst[j].value, in_kv_lst[base + j].value,
                  MAX_VALUE_SIZE) != 0) {

        return -1;
      }
    }
  }

  return 0;
}

void partitions_reset() {
  too_many_partitions = false;
  for (size_t i = 0; i < MAX_THREADS; i++) {
    partitions[i].count = 0;
  }
}

bool partition_input(void) {
  struct mr_in_kv pin_in_kv_lst[MAX_DATA_SIZE];

  for (size_t i = 0; i < MAX_DATA_SIZE; i++) {
    for (size_t j = 0; j < MAX_KEY_SIZE - 1; j++) {
      pin_in_kv_lst[i].key[j] = ' ' + rand() % ('~' - ' ');
    }
    for (size_t j = 0; j < MAX_VALUE_SIZE - 1; j++) {
      pin_in_kv_lst[i].value[j] = ' ' + rand() % ('~' - ' ');
    }
    pin_in_kv_lst[i].key[MAX_KEY_SIZE - 1] = '\0';
    pin_in_kv_lst[i].value[MAX_VALUE_SIZE - 1] = '\0';
  }

  struct mr_input pin_input = {pin_in_kv_lst, MAX_DATA_SIZE};
  struct mr_output pin_output;

  bool res = true;
  for (size_t i = 0; i < 5; i++) {
    size_t n = 1 << (i + 1);

    partitions_reset();
    bool f = mr_exec(&pin_input, pin_map, n, pin_reduce, 1, &pin_output) == 0 &&
             partition_cmp(pin_in_kv_lst, n) == 0 && !too_many_partitions;
    free_output(&pin_output);
    TEST(f, 3);
    res = res && f;
  }

  return res;
}

void pinter_map(const struct mr_in_kv *in_kv) {
  mr_emit_i(in_kv->key, in_kv->value);
}

void pinter_reduce(const struct mr_out_kv *inter_kv) {
  pthread_t self = pthread_self();

  pthread_mutex_lock(&p_lock);
  size_t i = 0;
  for (i = 0; i < MAX_THREADS; i++) {
    if (partitions[i].count == 0 || pthread_equal(partitions[i].id, self)) {
      partitions[i].id = self;
      strncpy(partitions[i].kv_lst[partitions[i].count].key, inter_kv->key,
              MAX_KEY_SIZE);
      strncpy(partitions[i].kv_lst[partitions[i].count].value,
              inter_kv->value[0], MAX_VALUE_SIZE);
      partitions[i].count++;
      break;
    }
  }
  if (i == MAX_THREADS) {
    too_many_partitions = true;
  }
  pthread_mutex_unlock(&p_lock);
}

bool partition_intermediate(void) {
  struct mr_in_kv pinter_in_kv_lst[MAX_DATA_SIZE];

  for (size_t i = 0; i < MAX_DATA_SIZE; i++) {
    snprintf(pinter_in_kv_lst[i].key, MAX_KEY_SIZE, "%4zu", i);
    snprintf(pinter_in_kv_lst[i].value, MAX_VALUE_SIZE, "%4zu", i);
  }

  struct mr_input pinter_input = {pinter_in_kv_lst, MAX_DATA_SIZE};
  struct mr_output pinter_output;

  bool res = true;
  for (size_t i = 0; i < 5; i++) {
    size_t r = 1 << (i + 1);

    partitions_reset();
    bool f = mr_exec(&pinter_input, pinter_map, 1, pinter_reduce, r,
                     &pinter_output) == 0 &&
             partition_cmp(pinter_in_kv_lst, r) == 0 && !too_many_partitions;
    free_output(&pinter_output);
    res = res && f;
  }
  TEST(res, 15);

  return res;
}
