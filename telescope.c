#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <fcntl.h>      // File control definitions
#include <errno.h>      // Error number definitions
#include <termios.h>    // POSIX terminal control definitions

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


void serialTelescope(const char *port, const int axis) {
  int usb = open( port, O_RDWR| O_NOCTTY );

  struct termios tty;
  memset (&tty, 0, sizeof tty);

  /* Error Handling */
  if ( tcgetattr ( usb, &tty ) != 0 ) {
    printf("Error %d from tcgetattr: %s\n", errno, strerror(errno)) ;
  }

  /* Set Baud Rate */
  cfsetospeed (&tty, (speed_t)B115200);
  cfsetispeed (&tty, (speed_t)B115200);

  /* Setting other Port Stuff */
  tty.c_cflag     &=  ~PARENB;            // Make 8n1
  tty.c_cflag     &=  ~CSTOPB;
  tty.c_cflag     &=  ~CSIZE;
  tty.c_cflag     |=  CS8;

  tty.c_cflag     &=  ~CRTSCTS;           // no flow control
  tty.c_cc[VMIN]   =  1;                  // read doesn't block
  tty.c_cc[VTIME]  =  5;                  // 0.5 seconds read timeout
  tty.c_cflag     |=  CREAD | CLOCAL;     // turn on READ & ignore ctrl lines

  /* Make raw */
  cfmakeraw(&tty);

  /* Flush Port, then applies attributes */
  tcflush( usb, TCIFLUSH );
  if ( tcsetattr ( usb, TCSANOW, &tty ) != 0) {
     printf("Error %d from tcsetattr\n", errno);
  }


  int written = write( usb, &axis, 1);
  if (written != 1) {
    printf("Error not writen correct side");
  }
}
