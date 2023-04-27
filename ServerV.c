#include "threadRequestHandlers.h"
#include "gpUtilities.h"
#include "gpLib.h"

#define BACKLOG 1024
sem_t mutex;

int main(int argc, char** argv){
    // Controllo degli argomenti passati da linea di comando
    if(argc != 2){
        fprintf(stderr, "usage: %s <Port>\n", argv[0]);
        exit(-1);
    }

    sem_init(&mutex,0,1);   //semaforo binario per l'accesso in mutua esclusione dei file

    // Definizione della struttura sockaddr_in per il ServerV
    unsigned short port = strtous(argv[1]);
    char* IPaddress = "0.0.0.0"; //INADDR_ANY
    struct sockaddr_in serverVAddr = getInitAddr(AF_INET, port, IPaddress);

    // Creazione della connessione, Bind, e attesa dei Client
    int listenfd = Socket(PF_INET, SOCK_STREAM, 0);
    Bind(listenfd, serverVAddr, sizeof(serverVAddr));
    Listen(listenfd, BACKLOG);
    fprintf(stdout, "ServerV attivo\n\n");

    //Accettazione e Gestione delle richieste
    //Definisco gli attributi per la creazione di un thread detached
    int connfd;
    int *ptrconnfd;
    pthread_t pthread;
    pthread_attr_t detachedThread;
    pthread_attr_init(&detachedThread);
    pthread_attr_setdetachstate(&detachedThread, PTHREAD_CREATE_DETACHED);



    for(;;){
        //Accettazione della connessione
        connfd = Accept(listenfd, (struct sockaddr*)NULL, NULL);

        //preparazione per far gestire la richiesta al thread
        //al thread passeremo la socket della connessione da gestire
        ptrconnfd = malloc(sizeof(int));
        *ptrconnfd = connfd;
        if(pthread_create(&pthread, &detachedThread, &handleRequest, ptrconnfd) != 0){
            fprintf(stderr, "Pthread Error: can't create the Thread\n");
        }

    }

    Close(listenfd);
    sem_destroy(&mutex);
    return 0;
}