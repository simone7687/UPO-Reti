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
#define ARGOMENT 45645
#define SOCKETCR 76567
#define BINDADRS 43654
#define SOCKETLS 63473
#define CONNECTA 47645

const char MESSAGE[] = "Hello UPO student!\n";

// invia messaggi di errore del server
int error_checking(int outcome, int type, int simpleChildSocket);

int main(int argc, char *argv[])
{
    int simpleSocket = 0;
    int simplePort = 0;
    int returnStatus = 0;
    struct sockaddr_in simpleServer;
    
    int clientNameLength = 0;

    if (error_checking(2 == argc, ARGOMENT, 0))
    {
        fprintf(stderr, "%s <port>'\n", argv[0]);
        exit(1);
    }

    simpleSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    // verifica creazione socket
    if (error_checking(simpleSocket != -1, SOCKETCR, 0))
    {
        exit(1);
    }

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
    if (error_checking(returnStatus == 0, BINDADRS, 0))
    {
        close(simpleSocket);
        exit(1);
    }

    /* lets listen on the socket for connections      */
    returnStatus = listen(simpleSocket, 5);

    // verifica socket
    if (error_checking(returnStatus != -1, SOCKETLS, 0))
    {
        close(simpleSocket);
        exit(1);
    }

    while (1)
    {
        struct sockaddr_in clientName = { 0 };
        int simpleChildSocket = 0;
        clientNameLength = sizeof(clientName);

        /* wait here */

        simpleChildSocket = accept(simpleSocket,(struct sockaddr *)&clientName, &clientNameLength);

        // verfica se e' stata accettata la connessione
        if (error_checking(simpleChildSocket != -1, CONNECTA, simpleChildSocket))
        {
            close(simpleSocket);
            exit(1);
        }

        /* handle the new connection request  */
        /* write out our message to the client */
        write(simpleChildSocket, MESSAGE, strlen(MESSAGE));
        close(simpleChildSocket);
    }

    close(simpleSocket);
    return 0;
}

int error_checking(int outcome, int type, int simpleChildSocket)
{
    // lunghezza massima di 512 caratteri
    char buffer[MAX_CHAR];

    // identifica l’esito positivo o negativo del messaggio e può assumere i valori OK ed ERR
    if (outcome)
    {
        // identifica il comando al quale la risposta fa riferimento, o la categoria di risposta.
        switch (type)
        {
            case ARGOMENT:
                strcpy(buffer, "OK ARGOMENT 'Correct argument'\n");
                break;
            case SOCKETCR:
                strcpy(buffer, "OK SOCKETCR 'Socket created!'\n");
                break;
            case BINDADRS:
                strcpy(buffer, "OK BINDADRS 'Bind completed!'\n");
                break;
            case SOCKETLS:
                strcpy(buffer, "OK SOCKETLS 'Can listen on socket!'\n");
                break;
            case CONNECTA:
                fprintf(stderr, "OK CONNECTA 'Connection accepted!'\n");  // Non viene visualizzato dal client
                strcpy(buffer, "OK START 'Welcome!'\n");
                break;
        }
    }
    else
    {
        // identifica il comando al quale la risposta fa riferimento, o la categoria di risposta.
        switch (type)
        {
            case ARGOMENT:
                strcpy(buffer, "ERR ARGOMENT 'Usage: ");
                break;
            case SOCKETCR:
                strcpy(buffer, "ERR SOCKETCR 'Could not create a socket!'\n");
                break;
            case BINDADRS:
                strcpy(buffer, "ERR BINDADRS 'Could not bind to address!'\n");
                break;
            case SOCKETLS:
                strcpy(buffer, "ERR SOCKETLS 'Cannot listen on socket!'\n");
                break;
            case CONNECTA:
                strcpy(buffer, "ERR CONNECTA 'Cannot accept connections!'\n");
                break;
        }
    }

    // verifivo se può essere inviato anche al client
    if (simpleChildSocket == 0)
    {
        fprintf(stderr, "%s", buffer);
    }
    else
    {
        fprintf(stderr, "%s", buffer);
        // invia un messaggio al client
        write(simpleChildSocket, buffer, strlen(buffer));
    }
    // input di chiusura del programma
    outcome = !outcome;
    return outcome;
}