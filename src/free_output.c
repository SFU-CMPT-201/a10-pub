#include "interface.h"
#include "tests.h"
#include <stdlib.h>

void free_output(struct mr_output *output) {
  if (output == NULL) {
    return;
  }

  if (output->kv_lst == NULL) {
    return;
  }

  for (size_t i = 0; i < output->count; i++) {
    if (output->kv_lst[i].value != NULL) {
      free(output->kv_lst[i].value);
      output->kv_lst[i].value = NULL;
    }
  }
  free(output->kv_lst);
  output->kv_lst = NULL;
}
