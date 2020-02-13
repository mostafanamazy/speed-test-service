#include "printtime.h"

#include <time.h>
#include <stdio.h>

void print_client(const char* addr, int port)
{
  time_t rawtime;
  struct tm *info;
  time( &rawtime );
  info = localtime( &rawtime );
  fprintf (stdout,
    "[%d-%02d-%02d %02d:%02d:%02d] %s:%d\n",
    info->tm_year + 1900,
    info->tm_mon + 1,
    info->tm_mday,
    info->tm_hour,
    info->tm_min,
    info->tm_sec,
    addr,
    port);
}
