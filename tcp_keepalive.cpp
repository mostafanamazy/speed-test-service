#ifdef _WIN32
#include <winsock2.h>
#include <Ws2def.h>
#include <ws2tcpip.h>
#include "mstcpip.h"
#else
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#endif
 
 
 int fd = d->socket->socketDescriptor();
    if(fd>0)
    {
#ifdef _WIN32
        DWORD         dwBytesRet=0;
        struct tcp_keepalive   alive;
        // Set the keepalive values
        //
        alive.onoff = TRUE;
        alive.keepalivetime = 10000;
        alive.keepaliveinterval = 2000;
        if (WSAIoctl((SOCKET)fd, SIO_KEEPALIVE_VALS, &alive, sizeof(alive),
                NULL, 0, &dwBytesRet, NULL, NULL) == SOCKET_ERROR)
        {
            warning("socket set keepalive error");
            disconnectFromHost();
            return ;
        }
#else
        int enableKeepAlive = 1;
        bool errorInSet = false;
        if(setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &enableKeepAlive, sizeof(enableKeepAlive)) < 0)
            errorInSet = true;
        int maxIdle = 10; /* seconds */
#if defined(Q_OS_MAC)
        if(setsockopt(fd, IPPROTO_TCP, TCP_KEEPALIVE, &maxIdle, sizeof(maxIdle)) < 0)
#else
        if(setsockopt(fd, IPPROTO_TCP, TCP_KEEPIDLE, &maxIdle, sizeof(maxIdle)) < 0)
#endif
            errorInSet = true;
        int count = 3; // send up to 3 keepalive packets out, then disconnect if no response
        if(setsockopt(fd, IPPROTO_TCP, TCP_KEEPCNT, &count, sizeof(count)) < 0)
            errorInSet = true;
        int interval = 2; // send a keepalive packet out every 2 seconds (after the 10 second idle period)
        if(setsockopt(fd, IPPROTO_TCP, TCP_KEEPINTVL, &interval, sizeof(interval)) < 0)
            errorInSet = true;
        if(errorInSet)
        {
            warning("socket set keepalive error");
            disconnectFromHost();
            return ;
        }
#endif
    }
