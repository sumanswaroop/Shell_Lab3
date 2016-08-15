/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <cstdlib>
#include <stdio.h>
#include <signal.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include <fstream>

#define BACK_LOG 50

using namespace std;

void error(string msg)
{
	perror(msg.c_str());
	exit(1);
}

//signal handler (signal send by child when it exits)
void sig_handler(int signo)
{
	if(signo == SIGCHLD)
	{
		int status,pid;
		//cout<<"Received Signal\n";
		//reap all zombie child with nonblocking wait pid
		while( (pid=waitpid(-1,&status,WNOHANG))>0 )
			{//cout<<"Pid of process reaped : "<<pid<<endl;
	}
	}
}

int main(int argc, char *argv[])
{	
	 //socket related vars
	 int sockfd, newsockfd, portno;
	 socklen_t clilen;
	 char buffer[256];
	 struct sockaddr_in serv_addr, cli_addr;
	 int n;
	 
	 if (argc < 2) {
		 fprintf(stderr,"ERROR, no port provided\n");
		 exit(1);
	 }

	 /* create socket */

	 sockfd = socket(AF_INET, SOCK_STREAM, 0);
	 if (sockfd < 0) 
		error("ERROR opening socket");

	 /* fill in port number to listen on. IP address can be anything (INADDR_ANY) */

	 bzero( (char *) &serv_addr, sizeof(serv_addr));
	 portno = atoi(argv[1]);
	 serv_addr.sin_family = AF_INET;
	 serv_addr.sin_addr.s_addr = INADDR_ANY;
	 serv_addr.sin_port = htons(portno);

	 /* bind socket to this port number on this machine */

	 if (bind(sockfd, (struct sockaddr *) &serv_addr,
			  sizeof(serv_addr)) < 0) 
			  error("ERROR on binding");

	 //Registering the signal handler in kernel
	 if (signal(SIGCHLD, sig_handler) == SIG_ERR)
        fprintf(stderr,"Can't catch SIGCHLD signal");
	 
	 while(true)
	 {
		 /* listen for incoming connection requests */

		  listen(sockfd, BACK_LOG);
		  clilen = sizeof(cli_addr);
		  /* accept a new request, create a newsockfd */
		  if ( (newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr,&clilen)) < 0) 
				error("ERROR on accept \n");
		  
		  int ret = fork();
		  if(ret==0)
		  {
			
			/* read message from client */
			 bzero(buffer,256);
			 if ( (n = read(newsockfd,buffer,255))<0) 
			 {	
			 	//close on failure
			 	close(newsockfd);
			 	error("ERROR reading from socket");
			 }
			 
			 //printf("%s requested by client IP %s \n",buffer,inet_ntoa(cli_addr.sin_addr));
			 
			 //parsing request
			 int i;
			 for(i=0;i<255;i++) if(buffer[i]==' ') break;
			 i++;
			 string filename = "";
			 for(;i<255;i++) {if(buffer[i]=='.') break; filename+=buffer[i];}
			 filename+=".txt";

			 //Reading the requested file
			 ifstream file (filename.c_str(), ios::in);
			 if(file.fail()) 
		 	 {
		 		close(newsockfd);
		 		error("File not found");
		 	 }

			 //buffer for file content
			 int length = 1024;
			 char buffer2[length];

			 /* send reply to client */ //the requested file is being written in socket
			 while(!file.eof())
			 {
			 	file.read(buffer2,length-1);
			 	n = write(newsockfd,buffer2,strlen(buffer2));
				if (n < 0) {
					close(newsockfd);
					error("ERROR writing to socket");
				}
				sleep(1);
			 }

			 //printf("File Transfer Completed to Client IP %s\n",inet_ntoa(cli_addr.sin_addr));
			 //cleanup
			 file.close();

			 //Close Socket
			 close(newsockfd);
			 return 0;

		  }
		  else
		  {	
		  	close(newsockfd);
		  }
	  }
	 return 0; 
}
