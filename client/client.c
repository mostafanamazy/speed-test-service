#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <errno.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>

#include "ini.h"
#include "globals.h"

#define MESSAGE         "END"

typedef struct
{
    int port;
    const char* ip;
    int timeout;
    int download_time;
    int upload_time;
} configuration;


void
init_sockaddr (struct sockaddr_in *name,
               const char *hostname,
               uint16_t port)
{
  struct hostent *hostinfo;

  name->sin_family = AF_INET;
  name->sin_port = htons (port);
  hostinfo = gethostbyname (hostname);
  if (hostinfo == NULL)
    {
      fprintf (stderr, "Unknown host %s.\n", hostname);
      exit (EXIT_FAILURE);
    }
  name->sin_addr = *(struct in_addr *) hostinfo->h_addr;
}

int
io_timeout (int filedes, unsigned int seconds, _Bool reading)
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

void
print_humanable(size_t bytes, int time)
{
  bytes = bytes / time;
  if(bytes > 1048576)
     fprintf(stdout, "`%d` MB/s (for %d seconds)\n", bytes >> 20, time);
  else if(bytes > 1024)
     fprintf(stdout, "`%d` KB/s (for %d seconds)\n", bytes >> 10, time);
  else
     fprintf(stdout, "`%d` B/s (for %d seconds)\n", bytes, time);
}

void
write_to_server (int filedes, configuration conf)
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

  while(end - start < conf.upload_time && nbytes > 0)
   {
     ret = io_timeout(filedes, timeout, 0);
     nbytes = ret > 0 ? write (filedes, data, sizeof(data)) : ret;
     total += nbytes < 0 ? 0 : nbytes;
     time(&end);
   }
  fprintf(stdout,"[Upload] ");
  print_humanable(total, up_time);
  ret = io_timeout(filedes, timeout, 0);
  nbytes = ret > 0 ? write (filedes, MESSAGE, strlen (MESSAGE) + 1) : ret;
  if (nbytes < 0)
    {
      perror ("write");
      exit (EXIT_FAILURE);
    }
    time(&start);
    total = 0;
    while( end - start < down_time && nbytes > 0)
    {
       ret = io_timeout(filedes, timeout, 1);
       nbytes = ret > 0 ? read (filedes, data, sizeof(data)) : ret;
       total += nbytes < 0 ? 0 : nbytes;
       time(&end);
    }
  fprintf(stdout,"[Download] ");
  print_humanable(total, down_time);
}

static int handler(void* user, const char* section, const char* name,
                   const char* value)
{
    configuration* pconfig = (configuration*)user;

    #define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
    if (MATCH("server", "address_port")) {
        pconfig->port = atoi(value);
    } else if (MATCH("server", "address_ip")) {
        pconfig->ip = strdup(value);
    } else if (MATCH("server", "timeout")) {
        pconfig->timeout = atoi(value);
    } else if (MATCH("test", "download_time")) {
        pconfig->download_time = atoi(value);
    } else if (MATCH("test", "upload_time")) {
        pconfig->upload_time = atoi(value);
    } else {
        return 0;  /* unknown section/name, error */
    }
    return 1;
}

int
main (int argc, char* argv[])
{
  configuration config;
  if (argc !=2 || ini_parse(argv[1], handler, &config) < 0)
   {
      printf("Can't load configuration file.\n");
      exit (EXIT_FAILURE);
   }
  int sock;
  struct sockaddr_in servername;

  /* Create the socket. */
  sock = socket (PF_INET, SOCK_STREAM, 0);
  if (sock < 0)
    {
      perror ("socket (client)");
      exit (EXIT_FAILURE);
    }

  /* Connect to the server. */
  init_sockaddr (&servername, config.ip, config.port);
  if (0 > connect (sock,
                   (struct sockaddr *) &servername,
                   sizeof (servername)))
    {
      perror ("connect (client)");
      exit (EXIT_FAILURE);
    }

  /* Send data to the server. */
  write_to_server (sock, config);
  close (sock);
  exit (EXIT_SUCCESS);
}
