#include <stdio.h>      
#include <sys/types.h>
#include <sys/socket.h>   
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> 
#include <ctype.h>

#define MAX_CHAR 512    /* lunghezza massima */

// I tipi di messaggi
#define ARGO 45645  /* argomenti */
#define SOCR 76567  /* creazione socket */
#define BIND 43654  /* bind address */
#define SOLS 63473  /* socket connesso */
#define CONT 47645  /* connesione accettata */
#define SYNTAX 75648  /* sintassi */
#define DATA 45748
#define ZEROVAL 45678

#define TEXT 5466
#define HIST 4664
#define EXIT 4576
#define QUIT 4575

const char MESSAGE[] = "Hello UPO student!\n";

int error_checking(int outcome, int type, int simpleChildSocket);   /* Invia messaggi di errore del server #1 (ritorna 1 per uscire dal programma)*/
int client_waiting(int sockfd);    /* Il server si pone in attesa di un messaggio di comando #4 (ritorna 0 per concludere la connesione)*/

int main(int argc, char *argv[])
{
    int simpleSocket = 0;
    int simplePort = 0;
    int returnStatus = 0;
    struct sockaddr_in simpleServer;
    
    int clientNameLength = 0;

    if (error_checking(2 == argc, ARGO, 0))
    {
        fprintf(stderr, "%s <port>'\n", argv[0]);
        exit(1);
    }

    simpleSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    // verifica creazione socket
    if (error_checking(simpleSocket != -1, SOCR, 0))
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
    if (error_checking(returnStatus == 0, BIND, 0))
    {
        close(simpleSocket);
        exit(1);
    }

    /* lets listen on the socket for connections      */
    returnStatus = listen(simpleSocket, 5);

    // verifica socket
    if (error_checking(returnStatus != -1, SOLS, 0))
    {
        close(simpleSocket);
        exit(1);
    }

    int x = 1;
    while (1)
    {
        struct sockaddr_in clientName = { 0 };
        int simpleChildSocket = 0;
        clientNameLength = sizeof(clientName);

        /* wait here */

        simpleChildSocket = accept(simpleSocket,(struct sockaddr *)&clientName, &clientNameLength);

        // verfica se e' stata accettata la connessione
        if (error_checking(simpleChildSocket != -1, CONT, simpleChildSocket))
        {
            close(simpleSocket);
            exit(1);
        }

        x = client_waiting(simpleChildSocket);
        /* handle the new connection request  */
        /* write out our message to the client */
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
    memset(&buffer, '\0', sizeof(buffer));

    // identifica l’esito positivo o negativo del messaggio e può assumere i valori OK ed ERR
    if (outcome)
    {
        strcpy(buffer, ok);
        // identifica il comando al quale la risposta fa riferimento, o la categoria di risposta.
        switch (type)
        {
            case ARGO:
                strcat(buffer, "ARGO 'Correct argument'");
                break;
            case SOCR:
                strcat(buffer, "SOCR 'Socket created!'");
                break;
            case BIND:
                strcat(buffer, "BIND 'Bind completed!'");
                break;
            case SOLS:
                strcat(buffer, "SOLS 'Can listen on socket!'");
                break;
            // All’apertura della connessione il server manda un messaggio di benvenuto (server) #2
            case CONT:
                fprintf(stderr, "OK CONT 'Connection accepted!'\n");  // Non viene visualizzato dal client
                strcat(buffer, "START 'Welcome!'");
                break;
            case SYNTAX:
                strcat(buffer, "SYNTAX 'Correct syntax!'");
                break;
            case QUIT:
                strcat(buffer, "QUIT 'Connection closed'");//, wait another client'");
                break;
            case EXIT:
                strcat(buffer, "EXIT 'Connection closed'");//, wait another client'");
                break;
        }
    }
    else
    {
        strcpy(buffer, err);
        // identifica il comando al quale la risposta fa riferimento, o la categoria di risposta.
        switch (type)
        {
            case ARGO:
                strcat(buffer, "ARGO 'Usage: ");
                fprintf(stderr, "%s", buffer);
                return !outcome;
            case SOCR:
                strcat(buffer, "SOCR 'Could not create a socket!'");
                break;
            case BIND:
                strcat(buffer, "BIND 'Could not bind to address!'");
                break;
            case SOLS:
                strcat(buffer, "SOLS 'Cannot listen on socket!'");
                break;
            case CONT:
                strcat(buffer, "CONT 'Cannot accept connections!'");
                break;
            case SYNTAX:
                strcat(buffer, "SYNTAX 'Incorrect syntax!'");
                break;
            case QUIT:
                strcat(buffer, "QUIT 'Connection closed, end of the program'");
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

int controlcommandsyntax(char val[])   /* Correttezza dei messaggi ricevuti (sintassi) #7 */
{
    // if (val[strlen(val)] != '\n')
    //     return 0;
    if (strlen(val) > MAX_CHAR)
        return 0;
    char head[strlen(val)];
    int l;
    int k = 0;
    memset(&head, '\0', sizeof(head));
    for (int i = 0; val[i] != ' ' && val[i] != '\0'; i++)
    {
        head[i] = val[i];
    }
    // memset(&val, '\0', sizeof(val));
    l = atoi(head);
    if (l == 0)
        return ZEROVAL;
    else
        return l;
}

int controlcommand(char val[], int l)   /* Correttezza dei messaggi ricevuti #7 */
{
    int k;
    for (int i = 0; val[i] != '\0'; i++)
    {
        if (val[i] != ' ')
        {
            k++;
            while (val[i] == ' ')
                i++;
        }
        if (isgraph(val[i]) != 0)
            return 0;
    }
    if (l == k)
        return l;
    else
        return 0;
}

int sumVal(char val[])
{
    int sum = 0;
    char s[strlen(val)];

    int l;
    int k = 0;
    // memset(&val, '\0', sizeof(val));
    for (int i = 0; val[i] != ' ' && val[i] != '\0'; i++) {}
    for (int i = 0; val[i] == ' '; i++) {}
    for (int i = 0; val[i] != '\0'; i++, k++)
    {
        s[k] = val[i];
        if(val[i] != ' ')
        {
            l = atoi(s);
            sum += l;
            for (int j = 0; val[i] == ' ' ; i++) {}
        }
    }
    // memset(&val, '\0', sizeof(val));
    return sum;
}

int client_waiting(int simpleChildSocket)
{
    char buffer[MAX_CHAR];
    int returnStatus = 0;
    // inizializzo una lista
    int count = 0;
    int sum = 0;

    while (1)
    {
        memset(&buffer, '\0', sizeof(buffer));
        returnStatus = read(simpleChildSocket, buffer, sizeof(buffer)); // Ricevo il messaggio del client
        if (returnStatus > 0)   // Nel caso abbia ricevuto qualcosa svolgo le operazioni successive
        {
            fprintf(stderr, "%s", buffer);
            returnStatus = controlcommandsyntax(buffer);
            switch (returnStatus)
            {
                case 0:
                    error_checking(returnStatus, SYNTAX, simpleChildSocket);
                    break;
                case ZEROVAL:
                    // TODO: 8.c
                default:
                    if (controlcommand(buffer, returnStatus))
                        count += returnStatus;
                        sum += sumVal(buffer);
                        // TODO: risponde con il messaggio `OK DATA <numero_dati_letti>` 
                        // ovvero la stringa `OK DATA` seguita da uno spazio e da una stringa numerica 
                        // che rappresenta il valore numero di dati estratti dal messaggio ricevuto.
                    error_checking(returnStatus, DATA, simpleChildSocket);
                    break;
            }
        }
    }
} 