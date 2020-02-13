#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <time.h>

#include "ini.h"
#include "globals.h"
#include "configuration.h"

#define MAX_CONN        16
#define MAX_EVENTS      32

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

int
read_from_client (int filedes)
{
  char buffer[MAXMSG];
  int nbytes;

  nbytes = read (filedes, buffer, MAXMSG);
  if (nbytes < 0)
    {
      /* Read error. */
//      perror ("read");
      return -1;
    }
  else if (nbytes == 0)
    {
    /* End-of-file. */
//    printf(stdout, "0 byte received");
    return -1;
    }
  else
    {
      /* Data read. */
//      fprintf (stderr, "Server: got message: %c`%d'\n",buffer[nbytes - 1], filedes);
    if (buffer [nbytes -1] == '\0')
    {
       return -2;
     }
      return 0;
    }
}

static int handler(void* user, const char* section, const char* name,
                   const char* value)
{
    configuration* pconfig = (configuration*)user;

    #define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
    if (MATCH("server", "port"))
       pconfig->port = atoi(value);
    else if (MATCH("server", "timeout"))
       pconfig->timeout = atoi(value);
    else if (MATCH("server", "ip"))
       pconfig->ip = strdup(value);
    else if (MATCH("server", "protocol"))
       pconfig->protocol = strdup(value);
    else
       return 0;
    return 1;
}

/*
 * register events of fd to epfd
 */
static void epoll_ctl_add(int epfd, int fd, uint32_t events)
{
	struct epoll_event ev;
	ev.events = events;
	ev.data.fd = fd;
	if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) == -1) {
		perror("epoll_ctl()\n");
		exit(1);
	}
}

static int setnonblocking(int sockfd)
{
	if (fcntl(sockfd, F_SETFD, fcntl(sockfd, F_GETFD, 0) | O_NONBLOCK) ==
	    -1) {
		return -1;
	}
	return 0;
}

int
main (int argc, char *argv[])
{
  configuration config;
  int sock;
  fd_set active_fd_set, read_fd_set;
  int i;
  struct sockaddr_in clientname;
  size_t size;
  int epfd;
	int nfds;
  struct epoll_event events[MAX_EVENTS];
  char buffer[MAXMSG] = {'C'};

  if (argc !=2 || ini_parse(argv[1], handler, &config) < 0)
  {
    fprintf(stdout, "Can't load configuration file.\n");
    exit (EXIT_FAILURE);
  }

  /* Create the socket and set it up to accept connections. */
  sock = make_socket (config.port);
  setnonblocking(sock);
  if (listen (sock, MAX_CONN) < 0)
  {
    perror ("listen");
    exit (EXIT_FAILURE);
  }
  epfd = epoll_create(1);
	epoll_ctl_add(epfd, sock, EPOLLIN | EPOLLOUT);


  while (1)
  {
    /* Block until input arrives on one or more active sockets. */
		nfds = epoll_wait(epfd, events, MAX_EVENTS, -1);

    /* Service all the sockets with input pending. */
		for (i = 0; i < nfds; i++)
    {

      if (events[i].data.fd == sock)
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
          time_t rawtime;
          struct tm *info;
          time( &rawtime );
          info = localtime( &rawtime );
          fprintf (stdout,
            "[%d-%02d-%02d %02d:%02d:%02d] %s:%d.\n",
            info->tm_year + 1900,
            info->tm_mon + 1,
            info->tm_mday,
            info->tm_hour,
            info->tm_min,
            info->tm_sec,
            inet_ntoa (clientname.sin_addr),
            ntohs (clientname.sin_port));
            setnonblocking(new);
            epoll_ctl_add(epfd, new,
            					      EPOLLIN | EPOLLOUT |
            					      EPOLLRDHUP | EPOLLHUP);
          }
          else if (events[i].events & EPOLLIN)
          {
            /* Data arriving on an already-connected socket. */
            int ret = read_from_client (events[i].data.fd);
            if (ret < -1)
            {
                write(events[i].data.fd, buffer, sizeof(buffer));
            }

          }
          else if (events[i].events & EPOLLOUT)
          {
              write(events[i].data.fd, buffer, sizeof(buffer));
          }
          if (events[i].events & (EPOLLRDHUP | EPOLLHUP)) {
  //          printf("[+] connection closed\n");
           epoll_ctl(epfd, EPOLL_CTL_DEL,
                events[i].data.fd, NULL);
            close(events[i].data.fd);
         }
        }
      }
    }
