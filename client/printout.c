#include "printout.h"
#include <stdio.h>

void print_humanable(size_t bytes, int time)
{
  bytes = bytes / time;
  if(bytes > 1048576)
     fprintf(stdout, "`%ld` MB/s (for %d seconds)\n", bytes >> 20, time);
  else if(bytes > 1024)
     fprintf(stdout, "`%ld` KB/s (for %d seconds)\n", bytes >> 10, time);
  else
     fprintf(stdout, "`%ld` B/s (for %d seconds)\n", bytes, time);
}
