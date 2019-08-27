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

#define TEXT 5466
#define HIST 4664
#define EXIT 4576
#define QUIT 4575

const char MESSAGE[] = "Hello UPO student!\n";
// lista
struct node
{
    char alphanumber;
    int counter;
    struct node *next;
};
typedef struct node element;
typedef element *list;
list newnode() {return malloc(sizeof(element));}
list head;
void deleteList();  /* Elimina l'intera lista */

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

int controlcommand(char buffer[])   /* Correttezza dei messaggi ricevuti #7 */
{
    if (strncmp(buffer, "TEXT ", 5) == 0)
    {return TEXT;}
    else if (strncmp(buffer, "HIST", 4) == 0)
    {return HIST;}
    else if (strncmp(buffer, "EXIT", 4) == 0)
    {return EXIT;}
    else if (strncmp(buffer, "QUIT", 4) == 0)
    {return QUIT;}
    // In caso il comando non fosse riconosciuto, ritorna 0
    return 0;
}

int controltext(int simpleChildSocket, char buffer[])   /* contrlla il comando TEXT #8 (ritorna 1 se è sbagliato)*/
{
    int count=0, count2 = 0, figures = -1;
    char n[10], m[10];
    // copia le cifre al contrario
    for (int i = strlen(buffer)-1; buffer[i] != ' ' && buffer[i] != '\0'; i--, figures++)
    {
        n[figures] = buffer[i];
    }
    // riordina le cife
    for (int i = 0, k = figures; k != -1; i++, k--)
    {
        m[i] = n[k-1];
    }
    m[figures] = '\0';
    count = atoi(m);
    // conta i caratteri
    for (int i = 5; i < (MAX_CHAR) && buffer[i] != '\n' && buffer[i] != '\0'; i++)
    {
        if (isalnum(buffer[i])) {count2++;}
    }
    count2 = count2 - figures;

    // serve per HIST #5
    list current = head;
    for (int i = 5, k = count2; i < (MAX_CHAR) && buffer[i] != '\n' && buffer[i] != '\0' && k != 0; i++, k--)
    {
        if (isalnum(buffer[i]))
        {
            current = head;
            int y = 1;
            while(y)
            {
                if (current == NULL)
                {
                    printf("PRIMO NULLO\n");
                    current = newnode();
                    current->next = NULL;
                    current->alphanumber = buffer[i];
                    current->counter = 1;
                    y = 0;
                }
                else if (current->alphanumber == buffer[i])
                {
                    current->counter++; y = 0;
                }
                else if (!isalnum(current->alphanumber) || current->counter == 0)
                {
                    current->alphanumber = buffer[i];
                    current->counter = 1;
                    y = 0;
                }
                else if (current->next == NULL)
                {
                    printf("NEXT NULLO1\n");
                    current->next = newnode();  // Si blocca qui, quando il primo client crea x nodi e il secondo crea y(>x) nodi
                    printf("NEXT NULLO2\n");
                    current->next->next = NULL;
                    current = current->next;
                    current->alphanumber = buffer[i];
                    current->counter = 1;
                    y = 0;
                }
                else
                {
                    current = current->next;
                }
            }
        }
    }

    sleep(1);
    // invia un messagio al client
    if(count2 == count)
    {
        strcpy(buffer, "OK TEXT ");
        sprintf(n, "%d", count2);
        strcat(buffer, n);
        strcat(buffer, "\n");
        write(simpleChildSocket, buffer, strlen(buffer));
        fprintf(stderr, "%s", buffer); 
        return 0;
    }
    else
    {
        strcpy(buffer, "ERR TEXT 'The characters counted by the client(");
        sprintf(n, "%d", count);
        strcat(buffer, n);
        strcat(buffer, ") are different from the characters of the server(");
        sprintf(n, "%d", count2);
        strcat(buffer, n);
        strcat(buffer, ")'\n");
        write(simpleChildSocket, buffer, strlen(buffer));
        fprintf(stderr, "%s", buffer); 
        return 1;
    }
}

void hist(int simpleChildSocket) /* il comando HIST #8 */
{
	char c[5], buffer[512];
    list current = head;
	if (current != NULL)
	{
        current = head;
        while(1)
        {
            sleep(1);
            memset(&buffer, '\0', sizeof(buffer));
            strcat(buffer, "OK HIST ");
            for (int i = 0; i < 7 && current != NULL; i++)
            {
                if (isalnum(current->alphanumber))
                {
                    memset(&c, '\0', sizeof(c));
                    c[0] = current->alphanumber;
                    strcat(buffer, c);
                    strcat(buffer, ":");
                    sprintf(c, "%d", current->counter);
                    strcat(buffer, c);
                    strcat(buffer, " ");
                }
                else 
                {i--;}
                if (current->next != NULL)
                {current=current->next;}
                else
                {break;}
            }
            strcat(buffer, "\n");
            write(simpleChildSocket, buffer, strlen(buffer));
            fprintf(stderr, "%s", buffer);
            if (current->next == NULL)
            {break;}
        }
	}
    memset(&buffer, '\0', sizeof(buffer));
	strcat(buffer, "OK HIST END\n");
    write(simpleChildSocket, buffer, strlen(buffer));
    fprintf(stderr, "%s", buffer);
}

int client_waiting(int simpleChildSocket)
{
    char buffer[MAX_CHAR];
    int returnStatus = 0;
    // inizializzo una lista
    head = newnode();
    head->counter = 0;
    list *l = &head;

    while (1)
    {
        memset(&buffer, '\0', sizeof(buffer));
        returnStatus = read(simpleChildSocket, buffer, sizeof(buffer)); // Ricevo il messaggio del client
        if (returnStatus > 0)   // Nel caso abbia ricevuto qualcosa svolgo le operazioni successive
        {
            fprintf(stderr, "%s", buffer);
            returnStatus = controlcommand(buffer);
            error_checking(returnStatus, SYNTAX, simpleChildSocket);
            switch (returnStatus)
            {
                case TEXT:  // se ha un errore nel TEXT li server termina
                    if (controltext(simpleChildSocket, buffer))
                    {deleteList();return 0;}
                    break;
                case HIST:
                    hist(simpleChildSocket);
                    break;
                case EXIT:  // il server chiuse la connesione con il client e ne aspetta un altro
                    hist(simpleChildSocket);
                    error_checking(1, EXIT, simpleChildSocket);
                    deleteList();return 1;
                    break;
                case QUIT:  // il server chiuse la connesione con il client e ne aspetta un altro
                    error_checking(1, QUIT, simpleChildSocket);
                    deleteList();return 1;
                    break;
                case 0: // se ha un errore di sintassi li server termina
                    deleteList();return 0;
                    break;
            }
        }
    }
} 

void deleteList()
{ 
    list current = head; 
    list next; 
    while (current != NULL)  
    {
        current->counter = 0;
        current = current->next;
//        next = current->next; 
//        free(current); 
//        current = next; 
    } 
//    head = NULL; 
}