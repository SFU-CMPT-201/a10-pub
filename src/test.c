#include "tests.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static size_t SUCCESS_CASES = 0;
static size_t TOTAL_CASES = 0;
static size_t TOTAL_SCORE = 0;

void print_test_result() {
  char buf[48];
  snprintf(buf, 48, "Score: %zu, Success cases: %zu/%zu\n", TOTAL_SCORE,
           SUCCESS_CASES, TOTAL_CASES);
  write(STDOUT_FILENO, buf, strlen(buf));
}

void test(char *file, size_t line, bool f, size_t pts) {
  if (f) {
    SUCCESS_CASES += 1;
    TOTAL_SCORE += pts;
  } else {
    char buf[128];
    snprintf(buf, 128, "Test failed at line %zu in %s\n", line, file);
    write(STDOUT_FILENO, buf, strlen(buf));
  }
  TOTAL_CASES += 1;
  print_test_result();
}
