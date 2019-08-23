#include <stdio.h>      
#include <sys/types.h>
#include <sys/socket.h>   
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MAX_CHAR 512    /* lunghezza massima */

int print_messages(char returnStatus[]);    /* stampa il <MESSAGGIO> del messaggio ricevuto. Se il messaggio non ha errori sintattici ritorna 1 altrimenti 0 #11 #12 */
char start[] = "OK START "; /* messaggio di benvento */
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
        printf(stderr, "Usage: %s <server> <port>\n", argv[0]);
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

void text(int simpleSocket)  /* Inserimento del testo #14  */
{
    char buffer[MAX_CHAR] = "TEXT ";
    int ch, i = 5;
    int cifre = 5+4+1;  // 5 per text, 4 per i numei e 1 per \n
    int caratteri = 0;
    char char_caratteri[4];
    while(i < MAX_CHAR-cifre)
    {
        if ((ch = getchar()) != '\n' && ch != EOF)
        {
            buffer[i] = ch;
            i++;
            if (ch != ' ') {caratteri++;}
        }
        else
        {break;}
    }
    buffer[i] = '\0';
    if(buffer[0] != '\0' && buffer[0] != '\n')
    {
        strcat(buffer, " ");
        sprintf(char_caratteri, "%d", caratteri);
        strcat(buffer, char_caratteri);
        strcat(buffer, "\n");
        write(simpleSocket, buffer, strlen(buffer)); 
        memset(&buffer, '\0', sizeof(buffer));
        if(i >= MAX_CHAR-cifre)
        {text(simpleSocket);}
    }
}

int execute_command(int simpleSocket)
{
    char buffer[MAX_CHAR];
    int ch, returnStatus;
    for (int i = 0; i < 5; i++)
    {
        if ((ch = getchar()) != '\n' && ch != EOF)
        {buffer[i] = ch;}
        else
        {break;}
    }
    
    if (strncmp(buffer, "TEXT ", 5) == 0)
    {text(simpleSocket);}
    else if (strncmp(buffer, "HIST ", 5) == 0)
    {}
    else if (strncmp(buffer, "EXIT ", 5) == 0)
    {}
    else if (strncmp(buffer, "QUIT ", 5) == 0)
    {}
    else
    {printf("Riprova\n"); return 1;}
    memset(&buffer, '\0', sizeof(buffer));



    while (1)
    {
        returnStatus = read(simpleSocket, buffer, sizeof(buffer));
        if (returnStatus > 0)
        {
            if(strncmp(buffer, "OK ", 3) == 0)
            {break;}
            else
            {print_messages(buffer);return 0;}
        }
    }
    memset(&buffer, '\0', sizeof(buffer));




    return 1;
}