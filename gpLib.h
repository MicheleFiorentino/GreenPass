#ifndef GPLIB_H
#define GPLIB_H

/**
 * La libreria gpLib.h (GreenPass Library) definisce la struttura dei pacchetti, funzioni di
 * di comune utilità, ed alcuni enumeratori
 */

#include <time.h>
#include <string.h>
#include <stdio.h>

/* percorso del file Database contenente le informazioni relative ai GP */
static const char* DATABASE_PATH = "data/gpDatabase.txt";
static const char* TEMP_DATABASE_PATH = "data/tempGpDatabase.txt";

/* PACCHETTI COINVOLTI NELLO SCAMBIO DI DATI */
#define SSNSIZE 17   //Il cod fiscale italiano ha 16 cifre + 1 carattere per '\0'
typedef struct{
    char SSN[SSNSIZE];
    time_t expDate;
    int opResult; 
} gpInfoPacket;

typedef struct{
    char SSN[SSNSIZE];
    unsigned int code;
} requestPacket;


/* ENUMERATORI */

// possibili valori di ritorno dalle diverse operazioni
enum results {
    VACCINE_OK,             //La registrazione al sistema è avvenuta con successo
    VACCINE_ERROR,          //Si è presentato un problema (problemi con la manipolazione dei file)
    GP_VALID,               //Il Green Pass risulta valido (non è scaduto)
    GP_NOT_VALID,           //Il Green Pass non risulta valido (è scaduto)
    GP_NOT_FOUND,           //Il Green Pass associato ad un certo SSN non è stato trovato nel sistema
    GP_ENABLED,             //Guarigione. Il Green pass è stato aggiornato
    GP_NOT_DISABLED,        //Il Green Pass non è stato disabilitato/non è scaduto, quindi non ha senso abilitarlo
    ENABLING_ERROR,         //Il Green Pass era in corso di abilitazione ma si è verificato un errore (problemi con la manipolazione dei file)
    GP_DISABLED,            //Contagio. Il Green pass è stato disabilitato
    DISABLING_ERROR,        //Il Green Pass era in corso di disabilitazione ma si è verificato un errore (problemi con la manipolazione dei file)
    GP_ALREADY_DISABLED,    //Si sta provando a disabilitare un Green Pass già disabilitato (expDate == 0)
    GP_EXPIRED,             //Si sta provando a disabilitare un Green Pass scaduto (non è comunque valido)
    SSN_ERROR               //Il codice Fiscale immesso non è valido
} ;

// richieste effettuabili dai client
enum requestCode{
    REGISTER_GREEN_PASS,            //Client
    CHECK_GREEN_PASS_VALIDITY,      //ClientS
    ENABLE_GREEN_PASS,              //ClientT
    DISABLE_GREEN_PASS,             //ClientT
};



/* FUNZIONI DI COMUNE UTILITÀ */

int checkSSN(char* ssn);                                     //controlla il numero di caratteri del codice fiscale

#endif