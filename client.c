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
            printf("ERROR the server did not welcome me :(\n");
        }
        else
        {
            if (print_messages(buffer))
            {
                printf("ERROR syntax, the server message must have:\n - this form:\n   <outcode> <type> <content>\n - a maximum of 512 characters\n - a end with newline\n");
            }
            else
            {
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
            printf("%c", buffer[i]);
            i++;
        }
        if (buffer[i] != '\n')
        {
            return 1;
        }
        printf("\n");
        i++;
    }
    return 0;
}