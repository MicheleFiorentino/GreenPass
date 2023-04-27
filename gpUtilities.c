#include "gpUtilities.h"


/* BERKELEY WRAPPERS */

int Socket(int domain, int type, int protocol){
    int fd = socket(domain, type, protocol);
    if(fd<0){
        fprintf(stderr,"socket error\n");
        exit(1);
    }
    return fd;
}

void Connect(int fd, struct sockaddr_in addr, socklen_t addrlen){
    int status = connect(fd, (struct sockaddr*)&addr, addrlen);
    if(status<0){
        fprintf(stderr,"connect error\n");
        exit(2);
    }
}

void Bind(int fd, struct sockaddr_in addr, socklen_t addrlen){
    int status = bind(fd, (struct sockaddr*)&addr, addrlen);
    if(status<0){
        fprintf(stderr,"bind error\n");
        exit(3);
    }
}

void Listen(int fd, int backlog){
    int status = listen(fd, backlog);
    if(status < 0){
        fprintf(stderr,"listen error\n");
        exit(4);
    }
}

int Accept(int fd, struct sockaddr *addr, socklen_t *addrlen){
    int connfd = accept(fd, addr, addrlen);
    if(connfd < 0){
        fprintf(stderr, "accept error\n");
        exit(5);
    }
    return connfd;
}

void Close(int fd) {
    if (close(fd) < 0) {
        perror("close error\n");
        exit(6);
    }
}



/* FULLWRITE AND FULLREAD */

ssize_t fullWrite(int fd, const void *buf, size_t count){

    ssize_t nwritten;
    size_t nleft = count;
    while(nleft > 0){
        if( (nwritten = write(fd, buf, nleft)) < 0 ){
            if(errno==EINTR){
                continue;
            } else {
                exit(nwritten);
            }
        }
        nleft -= nwritten;
        buf += nwritten;
    }
    return nleft;
}

ssize_t fullRead(int fd, void *buf, size_t count){

    ssize_t nread;
    size_t nleft = count;
    while(nleft > 0){
        if( (nread = read(fd, buf, nleft)) < 0 ){
            if(errno==EINTR){
                continue;
            } else {
                exit(nread);
            }
        } else if(nread==0){   //EOF
            break;
        }
        nleft -= nread;
        buf += nread;
    }
    buf = 0;
    return(nleft);
}



/* FUNZIONI DI COMUNE UTILITÀ */

//string to unsigned short
unsigned short strtous(char* text){
    unsigned short number = (unsigned short) strtoul(text, NULL, 10);
    /* Verifica errori:
     * - EINVAL: conversione non possibile, caratteri della base non supportati
     * - ERANGE la conversione ha causato un overflow  */
    if (number == 0 && (errno == EINVAL || errno == ERANGE)) {
        perror("strtoul");
        exit(-1);
    }
    return number;
}

// Initializes a struct sockaddr_in addr and returns it
struct sockaddr_in getInitAddr(int family, unsigned short port, char* IP){
    struct sockaddr_in addr;
    memset((void *)&addr, 0, sizeof(addr)); //clean addr
    addr.sin_family = family;
    addr.sin_port = htons(port);
    if(inet_pton(family, IP, &addr.sin_addr) < 0){
        fprintf(stderr, "inet_pton error for %s\n", IP);
        exit(-1);
    }
    return addr;
}

// Connect to ServerV
int connectWithServerV(unsigned short port){
    // Definizione dell'Indirizzo
    char* IPaddress = "127.0.0.1";
    struct sockaddr_in serverVAddr = getInitAddr(AF_INET, port, IPaddress);

    // Creazione della connessione e connessione al ServerV
    int sockfd = Socket(PF_INET, SOCK_STREAM, 0);
    Connect(sockfd, serverVAddr, sizeof(serverVAddr));

    return sockfd;
}