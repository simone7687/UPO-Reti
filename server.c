#include <stdio.h>      
#include <sys/types.h>
#include <sys/socket.h>   
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> 

#define MAX_CHAR 512    /* lunghezza massima */

// I tipi di messaggi
#define ARGOMENT 45645  /* argomenti */
#define SOCKETCR 76567  /* creazione socket */
#define BINDADRS 43654  /* bind address */
#define SOCKETLS 63473  /* socket connesso */
#define CONNECTA 47645  /* connesione accettata */

#define TEXT 5466
#define HIST 4664
#define EXIT 4576
#define QUIT 4575

const char MESSAGE[] = "Hello UPO student!\n";

int error_checking(int outcome, int type, int simpleChildSocket);   /* Invia messaggi di errore del server #1 */
void client_waiting(int sockfd);    /* Il server si pone in attesa di un messaggio di comando #4 */

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

    //while (1)
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

        client_waiting(simpleChildSocket);
        /* handle the new connection request  */
        /* write out our message to the client */
        //write(simpleChildSocket, MESSAGE, strlen(MESSAGE));
        // close(simpleChildSocket);
    }

    close(simpleSocket);
    return 0;
}

int error_checking(int outcome, int type, int simpleChildSocket)
{
    // lunghezza massima di 512 caratteri #1
    char buffer[MAX_CHAR];
    char err[] = "ERR ";
    char ok[] = "OK ";

    // identifica l’esito positivo o negativo del messaggio e può assumere i valori OK ed ERR
    if (outcome)
    {
        strcpy(buffer, ok);
        // identifica il comando al quale la risposta fa riferimento, o la categoria di risposta.
        switch (type)
        {
            case ARGOMENT:
                strcat(buffer, "ARGOMENT 'Correct argument'");
                break;
            case SOCKETCR:
                strcat(buffer, "SOCKETCR 'Socket created!'");
                break;
            case BINDADRS:
                strcat(buffer, "BINDADRS 'Bind completed!'");
                break;
            case SOCKETLS:
                strcat(buffer, "SOCKETLS 'Can listen on socket!'");
                break;
            // All’apertura della connessione il server manda un messaggio di benvenuto (server) #2
            case CONNECTA:
                fprintf(stderr, "OK CONNECTA 'Connection accepted!'\n");  // Non viene visualizzato dal client
                strcat(buffer, "START 'Welcome!'");
                break;
        }
    }
    else
    {
        strcpy(buffer, err);
        // identifica il comando al quale la risposta fa riferimento, o la categoria di risposta.
        switch (type)
        {
            case ARGOMENT:
                strcat(buffer, "ARGOMENT 'Usage: ");
                fprintf(stderr, "%s", buffer);
                return !outcome;
            case SOCKETCR:
                strcat(buffer, "SOCKETCR 'Could not create a socket!'");
                break;
            case BINDADRS:
                strcat(buffer, "BINDADRS 'Could not bind to address!'");
                break;
            case SOCKETLS:
                strcat(buffer, "SOCKETLS 'Cannot listen on socket!'");
                break;
            case CONNECTA:
                strcat(buffer, "CONNECTA 'Cannot accept connections!'");
                break;
        }
    }
    strcat(buffer, "\n");

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
    return !outcome;
}
int controlcommand(char buffer[])   /* Controlla la corratterza del comando */
{
    if (strncmp(buffer, "TEXT ", 5) == 0)
    {return TEXT;}
    else if (strncmp(buffer, "HIST ", 5) == 0)
    {return HIST;}
    else if (strncmp(buffer, "EXIT ", 5) == 0)
    {return EXIT;}
    else if (strncmp(buffer, "QUIT ", 5) == 0)
    {return QUIT;}
    // In caso il comando non fosse riconosciuto, ritorna 0
    return 0;
}

void client_waiting(int simpleChildSocket)
{
    int i = 1;
    char buffer[MAX_CHAR];
    int returnStatus = 0;
    while (1)
    {
        // Una volta eseguita un'operazione, svuoto il buffer per le operazioni successive o per le connessioni successive
        memset(&buffer, '\0', sizeof(buffer));
        returnStatus = read(simpleChildSocket, buffer, sizeof(buffer)); // Ricevo il messaggio del client
        if (returnStatus > 0)   // Nel caso abbia ricevuto qualcosa svolgo le operazioni successive
        {
            printf("MESSAGGIO RICEVUTO:  %s\n", buffer);
            i = controlcommand(buffer);
            switch (i)
            {
                case TEXT:
                    break;
                case HIST:
                    break;
                case EXIT:
                    break;
                case QUIT:
                    break;
                default:
                    break;
            }
        }
        else 
        {
        }
    }
} 