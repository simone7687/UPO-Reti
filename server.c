#include <stdio.h>      
#include <sys/types.h>
#include <sys/socket.h>   
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> 

#define MAX_CHAR 512    /* lunghezza massima */

// I tipi di messaggi
#define ARGO 45645  /* argomenti */
#define SOCR 76567  /* creazione socket */
#define BIND 43654  /* bind address */
#define SOLS 63473  /* socket connesso */
#define CONT 47645  /* connesione accettata */
#define SYNT 75648  /* sintassi */

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
void deleteList(list* head_ref); /* Function to delete the entire linked list */

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
    while (x)
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
            case SYNT:
                strcat(buffer, "SYNT 'Correct syntax!'");
                break;
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
            case SYNT:
                strcat(buffer, "SYNT 'Incorrect syntax!'");
                break;
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
    else if (strncmp(buffer, "EXIT ", 5) == 0)
    {return EXIT;}
    else if (strncmp(buffer, "QUIT ", 5) == 0)
    {return QUIT;}
    // In caso il comando non fosse riconosciuto, ritorna 0
    return 0;
}

int controltext(int simpleChildSocket, char buffer[], element* l)   /* contrlla il comando TEXT #8 (ritorna 1 se è sbagliato)*/
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
        if (buffer[i] != ' ') {count2++;}
    }
    count2 = count2 - figures;

    // serve per HIST #5
    list head = l;
    for (int i = 5, k = count2; i < (MAX_CHAR) && buffer[i] != '\n' && buffer[i] != '\0' && k != 0; i++, k--)
    {
        if (buffer[i] != ' ')
        {
            l = head;
            int y = 1;
            while(y)
            {
                if (l->alphanumber == buffer[i])
                {
                    l->counter++;
                    y=0;
                }
                else if (l->alphanumber == '\0')
                {
                    l->alphanumber = buffer[i];
                    l->counter = 1;
                    y=0;
                }
                else if (l->next == NULL)
                {
                    l->next = newnode();
                    l->next->next = NULL;
                    l = l->next;
                    l->alphanumber = buffer[i];
                    l->counter = 1;
                    y=0;
                }
                else
                {
                    l=l->next;
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

int hist(element* l, int simpleChildSocket) /* il comando HIST #8 (ritorna 1 se è sbagliato)*/
{
	char c[5], buffer[512];
    list head = l;
	if (l != NULL)
	{
        l = head;
        while(1)
        {
            sleep(1);
            memset(&buffer, '\0', sizeof(buffer));
            strcat(buffer, "OK HIST ");
            for (int i = 0; i < 7; i++)
            {
                memset(&c, '\0', sizeof(c));
                c[0] = l->alphanumber;
                strcat(buffer, c);
                strcat(buffer, ":");
                sprintf(c, "%d", l->counter);
                strcat(buffer, c);
                strcat(buffer, " ");
                if (l->next != NULL)
                {l=l->next;}
                else
                {break;}
            }
            strcat(buffer, "\n");
            write(simpleChildSocket, buffer, strlen(buffer));
            fprintf(stderr, "%s", buffer);
            if (l->next == NULL)
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
    list head = newnode();
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
            error_checking(returnStatus, SYNT, simpleChildSocket);
            switch (returnStatus)
            {
                case TEXT:
                    if (controltext(simpleChildSocket, buffer, *l)) {deleteList(l);return 0;}
                    break;
                case HIST:
                    hist(*l, simpleChildSocket);
                    break;
                case EXIT:
                    break;
                case QUIT:
                    break;
                case 0:
                    deleteList(l);return 0;
                    break;
            }
        }
    }
} 

void deleteList(list* head_ref) /* Elimina l'intera lista */
{ 
   list current = *head_ref; 
   list next; 
   while (current != NULL)  
   { 
       next = current->next; 
       free(current); 
       current = next; 
   } 
   *head_ref = NULL; 
}