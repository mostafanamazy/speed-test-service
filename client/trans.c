#include "trans.h"
#include "printout.h"
#include "globals.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <time.h>

#define MESSAGE         "END"

static int io_timeout (int filedes, unsigned int seconds, _Bool reading);

void trans_to_server (int filedes, configuration conf)
{
  int nbytes = 1;
  size_t total = 0;
  time_t start, end;
  time(&start);
  time(&end);
  char data[MAXMSG];
  memset(data, 'C', sizeof(data));
  int up_time = conf.upload_time > 0 ? conf.upload_time : 10;
  int down_time = conf.download_time > 0 ? conf.download_time : 10;
  int timeout = conf.timeout > 0 ? conf.timeout : 10;
  int ret;

  #define CHECKERROR(status, str) {\
            if(status < 0){\
              perror(str);\
              exit(EXIT_FAILURE);\
            }}
  while(end - start < up_time && nbytes > 0)
   {
     ret = io_timeout(filedes, timeout, 0);
     nbytes = ret > 0 ? write (filedes, data, sizeof(data)) : ret;
     total += nbytes < 0 ? 0 : nbytes;
     time(&end);
   }
  CHECKERROR(nbytes, "write");
  fprintf(stdout,"[Upload] ");
  print_humanable(total, up_time);
  ret = io_timeout(filedes, timeout, 0);
  nbytes = ret > 0 ? write (filedes, MESSAGE, strlen (MESSAGE) + 1) : ret;
  CHECKERROR(nbytes, "write");
  time(&start);
  total = 0;
  while( end - start < down_time && nbytes > 0)
  {
       ret = io_timeout(filedes, timeout, 1);
       nbytes = ret > 0 ? read (filedes, data, sizeof(data)) : ret;
       total += nbytes < 0 ? 0 : nbytes;
       time(&end);
  }
  CHECKERROR(nbytes, "read");
  fprintf(stdout,"[Download] ");
  print_humanable(total, down_time);
}

static int io_timeout (int filedes, unsigned int seconds, _Bool reading)
{
  fd_set set;
  struct timeval timeout;


  /* Initialize the file descriptor set. */
  FD_ZERO (&set);
  FD_SET (filedes, &set);

  /* Initialize the timeout data structure. */
  timeout.tv_sec = seconds;
  timeout.tv_usec = 0;

  /* select returns 0 if timeout, 1 if input available, -1 if error. */
  int ret;
  do
   {
     if(reading)
      ret = select(FD_SETSIZE, &set, NULL, NULL, &timeout);
     else
      ret = select(FD_SETSIZE, NULL, &set, NULL, &timeout);

    } while (ret == -1 && errno == EINTR);
   return ret;
}
