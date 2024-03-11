#include "interface.h"
#include "tests.h"
#include <pthread.h>
#include <stdio.h>

struct thread {
  pthread_t id;
  size_t count;
};

pthread_mutex_t nmr_lock = PTHREAD_MUTEX_INITIALIZER;

bool too_many_threads = false;
struct thread threads[MAX_THREADS] = {0};

void nmr_reset() {
  too_many_threads = false;
  for (size_t i = 0; i < MAX_THREADS; i++) {
    threads[i].count = 0;
  }
}

int thread_cmp(size_t max) {
  size_t count = 0;
  for (size_t i = 0; i < MAX_THREADS; i++) {
    if (threads[i].count > 0) {
      count++;
    }
  }

  if (count != max) {
    return -1;
  }
  return 0;
}

void thread_count() {
  pthread_t self = pthread_self();

  pthread_mutex_lock(&nmr_lock);
  size_t i = 0;
  for (i = 0; i < MAX_THREADS; i++) {
    if (threads[i].count == 0 || pthread_equal(threads[i].id, self)) {
      threads[i].id = self;
      threads[i].count++;
      break;
    }
  }
  if (i == MAX_THREADS) {
    too_many_threads = true;
  }
  pthread_mutex_unlock(&nmr_lock);
}

bool run(void (*nomr_map)(const struct mr_in_kv *),
         void (*nomr_reduce)(const struct mr_out_kv *), int type) {
  struct mr_in_kv nomr_in_kvs[MAX_THREADS];

  for (size_t i = 0; i < MAX_THREADS; i++) {
    snprintf(nomr_in_kvs[i].key, MAX_KEY_SIZE, "%zu", i);
    snprintf(nomr_in_kvs[i].value, MAX_VALUE_SIZE, "%zu", i);
  }

  struct mr_input nomr_input = {nomr_in_kvs, MAX_THREADS};
  struct mr_output nomr_output;

  size_t mappers = 1, reducers = 1;

  bool res = true;

  for (size_t i = 0; i < 5; i++) {
    size_t n = 1 << (i + 1);

    if (type == 0) {
      mappers = n;
    } else {
      reducers = n;
    }

    nmr_reset();
    bool f = mr_exec(&nomr_input, nomr_map, mappers, nomr_reduce, reducers,
                     &nomr_output) == 0 &&
             thread_cmp(mappers == 1 ? reducers : mappers) == 0 &&
             !too_many_threads;
    free_output(&nomr_output);
    TEST(f, 1);
    res = res && f;
  }

  return res;
}

void nom_map(const struct mr_in_kv *in_kv) { thread_count(); }

void nom_reduce(const struct mr_out_kv *inter_kv) {}

bool number_of_mappers(void) { return run(nom_map, nom_reduce, 0); }

void nor_map(const struct mr_in_kv *in_kv) {
  mr_emit_i(in_kv->key, in_kv->value);
}

void nor_reduce(const struct mr_out_kv *inter_kv) { thread_count(); }

bool number_of_reducers(void) { return run(nor_map, nor_reduce, 1); }
