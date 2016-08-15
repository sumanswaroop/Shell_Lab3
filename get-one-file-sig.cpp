#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

void my_handler(int s){
           printf("Caught signal %d\n",s);
           exit(1); 

}

void error(char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
    int sockfd, portno, n;
    bool display;
    if(argv[4]=="display") display=1;
    else display=0;

    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[256];
    if (argc != 5) {
       fprintf(stderr,"usage %s filename hostname port display\n", argv[0]);
       exit(0);
    }

    struct sigaction sigIntHandler;

   sigIntHandler.sa_handler = my_handler;
   sigemptyset(&sigIntHandler.sa_mask);
   sigIntHandler.sa_flags = 0;

   sigaction(SIGINT, &sigIntHandler, NULL);

   pause();

    /* create socket, get sockfd handle */

    portno = atoi(argv[3]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");

    /* fill in server address in sockaddr_in datastructure */

    server = gethostbyname(argv[2]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);

    /* connect to server */

    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");

    /* ask user for input */

    bzero(buffer,256);
    sprintf(buffer, "get %s",argv[1]);

    /* send user message to server */

    n = write(sockfd,buffer,strlen(buffer));
    if (n < 0) 
         error("ERROR writing to socket");
    bzero(buffer,256);

    /* read reply from server */

    int received_size=0,curr_size;
    //Receive File
    while((curr_size=read(sock_fd, buffer, 1023)>0)){
        if(display) {cout<<buffer;}
      received_size+=curr_size;
    }
    if(received_size==0)
        fprintf(stderr, "Requested File could not be served by server\n");

    return 0;
}
