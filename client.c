#include <stdio.h>      
#include <sys/types.h>
#include <sys/socket.h>   
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <ctype.h>

#define MAX_CHAR 512    /* lunghezza massima */

int print_messages(char returnStatus[]);    /* stampa il <MESSAGGIO> del messaggio ricevuto. Se il messaggio non ha errori sintattici ritorna 1 altrimenti 0 #11 #12 */
char start[] = "OK START "; /* messaggio di benvento */
// Le opzioni che il client offre (client) #13
char illustration[] = "Enter the numbers whose average and variance must be calculated, according to the criterion you prefer\n";
int execute_command(int simpleSocket);  /* Esegue i comandi #14 */
char command_line[] = "\nCommand > "; /* Command > */

int main(int argc, char *argv[])
{
    int simpleSocket = 0;
    int simplePort = 0;
    int returnStatus = 0;
    char buffer[256] = "";
    struct sockaddr_in simpleServer;

    if (3 != argc)
    {
        fprintf(stderr, "Usage: %s <server> <port>\n", argv[0]);
        exit(1);
    }

    /* create a streaming socket      */
    simpleSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (simpleSocket == -1)
    {
        fprintf(stderr, "Could not create a socket!\n");
        exit(1);
    }
    else
    {
	    fprintf(stderr, "Socket created!\n");
    }

    /* retrieve the port number for connecting */
    simplePort = atoi(argv[2]);

    /* setup the address structure */
    /* use the IP address sent as an argument for the server address  */
    //bzero(&simpleServer, sizeof(simpleServer)); 
    memset(&simpleServer, '\0', sizeof(simpleServer));
    simpleServer.sin_family = AF_INET;
    //inet_addr(argv[2], &simpleServer.sin_addr.s_addr);
    simpleServer.sin_addr.s_addr=inet_addr(argv[1]);
    simpleServer.sin_port = htons(simplePort);

    /*  connect to the address and port with our socket  */
    returnStatus = connect(simpleSocket, (struct sockaddr *)&simpleServer, sizeof(simpleServer));

    if (returnStatus == 0)
    {
	    fprintf(stderr, "Connect successful!\n");
    }
    else
    {
        fprintf(stderr, "Could not connect to address!\n");
        close(simpleSocket);
        exit(1);
    }

    /* get the message from the server   */
    returnStatus = read(simpleSocket, buffer, sizeof(buffer));

    if ( returnStatus > 0 )
    {
        //printf("%d: %s", returnStatus, buffer);
        // controlla se il primo messaggio e' OK START #12
        if(strncmp(start, buffer, strlen(start)) != 0)
        {
            fprintf(stderr, "ERROR the server did not welcome me :(\n");
        }
        else
        {
            if (print_messages(buffer))
            {
                fprintf(stderr, "ERROR syntax, the server message must have:\n - this form:\n   <outcode> <type> <content>\n - a maximum of 512 characters\n - a end with newline\n");
            }
            else
            {
                // Le opzioni che il client offre (client) #13
                printf("%s", illustration);
                // Il client sollecita l’utente ad inserire il testo #14
                printf("%s", command_line);
                while (execute_command(simpleSocket))
                {
                    printf("%s", command_line);
                }
            }
        }
    }
    else
    {
        fprintf(stderr, "Return Status = %d \n", returnStatus);
    }

    close(simpleSocket);
    return 0;
}

int print_messages(char buffer[])
{
    int i = 0;
    while (buffer[i] != '\0')
    {
        // outcode
        if(buffer[i] == 'O' && buffer[i+1] == 'K' && buffer[i+2] == ' ')
        {
            i += 2;
        }
        else if(buffer[i] == 'E' && buffer[i+1] == 'R' && buffer[i+2] == 'R' && buffer[i+3] == ' ')
        {
            i += 3;
        }
        else
        {
            return 1;
        }
        while (buffer[i] == ' ')    // salta gli spazzi
        {
            i++;
        }
        // type
        while (buffer[i] != ' ' && buffer[i] != '\0')
        {
            i++;
        }
        while (buffer[i] == ' ')    // salta gli spazzi
        {
            i++;
        }
        if (buffer[i] == '\0' || buffer[i] == '\n')
        {
            return 1;
        }
        // content
        while (buffer[i] != '\0' && buffer[i] != '\n')
        {
            if (i >= MAX_CHAR)
            {
                return 1;
            }
            fprintf(stderr, "%c", buffer[i]);
            i++;
        }
        if (buffer[i] != '\n')
        {
            return 1;
        }
        fprintf(stderr, "\n");
        i++;
    }
    return 0;
}

int error_checking(int simpleSocket)    /* aspetta un messaggio del servre e controlla se questo è di errore (return 1 se ha un errore) */
{
    int i;
    char buffer[MAX_CHAR];
    while (1)
    {
        i = read(simpleSocket, buffer, sizeof(buffer));
        if (i > 0)
        {
            if(strncmp(buffer, "OK ", 3) == 0)
            {return 0;}
            else
            {print_messages(buffer);return 1;}
        }
    }
}

int text(int simpleSocket)  /* Inserimento del testo #14  (return 1 se ha un errore)*/
{
    int ch, i = 0;
    int cifre = 1;
    int caratteri = 0;
    char char_caratteri[4];
    char val[MAX_CHAR];
    // conta numeri
    while(i < MAX_CHAR-cifre)
    {
        if (isgraph(val[i]) != 0)
            return 0;
        if (val[i] != ' ')
        {
            caratteri++;
            while (val[i] == ' '; i++)
                i++;
        }
        else if ((ch = getchar()) == '\n' || ch == EOF)
        {break;}
    }
    val[i] = '\0';
    // invia messaggio
    if(caratteri != 0)
    {
        sprintf(char_caratteri, "%d", caratteri);
        strcat(buffer, char_caratteri);
        strcat(buffer, " ");
        strcat(buffer, val);
        strcat(buffer, "\n");
        write(simpleSocket, buffer, strlen(buffer)); 
        memset(&buffer, '\0', sizeof(buffer));
        if(i >= MAX_CHAR-cifre) // se il messaggio è troppo grande
        {if(text(simpleSocket)) {return 1;}}
    }
    else
    {
        strcat(buffer, "0");
        strcat(buffer, "\n");
        write(simpleSocket, buffer, strlen(buffer)); 
        memset(&buffer, '\0', sizeof(buffer));
        if(i >= MAX_CHAR-cifre) // se il messaggio è troppo grande
        {if(text(simpleSocket)) {return 1;}}
    }
    // controlla risposta del server
    if(!error_checking(simpleSocket))
    {return error_checking(simpleSocket);}
    else
    {return 1;}
}

int execute_command(int simpleSocket)
{
    char buffer[MAX_CHAR];
    memset(&buffer, '\0', sizeof(buffer));
    int ch, returnStatus;
    
    if (strncmp(buffer, "/n", 5) == 0)
        printf("Riprova\n"); 
    else
        if(text(simpleSocket))
            return 0;
    
    for (int i = 0;(ch = getchar()) != '\n' && ch != EOF ; i++){}
    return 1;}

    memset(&buffer, '\0', sizeof(buffer));

    return 1;
}