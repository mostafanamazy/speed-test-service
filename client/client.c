#include "ini.h"
#include "sockaddress.h"
#include "trans.h"

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
  trans_to_server (sock, config);
  close (sock);
  exit (EXIT_SUCCESS);
}
