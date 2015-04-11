#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#include "telescope.h"


void signalTelescope(const char *address, const char *port, const int axis) {
  printf("in signal telescope(%s:%s/?=%d)\n", address, port, axis);
  char request[1000];
  struct hostent *server;
  struct sockaddr_in serveraddr;

  int tcpSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (tcpSocket < 0) {
    printf("Error opening socket\n");
    return;
  }

  server = gethostbyname(address);
  if (server == NULL) {
    printf("gethostbyname() failed\n");
    return;
  }


  printf("\n");

  memset(&serveraddr, 0, sizeof(serveraddr));
  serveraddr.sin_family = AF_INET;

  memcpy(server->h_addr_list[0], &serveraddr.sin_addr.s_addr, server->h_length);

  serveraddr.sin_port = htons(atoi(port));
  if (connect(tcpSocket, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0) {
    printf("Error Connecting\n");
    return;
  }

  memset(request, 0, 1000);

  sprintf(request, "Get /?%d HTTP/1.1\r\nHost:%s\r\n\r\n", axis, address);

  printf("%s\n", request);

  if (send(tcpSocket, request, strlen(request), 0) < 0) {
    printf("Error with send()\n");
  }

  memset(request, 0, 1000);

  recv(tcpSocket, request, 999, 0);
  printf("\n%s", request);

  close(tcpSocket);
}
