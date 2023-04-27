#include "gpUtilities.h"
#include "gpLib.h"

int main(int argc, char** argv){

    // Controllo degli argomenti passati da linea di comando
    if(argc != 2){
        fprintf(stderr, "usage: %s <Port>\n", argv[0]);
        exit(-1);
    }

    // Definizione dell'Indirizzo
    unsigned short port = strtous(argv[1]);
    char* IPaddress = "127.0.0.1";
    struct sockaddr_in vacCenterAddr = getInitAddr(AF_INET, port, IPaddress);

    // Creazione della connessione e connessione al Centro Vaccinale
    int vacCenterSockfd = Socket(PF_INET, SOCK_STREAM, 0);
    Connect(vacCenterSockfd, vacCenterAddr, sizeof(vacCenterAddr));

    //Inserimento da parte dell'utente del codice fiscale
    requestPacket rp;
    fprintf(stdout, "Inserire Tessera Sanitaria: ");
    scanf(" %s", rp.SSN);
    rp.SSN[SSNSIZE-1] = '\0';
    rp.code = REGISTER_GREEN_PASS;

    // Invio Codice della tessera sanitaria e codice operazione al Centro Vaccinale (dunque il pacchetto rp)
    if(fullWrite(vacCenterSockfd, (void*)&rp, sizeof(rp)) != 0){
        fprintf(stderr, "fullWrite error\n");
        exit(-1);
    }

    // Attesa dell'esito da parte del Centro Vaccinale
    gpInfoPacket gpInfo;
    if(fullRead(vacCenterSockfd, (void*)&gpInfo, sizeof(gpInfo)) != 0){
        fprintf(stderr, "fullRead error\n");
        exit(-1);
    }

    // Analisi dell'esito
    switch(gpInfo.opResult){
        case VACCINE_OK:
            printf("\nVaccinazione effettuata con successo per %s.\n", gpInfo.SSN);
            printf("Data di scadenza del GreenPass: %s\n", ctime(&gpInfo.expDate));
            break;
        case VACCINE_ERROR:
            printf("\nSi e\' verificato un errore durante la registrazione.\n");
            break;
        case SSN_ERROR:
            printf("\nIl numero di caratteri inserito e' insufficiente.\n");
            printf("Il codice fiscale italiano ha 16 caratteri.\n\n");
            break;
        default:
            break;
    }

    Close(vacCenterSockfd);
    return 0;
}