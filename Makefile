all: Client ClientS ClientT CentroVaccinale ServerG ServerV

Client: Client.o gpUtilities.o gpLib.o
	gcc Client.o gpUtilities.o gpLib.o -o Client.out

Client.o: Client.c gpUtilities.h gpLib.h
	gcc -c Client.c

ClientS: ClientS.o gpUtilities.o gpLib.o
	gcc ClientS.o gpUtilities.o gpLib.o -o ClientS.out

ClientS.o: ClientS.c gpUtilities.h gpLib.h
	gcc -c ClientS.c

ClientT: ClientT.o gpUtilities.o gpLib.o
	gcc ClientT.o gpUtilities.o gpLib.o -o ClientT.out

ClientT.o: ClientT.c gpUtilities.h gpLib.h
	gcc -c ClientT.c


CentroVaccinale: CentroVaccinale.o gpUtilities.o gpLib.o
	gcc CentroVaccinale.o gpUtilities.o gpLib.o -o CentroVaccinale.out

CentroVaccinale.o: CentroVaccinale.c gpUtilities.h gpLib.h
	gcc -c CentroVaccinale.c

ServerG: ServerG.o gpUtilities.o gpLib.o
	gcc ServerG.o gpUtilities.o gpLib.o -o ServerG.out

ServerG.o: ServerG.c gpUtilities.h gpLib.h
	gcc -c ServerG.c

ServerV: ServerV.o threadRequestHandlers.o gpUtilities.o gpLib.o
	gcc ServerV.o gpUtilities.o gpLib.o threadRequestHandlers.o -pthread -o ServerV.out

ServerV.o: ServerV.c gpUtilities.h gpLib.h threadRequestHandlers.h
	gcc -c ServerV.c



gpUtilities.o: gpUtilities.c gpUtilities.h
	gcc -c gpUtilities.c

gpLib.o: gpLib.c gpLib.h
	gcc -c gpLib.c

threadRequestHandlers.o: threadRequestHandlers.c threadRequestHandlers.h
	gcc -c threadRequestHandlers.c


clean:
	rm -f *.o
	rm -f *.out