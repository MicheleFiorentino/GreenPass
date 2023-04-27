#ifndef THREADREQUESTHANDLERS_H
#define THREADREQUESTHANDLERS_H

#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include "gpUtilities.h"
#include "gpLib.h"

#define AVGMONTH_IN_SECONDS 2628288 //secondi equivalenti a 1 mese in media
extern sem_t mutex;    //semaforo che consente la mutua esclusione per i file

//funzione principale che analizza la richiesta e richiama la routine opportuna
void* handleRequest(void* arg);

//routine che gestiscono le richieste
gpInfoPacket request_RegisterGreenPass(char* ssn, int connfd);          //Gestisce la richiesta di registrazione del GP da parte di Client
gpInfoPacket request_CheckGreenPassValidity(char* ssn, int connfd);     //Gestisce la richiesta di controllo della validità del GP da parte di ClientS
gpInfoPacket request_EnableGreenPass(char* ssn, int connfd);            //Gestisce la richiesta di abilitazione del GP da parte di ClientT
gpInfoPacket request_DisableGreenPass(char* ssn, int connfd);           //Gestisce la richiesta di disabilitazione del GP da parte di ClientT

//utilità per le routine che gestiscono le richieste
int registerGreenPassInDatabase(gpInfoPacket gpInfo);        //Usato per registare il GP nel database
time_t findGreenPassExpDate(char* ssn);                      //trova (eventualmente) la data di scadenza del GP associato a quel SSN
time_t getExpDateFromDBLine(char* line);                     //estrapola dalla riga (ssn+expDate) solo la data di scadenza e la converte in time_t

#endif