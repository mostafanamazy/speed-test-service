#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <errno.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>

#define PORT            5555
#define MESSAGE         "END"
#define SERVERHOST      "127.0.0.1"
#define ELAPSE		10
#define ELAPSED		10
#define TIMEOUT         10

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
input_timeout (int filedes, unsigned int seconds)
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
      ret = select(FD_SETSIZE, &set, NULL, NULL, &timeout);
    } while (ret == -1 && errno == EINTR);
   return ret;
}


void
write_to_server (int filedes)
{
  int nbytes = 1;
  size_t total = 0;
  time_t start, end;
  time(&start);
  time(&end);
  char data[1024];
  memset(data, 'C', sizeof(data));

  while(end - start < ELAPSE && nbytes > 0)
   {
     nbytes = write (filedes, data, sizeof(data));
     total += nbytes < 0 ? 0 : nbytes;
     time(&end);
   }
   fprintf(stdout,"%ld \n" , total);
  nbytes = write (filedes, MESSAGE, strlen (MESSAGE) + 1);
  if (nbytes < 0)
    {
      perror ("write");
      exit (EXIT_FAILURE);
    }
    start = end;
    total = 0;
    while( end - start < ELAPSED && nbytes > 0)
    {
       int ret = input_timeout(filedes, TIMEOUT);  
       nbytes = ret > 0 ? read (filedes, data, sizeof(data)) : ret;
       total += nbytes < 0 ? 0 : nbytes;
       time(&end);
    }
   fprintf(stdout,"%ld" , total);
}


int
main (void)
{
  extern void init_sockaddr (struct sockaddr_in *name,
                             const char *hostname,
                             uint16_t port);
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
  init_sockaddr (&servername, SERVERHOST, PORT);
  if (0 > connect (sock,
                   (struct sockaddr *) &servername,
                   sizeof (servername)))
    {
      perror ("connect (client)");
      exit (EXIT_FAILURE);
    }

  /* Send data to the server. */
  write_to_server (sock);
  close (sock);
  exit (EXIT_SUCCESS);
}
