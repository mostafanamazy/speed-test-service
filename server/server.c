#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>

#define PORT    5555
#define MAXMSG  1024


int
make_socket (uint16_t port)
{
  int sock;
  struct sockaddr_in name;

  /* Create the socket. */
  sock = socket (PF_INET, SOCK_STREAM, 0);
  if (sock < 0)
    {
      perror ("socket");
      exit (EXIT_FAILURE);
    }

  /* Give the socket a name. */
  name.sin_family = AF_INET;
  name.sin_port = htons (port);
  name.sin_addr.s_addr = htonl (INADDR_ANY);
  if (bind (sock, (struct sockaddr *) &name, sizeof (name)) < 0)
    {
      perror ("bind");
      exit (EXIT_FAILURE);
    }

  return sock;
}

static void *
thread_write_client(void *arg)
{
  int filedes = (int) arg;
  char buffer[MAXMSG];
  int nbytes=1;
  memset(buffer, 'C', MAXMSG);
  while (nbytes > 0)
      nbytes = write(filedes, buffer, MAXMSG);
  close(filedes);
}

int
read_from_client (int filedes)
{
  char buffer[MAXMSG];
  int nbytes;

  nbytes = read (filedes, buffer, MAXMSG);
  if (nbytes < 0)
    {
      /* Read error. */
      perror ("read");
      exit (EXIT_FAILURE);
    }
  else if (nbytes == 0)
    {
    /* End-of-file. */
    printf(stdout, "0 byte received");
    return -1;
    }
  else
    {
      /* Data read. */
      fprintf (stderr, "Server: got message: %c`%d'\n",buffer[nbytes - 1], filedes);
    if (buffer [nbytes -1] == '\0')
    {
       pthread_t t;
       if(pthread_create(&t, NULL,thread_write_client, filedes) == 0)
          pthread_detach(t); 
       return -2;
     }
      return 0;
    }
}

int
main (void)
{
  extern int make_socket (uint16_t port);
  int sock;
  fd_set active_fd_set, read_fd_set;
  int i;
  struct sockaddr_in clientname;
  size_t size;

  /* Create the socket and set it up to accept connections. */
  sock = make_socket (PORT);
  if (listen (sock, 1) < 0)
    {
      perror ("listen");
      exit (EXIT_FAILURE);
    }

  /* Initialize the set of active sockets. */
  FD_ZERO (&active_fd_set);
  FD_SET (sock, &active_fd_set);

  while (1)
    {
      /* Block until input arrives on one or more active sockets. */
      read_fd_set = active_fd_set;
      if (select (FD_SETSIZE, &read_fd_set, NULL, NULL, NULL) < 0)
        {
          perror ("select");
          exit (EXIT_FAILURE);
        }

      /* Service all the sockets with input pending. */
      for (i = 0; i < FD_SETSIZE; ++i)
        if (FD_ISSET (i, &read_fd_set))
          {
            if (i == sock)
              {
                /* Connection request on original socket. */
                int new;
                size = sizeof (clientname);
                new = accept (sock,
                              (struct sockaddr *) &clientname,
                              &size);
                if (new < 0)
                  {
                    perror ("accept");
                    exit (EXIT_FAILURE);
                  }
  //perror(inet_ntoa (clientname.sin_addr));
                fprintf (stderr,
                         "Server: connect from host  , port %hd.\n",
                         //clientname.sin_addr ? inet_ntoa (clientname.sin_addr): "",
                         ntohs (clientname.sin_port));
                FD_SET (new, &active_fd_set);
              }
            else
              {
                /* Data arriving on an already-connected socket. */
                int ret = read_from_client (i);
                if (ret < 0)
                  {
                    if (ret == -1)
                       close (i);
                    FD_CLR (i, &active_fd_set);
                  }
              }
          }
    }
}
