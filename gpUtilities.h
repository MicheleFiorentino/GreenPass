#ifndef GPUTILITIES_H
#define GPUTILITIES_H

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>


/* BERKELEY WRAPPERS */

int  Socket(int domain, int type, int protocol);
void Connect(int fd, struct sockaddr_in addr, socklen_t addrlen);
void Bind(int fd, struct sockaddr_in addr, socklen_t addrlen);
void Listen(int fd, int backlog);
int  Accept(int fd, struct sockaddr *addr, socklen_t *addrlen);
void Close(int fd);


/* FULLWRITE AND FULLREAD */

ssize_t fullWrite(int fd, const void *buf, size_t count);
ssize_t fullRead(int fd, void *buf, size_t count);


/* FUNZIONI DI COMUNE UTILITÀ */

//string to unsigned short
unsigned short strtous(char* text);

// Initializes a struct sockaddr_in addr and returns it
struct sockaddr_in getInitAddr(int family, unsigned short port, char* IP);

// Connect to ServerV
int connectWithServerV(unsigned short port);

#endif