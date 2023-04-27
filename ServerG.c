#include <stdio.h>
#include <signal.h>
#include "gpUtilities.h"
#include "gpLib.h"

#define BACKLOG 1024

int main(int argc, char** argv){

    // Controllo degli argomenti passati da linea di comando
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <Porta ServerG> <Porta ServerV>.\n", argv[0]);
        exit(-1);
    }

    signal(SIGCHLD, SIG_IGN);   //i figli non hanno bisogno di essere raccolti dal padre

    // Definizione della struttura sockaddr_in per il ServerG
    unsigned short port = strtous(argv[1]);
    char* IPaddress = "0.0.0.0"; //INADDR_ANY
    struct sockaddr_in serverGAddr = getInitAddr(AF_INET, port, IPaddress);

    // Creazione della connessione, Bind, e attesa dei Client
    int listenfd = Socket(PF_INET, SOCK_STREAM, 0);
    Bind(listenfd, serverGAddr, sizeof(serverGAddr));
    Listen(listenfd, BACKLOG);
    fprintf(stdout, "ServerG attivo\n\n");

    //Accettazione e Gestione delle richieste
    int connfd, serverVsockfd, pid;
    requestPacket rp;
    gpInfoPacket gpInfo;

    for(;;){
        //Accettazione della connessione (connfd può riferirsi sia a un ClientS che a un ClientT)
        connfd = Accept(listenfd, (struct sockaddr*)NULL, NULL);

        //Creazione processo figlio
        pid = fork();
        if(pid < 0){
            perror("fork error");
            exit(-1);
        }
        else if(pid == 0){  //figlio. Gestisce la richiesta
            Close(listenfd);

            // Connessione con ServerV
            unsigned short serverVPort = strtous(argv[2]);
            serverVsockfd = connectWithServerV(serverVPort);

            // 1. Ricezione della richiesta da ClientS/ClientT
            memset((void *)&rp, 0, sizeof(rp)); //clean struct
            if(fullRead(connfd, (void*)&rp, sizeof(rp)) != 0){
                fprintf(stderr, "fullRead1 error\n");
                exit(-1);
            }

            // 2 Invio della richiesta di ClientS/ClientT a ServerV
            if(fullWrite(serverVsockfd, (void*)&rp, sizeof(rp)) != 0){
                fprintf(stderr, "fullWrite1 error\n");
                exit(-1);
            }

            // 3 Attesa dell'esito da parte di ServerV
            memset((void *)&gpInfo, 0, sizeof(gpInfo)); //clean struct
            if(fullRead(serverVsockfd, (void*)&gpInfo, sizeof(gpInfo)) != 0){
                fprintf(stderr, "fullRead2 error\n");
                exit(-1);
            }

            // 4 Invio dell'esito a ClientS/ClientT
            if(fullWrite(connfd, (void*)&gpInfo, sizeof(gpInfo)) != 0){
                fprintf(stderr, "fullWrite2 error\n");
                exit(-1);
            }

            Close(connfd);
            Close(serverVsockfd);
            exit(0);
        }
        else{   //padre. Passa all'accettazione di una nuova richiesta
            Close(connfd);
        }
    }

    Close(listenfd);
    exit(0);
}