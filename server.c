#include <stdio.h>      
#include <sys/types.h>
#include <sys/socket.h>   
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> 

#define MAX_CHAR 512
// #define true 1
// #define false 2
//I tipi di messaggi
#define START 45645
#define SOCKETCR 76567
#define BINDADRS 43654
#define SOCKETLS 63473
#define CONNECTA 47645

const char MESSAGE[] = "Hello UPO student!\n";

// invia messaggi di errore del server
int error_checking(int outcome, int type);

int main(int argc, char *argv[])
{
    // lunghezza massima di 512 caratteri
    char buffer[MAX_CHAR] = "";
    
    int simpleSocket = 0;
    int simplePort = 0;
    int returnStatus = 0;
    struct sockaddr_in simpleServer;

    if (error_checking(2 == argc, START))
    {
        fprintf(stderr, "%s <port>'\n", argv[0]);
        exit(1);
    }

    simpleSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    // verifica creazione socket
    error_checking(simpleSocket != -1, SOCKETCR);

    /* retrieve the port number for listening */
    simplePort = atoi(argv[1]);

    /* setup the address structure */
    /* use INADDR_ANY to bind to all local addresses  */
    memset(&simpleServer, '\0', sizeof(simpleServer)); 
    simpleServer.sin_family = AF_INET;
    simpleServer.sin_addr.s_addr = htonl(INADDR_ANY);
    simpleServer.sin_port = htons(simplePort);

    /*  bind to the address and port with our socket  */
    returnStatus = bind(simpleSocket,(struct sockaddr *)&simpleServer,sizeof(simpleServer));

    // verifica bind
    if (error_checking(returnStatus == 0, BINDADRS))
    {
        close(simpleSocket);
        exit(1);
    }

    /* lets listen on the socket for connections      */
    returnStatus = listen(simpleSocket, 5);

    // verifica socket
    if (error_checking(returnStatus != -1, SOCKETLS))
    {
        close(simpleSocket);
        exit(1);
    }

    while (1)
    {
        struct sockaddr_in clientName = { 0 };
        int simpleChildSocket = 0;
        int clientNameLength = sizeof(clientName);

        /* wait here */

        simpleChildSocket = accept(simpleSocket,(struct sockaddr *)&clientName, &clientNameLength);

        // verfica se e' stata accettata la connessione
        if(error_checking(simpleChildSocket != -1, CONNECTA))
        {
            close(simpleSocket);
            //exit(1);
        }

        /* handle the new connection request  */
        /* write out our message to the client */
        write(simpleChildSocket, MESSAGE, strlen(MESSAGE));
        close(simpleChildSocket);
    }

    close(simpleSocket);
    return 0;
}

int error_checking(int outcome, int type)
{
    // identifica l’esito positivo o negativo del messaggio e può assumere i valori OK ed ERR
    if (outcome)
    {
        fprintf(stderr , "OK ");
        // identifica il comando al quale la risposta fa riferimento, o la categoria di risposta.
        switch (type)
        {
            case START:
                fprintf(stderr, "START 'Welcome'\n");
                return 0;
            case SOCKETCR:
                fprintf(stderr, "SOCKETCR 'Socket created!'\n");
                return 0;
            case BINDADRS:
                fprintf(stderr, "BINDADRS 'Bind completed!'\n");
                return 0;
            case SOCKETLS:
                fprintf(stderr, "SOCKETLS 'Can listen on socket!'\n");
                return 0;
            case CONNECTA:
                fprintf(stderr, "CONNECTA 'Connection accepted!'\n");
                return 0;
        }
    }
    else
    {
        fprintf(stderr , "ERR ");
        // identifica il comando al quale la risposta fa riferimento, o la categoria di risposta.
        switch (type)
        {
            case START:
                fprintf(stderr, "START 'Usage: ");
                return 1;
            case SOCKETCR:
                fprintf(stderr, "SOCKETCR 'Could not create a socket!'\n");
                return 1;
            case BINDADRS:
                fprintf(stderr, "BINDADRS 'Could not bind to address!'\n");
                return 1;
            case SOCKETLS:
                fprintf(stderr, "SOCKETLS 'Cannot listen on socket!'\n");
                return 1;
            case CONNECTA:
                fprintf(stderr, "CONNECTA 'Cannot accept connections!'\n");
                return 1;
        }
    }
}