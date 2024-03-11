#include "tests.h"

int main(int argc, char *argv[]) {
  if (single_map() && single_reduce() && single_map_reduce() &&
      number_of_mappers() && number_of_reducers() && partition_input() &&
      partition_intermediate() && full_map_reduce())
    TEST(true, 5);
  return 0;
}
