#include "gpLib.h"

int checkSSN(char* ssn){
    int len = (int)strlen(ssn);
    if(len == SSNSIZE-1)
        return 0;
    else
        return -1;
}