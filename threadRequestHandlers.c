#include "threadRequestHandlers.h"

void* handleRequest(void* arg){
    int connfd = *(int*)arg;
    requestPacket rp;
    gpInfoPacket gpInfo;

    // Ricezione della richiesta da Centro Vaccinale / ServerG
    memset((void *)&rp, 0, sizeof(rp)); //clean struct
    if(fullRead(connfd, (void*)&rp, sizeof(rp)) != 0){
        fprintf(stderr, "fullWrite error\n");
        exit(-1);
    }

    //Verifica del numero di tessera sanitaria
    int resSSN = checkSSN(rp.SSN);
    if(resSSN == 0){    //SSN valido

        //Analizzo la richiesta per richiamare la funzione opportuna
        switch(rp.code){
            case REGISTER_GREEN_PASS:
                gpInfo = request_RegisterGreenPass(rp.SSN, connfd);
                break;
            case CHECK_GREEN_PASS_VALIDITY:
                gpInfo = request_CheckGreenPassValidity(rp.SSN, connfd);
                break;
            case ENABLE_GREEN_PASS:
                gpInfo = request_EnableGreenPass(rp.SSN, connfd);
                break;
            case DISABLE_GREEN_PASS:
                gpInfo = request_DisableGreenPass(rp.SSN, connfd);
                break;
            default:
                break;
        }

    }
    else{       //SSN non valido
        gpInfo.opResult = SSN_ERROR;
    }

    //Invio del pacchetto a CentroVaccinale/ServerG, il quale contiene anche la risposta
    if(fullWrite(connfd, (void*)&gpInfo, sizeof(gpInfo)) != 0){
        fprintf(stderr, "fullWrite error\n");
        exit(-1);
    }

    Close(connfd);
    free(arg);
}


/* REGISTER GREEN PASS */

gpInfoPacket request_RegisterGreenPass(char* ssn, int connfd){
    gpInfoPacket gpInfo;

    //Informazioni relative al Green Pass
    strcpy(gpInfo.SSN, ssn);
    gpInfo.expDate = time(NULL) + AVGMONTH_IN_SECONDS*6; //tempo corrente + 6 mesi

    // Registra il GP nel DB ed inoltre verifica se 
    // è già presente un GP di quel SSN nel database (in mutua esclusione)
    sem_wait(&mutex);
    int res = registerGreenPassInDatabase(gpInfo);
    sem_post(&mutex);

    //Controllo dell'esito
    if(res == 0)
        gpInfo.opResult = VACCINE_OK;
    else
        gpInfo.opResult = VACCINE_ERROR;
    

    return gpInfo;
}


/* CHECK GREEN PASS VALIDITY */

gpInfoPacket request_CheckGreenPassValidity(char* ssn, int connfd){
    gpInfoPacket gpInfo;

    //Informazioni relative al Green Pass
    strcpy(gpInfo.SSN, ssn);

    //Cerchiamo l'SSN nel database e ritorniamo la data di scadenza del GP
    //Può anche presentarsi la possibilità che il GP non venga trovato
    sem_wait(&mutex);
    gpInfo.expDate = findGreenPassExpDate(ssn);
    sem_post(&mutex);

    //Controllo dell'esito
    //Se l'expDate è 0 secondi, allora il GP è stato invalidato
    //o se è < data corrente, comunque non è valido perché scaduto
    //-1 indica che il GP non è stato trovato
    if(gpInfo.expDate >= time(NULL))                                      //GP trovato, ed è valido
        gpInfo.opResult = GP_VALID;
    else if(gpInfo.expDate == 0 || (gpInfo.expDate > 0 && gpInfo.expDate < time(NULL)) )         //GP trovato, ma non è valido
        gpInfo.opResult = GP_NOT_VALID;
    else                                                                  //GP non trovato, expDate == -1
        gpInfo.opResult = GP_NOT_FOUND; 


    return gpInfo;
}

/* ENABLE GREEN PASS */

gpInfoPacket request_EnableGreenPass(char* ssn, int connfd){

    //Abilitare un Green Pass significa in pratica rilasciarne uno nuovo,
    //dunque si tratta di richiamare la funzione per registrare il Green Pass
    //questo però potremo farlo solo il GreenPass risultava già disabilitato o scaduto
    //dunque dovremo anche controllare il campo expDate

    gpInfoPacket gpInfo;

    //Informazioni relative al Green Pass
    strcpy(gpInfo.SSN, ssn);

    //Cerchiamo l'SSN nel database e ritorniamo la data di scadenza del GP
    //Può anche presentarsi la possibilità che il GP non venga trovato
    sem_wait(&mutex);
    gpInfo.expDate = findGreenPassExpDate(ssn);
    sem_post(&mutex);

    //Controllo dell'esito
    //Se l'expDate è 0 secondi, allora il GP è stato invalidato
    //o se è < data corrente, comunque non è valido perché scaduto
    //-1 indica che il GP non è stato trovato

    if (gpInfo.expDate == -1)                   //Green Pass non trovato
        gpInfo.opResult = GP_NOT_FOUND;
    else if(gpInfo.expDate > time(NULL))        //GP trovato, ma è valido
        gpInfo.opResult = GP_NOT_DISABLED;
    else{                                       //abilitiamo (o comunque aggiorniamo) il GP (ovvero lo registriamo)

        gpInfo.expDate = time(NULL) + AVGMONTH_IN_SECONDS*6; //Nuova data di scadenza, guarigione. tempo corrente + 6 mesi
        
        // Registra il GP nel DB ed inoltre verifica se 
        // è già presente un GP di quel SSN nel database (in mutua esclusione)
        sem_wait(&mutex);
        int res = registerGreenPassInDatabase(gpInfo);
        sem_post(&mutex);

        //Controllo dell'esito
        if(res == 0)
            gpInfo.opResult = GP_ENABLED;
        else
            gpInfo.opResult = ENABLING_ERROR;

    }                                        

    
    return gpInfo;
}

/* DISABLE GREEN PASS */

gpInfoPacket request_DisableGreenPass(char* ssn, int connfd){

    //Disabilitare un Green Pass significa in pratica porre la sua data di scadenza a 0,
    //che significa impostare la data al 1° Gennaio 1970. Questo significa che il GP risulterà
    //sempre "scaduto".
    //Inoltre verificheremo se il GP è stato già disabilitato (non ha senso disabilitare un GP
    //già disabilitato) o se è scaduto (anche qui, non ha senso disabilitare un GP scaduto)

    gpInfoPacket gpInfo;

    //Informazioni relative al Green Pass
    strcpy(gpInfo.SSN, ssn);

    //Cerchiamo l'SSN nel database e ritorniamo la data di scadenza del GP
    //Può anche presentarsi la possibilità che il GP non venga trovato
    sem_wait(&mutex);
    gpInfo.expDate = findGreenPassExpDate(ssn);
    sem_post(&mutex);

    //Controllo dell'esito
    //Se l'expDate è 0 secondi, allora il GP è stato invalidato
    //o se è < data corrente, comunque non è valido perché scaduto
    //-1 indica che il GP non è stato trovato

    if (gpInfo.expDate == -1)                   //Green Pass non trovato
        gpInfo.opResult = GP_NOT_FOUND;
    else if (gpInfo.expDate == 0)               //Green Pass già disabilitato
        gpInfo.opResult = GP_ALREADY_DISABLED;
    else if (gpInfo.expDate < time(NULL))       //Green Pass scaduto
        gpInfo.opResult = GP_EXPIRED;
    else{                                       //disabilitiamo il GP

        gpInfo.expDate = 0; //Impostiamo la scadenza al 1° Gennaio 1970
        
        // Registra il GP nel DB ed inoltre verifica se 
        // è già presente un GP di quel SSN nel database (in mutua esclusione)
        sem_wait(&mutex);
        int res = registerGreenPassInDatabase(gpInfo);
        sem_post(&mutex);

        //Controllo dell'esito
        if(res == 0)
            gpInfo.opResult = GP_DISABLED;
        else
            gpInfo.opResult = DISABLING_ERROR;

    }                                        


    return gpInfo;
}



/* UTILITIES */

int registerGreenPassInDatabase(gpInfoPacket gpInfo){
    FILE* fp, *fptemp;
    char* line = NULL;
    size_t len = 0;
    ssize_t nread;

    //Apertura del file originale in sola lettura
    fp = fopen(DATABASE_PATH, "r");
    if(fp == NULL){
        fprintf(stderr,"file read error. By thread %ld\n", pthread_self());
        return -1;
    }

    //Creazione del file temporaneo in sola scrittura
    fptemp = fopen(TEMP_DATABASE_PATH, "w");
    if(fptemp == NULL){
        fprintf(stderr,"file write error. By thread %ld\n", pthread_self());
        return -2;
    }

    //Copia delle righe nel file temporaneo, tranne eventualmente
    //la riga da sostituire (che proviamo a cercare)
    char tempFileSSN[SSNSIZE];
    while((nread = getline(&line, &len, fp)) != -1){    //Scorriamo per tutte le righe
        strncpy(tempFileSSN, line, SSNSIZE);            //copia del ssn in un buffer temporaneo
        tempFileSSN[SSNSIZE-1] = '\0';                  //Inseriamo il carattere di fine stringa
        if((strcmp(tempFileSSN, gpInfo.SSN)) == 0){            //GP trovato,
            continue;                                   // - in tal caso non copiamo la riga nel file temp.
        }
        fprintf(fptemp, "%s", line);   //altrimenti copiamo la riga nel file temp.
    }
    fclose(fp);             //chiudiamo il file originale
    remove(DATABASE_PATH);  //eliminiamo il file originale

    //Inseriamo il nuovo GreenPass di quel SSN nel DB temp (nb. siamo già alla fine del file)
    //poi facciamo diventare il file temp il file originale (lo rinominiamo)
    fprintf(fptemp, "\n%s %ld", gpInfo.SSN, gpInfo.expDate);
    fclose(fptemp);
    rename(TEMP_DATABASE_PATH, DATABASE_PATH);

    return 0;
}

time_t findGreenPassExpDate(char* ssn){
    FILE* fp;
    char* line = NULL;
    size_t len = 0;
    ssize_t nread;

    //Apertura del file originale in sola lettura
    fp = fopen(DATABASE_PATH, "r");
    if(fp == NULL){
        fprintf(stderr,"file read error. By thread %ld\n", pthread_self());
        return -1;
    }

    //ritorniamo (se c'è) la corrispondenza
    time_t foundExpDate;
    char tempFileSSN[SSNSIZE];
    while((nread = getline(&line, &len, fp)) != -1){    //Scorriamo per tutte le righe
        strncpy(tempFileSSN, line, SSNSIZE);            //copia del ssn in un buffer temporaneo
        tempFileSSN[SSNSIZE-1] = '\0';                  //Inseriamo il carattere di fine stringa
        if((strcmp(tempFileSSN, ssn)) == 0){            //GP trovato,
            foundExpDate = getExpDateFromDBLine(line);  // - in tal caso estrapoliamo la parte relativa all'expDate
            fclose(fp);
            return foundExpDate;
        }
    }

    fclose(fp);
    return -1;      //GP non trovato
}

//Un SSN contiene 16 caratteri, consideriamo uno spazio. Dunque leggeremo dal 18° carattere in poi (17 perché 0 based)
time_t getExpDateFromDBLine(char* line){
    int beginIdx = 17;
    char stringExpDate[20]; //sovrastimo

    strcpy(stringExpDate, line + beginIdx);                 //ottengo la stringa contente l'expDate
    time_t numberExpDate = strtoll(stringExpDate,0,10);     //converto la stringa nel formato time_t

    return numberExpDate;  //ritorno la stringa convertita in long long (time_t)
}