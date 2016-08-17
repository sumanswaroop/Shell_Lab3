#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <string.h>
#include <string>
#include <map>
#include <signal.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/signal.h>
#include <string.h>
#include <string>
#include <fcntl.h>
#include <iostream>
#include <set>
#include <vector>
#include "make-tokens.h"

using namespace std;
//FUnction Pointer
typedef int (*FnPtr)(char **);


//colors
#define RESET   "\033[0m"
#define BLACK   "\033[30m"      /* Black */
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */
#define YELLOW  "\033[33m"      /* Yellow */
#define BLUE    "\033[34m"      /* Blue */
#define MAGENTA "\033[35m"      /* Magenta */
#define CYAN    "\033[36m"      /* Cyan */
#define WHITE   "\033[37m"      /* White */
#define BOLDBLACK   "\033[1m\033[30m"      /* Bold Black */
#define BOLDRED     "\033[1m\033[31m"      /* Bold Red */
#define BOLDGREEN   "\033[1m\033[32m"      /* Bold Green */
#define BOLDYELLOW  "\033[1m\033[33m"      /* Bold Yellow */
#define BOLDBLUE    "\033[1m\033[34m"      /* Bold Blue */
#define BOLDMAGENTA "\033[1m\033[35m"      /* Bold Magenta */
#define BOLDCYAN    "\033[1m\033[36m"      /* Bold Cyan */
#define BOLDWHITE   "\033[1m\033[37m"      /* Bold White */
//type
#define EXEC 1
#define PIPE 2
#define REDIR 3
#define BACK 4
//server details
char server_ip[16];
char server_port[5];
int status;

//background process list
set<int> bpid;

//define all builtins
int cd_builtin(char **args)
{
	if(args[1]==NULL)
	{
		fprintf( stderr,"Insufficient Arguments Provided : 1 expected , 0 Provided\n");
		return -1;
	}
	else if(args[2] == NULL)
	{	
		if(chdir(args[1])!=0)
			fprintf( stderr,"No Such Directory exists\n");
		return -1;
	}
	else fprintf( stderr,"Extra Arguments Provided\n");

	return 0;
}
//function to take setup server ip and port (in buit func)
int server_setup(char **args)
{	
	//if insufficient arguments
	if(args[1]==NULL || args[2] == NULL)
	{	fprintf(stderr, "Invalid Arguments");
		return -1;
	}
	//copy args provided to server ip and port resp
	strcpy(server_ip,args[1]);
	strcpy(server_port,args[2]);
	return 0;
}
//handle exit in built
int exit(char **args)
{		
	//kill all background process
	set<int>::iterator it;
	vector<int> temp;
	//kill all background process
	for(it = bpid.begin();it!=bpid.end();it++)
	{
		int a = *it;
		temp.push_back(a);
		kill(a, SIGINT);
		
	}
	//erase there pid from the background process list
	for(int i = 0;i<temp.size();i++)bpid.erase(temp[i]);
	//kill all children process in same group
	kill(0, SIGTERM);
	exit(0);
}
//map builtins
void map_builtin(map<string , FnPtr > &builtins)
{
	builtins["cd"] = cd_builtin;
	builtins["server"] = server_setup;
	builtins["exit"] = exit;
}

//commands struct
//general cmd
struct cmd{
	int type;
};
//executable cmd
struct execcmd{
	int type;
	char **argv = new char*[64];
};
//pipe command
struct pipecmd
{
	int type;
	struct execcmd *left;
	struct execcmd *right;
};
//redirect command
struct redircmd
{
	int type;
	char *file;
	char **argv = new char*[64];
	
};



void run(void *cmd)
{	
	//pipe
	int p[2];
	int pid;
	if(cmd ==NULL)
		exit(0);
	//the three command type pointer
	struct execcmd *ecmd;
	struct pipecmd *pcmd;
	struct redircmd *rcmd;
	//check type of command
	switch(((struct cmd *)cmd)->type)
	{
		case EXEC:

			ecmd = (struct execcmd *)cmd;
			//check if no command
			if(ecmd-> argv[0] == NULL)
				fprintf(stderr, "Exec Error\n");
			//parallel
			else if(strcmp(ecmd->argv[0],"getpl")==0)
			{	
				int num = 1;
				vector<int> pidlist;
				//maintain a list of pids to wait for after running parallely 
				while(ecmd->argv[num]!=NULL)
				{	
					//fork a child
					pid=fork();

					if(pid ==0)
					{	//execute 
						execl("client", "client", ecmd->argv[num], server_ip, server_port, "nodisplay", (char *)0);
						fprintf(stderr,"Error On Executing the executable\n");
						exit(1);	
					}
					num++;
					//in parent push the pid
					pidlist.push_back(pid);
				}
				//wait for all pids pushed
				for(int i = 0;i<num;i++)waitpid(pidlist[i] ,&status, 0);
				pidlist.clear();
			}
			//sequential or one
			else if(strcmp(ecmd->argv[0],"getfl")==0 || strcmp(ecmd->argv[0],"getsq")==0 )
			{	
				pid = fork();
				if(pid ==0)
				{	//execute
					execv("client", ecmd->argv);
					fprintf(stderr,"Error on execution of Command\n");
					exit(1);
				}
				//wait for the pid
				else waitpid(pid, &status, 0);
			}
			//background
			else if(strcmp(ecmd->argv[0],"getbg")==0)
			{	
				pid = fork();
				if(pid==0)
				{	
					strcpy(ecmd->argv[4], "nodisplay");
					//set the group id
					setpgrp();
					//execute
					execv("client",ecmd->argv);
					exit(1);
				}//no wait .. append the pid to backgrnd list
				else if(pid>0){
					bpid.insert(pid);
				}
			}
			//everything else // linux commands
			else if((pid = fork())==0)
			{
					execvp(ecmd->argv[0], ecmd->argv);
					fprintf(stderr,"Unknown Command\n");
					exit(1);
			}
			else waitpid(pid, &status, 0);
			break;

		case PIPE:
		//type cast passed cmd argument to pipecmd type
			pcmd = (struct pipecmd *)cmd;
			if( pipe(p)<0)
			{
					fprintf(stderr, "Pipe Initiation Error");
					return;
			}
			//writer process
			int pid2;
			if((pid =fork())==0)
			{	
				close(1);	//close output
				dup(p[1]);	//dup stdout to p[1]
				close(p[0]);	//close both pipes
				close(p[1]);
				//execute left
				execv("client", pcmd->left->argv);
				
			}
			//reader process
			if((pid2 = fork())==0)
			{	
				close(0);	//close stdin
				dup(p[0]);	//make p[0] to stdin
				close(p[0]);	//close pipe
				close(p[1]);
				//execute right
				execvp(pcmd->right->argv[0], pcmd->right->argv);
				
			}
			close(p[0]);
			close(p[1]);
			//wait for both process
			waitpid(pid, &status, 0);
			waitpid(pid2, &status, 0);
			break;

		case REDIR:
			rcmd = (struct redircmd *)cmd;
			
			if((pid = fork())==0)
			{
				//open file
				int fd = open(rcmd->file, O_RDWR | O_CREAT , S_IRUSR | S_IWUSR);
				//close stdout and redirect 1 to fd
				dup2(fd, 1);
				close(fd);
				//execute
				execv("client", rcmd->argv);
				exit(1);
			}
			waitpid(pid, &status,0);
			break;

	}
}
//

//Command Parsing
//check for symbol
int type(char **start)
{	
	char **sep = start;
	int i = 0;
	while(sep[i]!=NULL)
	{
		if(strcmp(sep[i],">" )==0)
			return REDIR;
		else if(strcmp(sep[i], "|")==0)
			return PIPE;
		i++;
	}
	return EXEC;
}

struct cmd * parse(char **tokens)
{	
	//seperator
	//token like piping rediricting
	int tok = type(tokens);
	struct execcmd cmd;
	struct pipecmd pcmd;
	struct redircmd rcmd;

	switch(tok)
	{
		case EXEC:
			cmd.type = EXEC;
			//single or backgrnd
			if(strcmp(tokens[0],"getfl")==0 || strcmp(tokens[0],"getbg")==0)
			{
				int i = 0;
				//set up arguments
				while(tokens[i]!=NULL)
				{	
					cmd.argv[i] = new char[MAX_TOKEN_SIZE];
					strcpy(cmd.argv[i], tokens[i]);
					i++;
				}
				if(i!=2)
				{
					fprintf(stderr, "Invalid Number of Arguments\n");
					break;
				}

				cmd.argv[i] = new char[MAX_TOKEN_SIZE];
				strcpy(cmd.argv[i],server_ip);
				cmd.argv[i+1] = new char[MAX_TOKEN_SIZE];
				strcpy(cmd.argv[i+1], server_port);
				cmd.argv[i+2] = new char[MAX_TOKEN_SIZE];
				strcpy(cmd.argv[i+2], "display");
				cmd.argv[i+3] = NULL;
				run((void *)&cmd);
				
			}
			//sequential
			//command arguments setup
			else if(strcmp(tokens[0],"getsq")==0)
			{	
				int i =1;
				cmd.argv[0] = new char[MAX_INPUT_SIZE];
				strcpy(cmd.argv[0],tokens[0]);
				cmd.argv[1] = new char[MAX_INPUT_SIZE];
				cmd.argv[2] = new char[MAX_INPUT_SIZE];
				cmd.argv[3] = new char[MAX_INPUT_SIZE];
				cmd.argv[4] = new char[MAX_TOKEN_SIZE];

				strcpy(cmd.argv[2], server_ip);
				strcpy(cmd.argv[3], server_port);
				strcpy(cmd.argv[4],"nodisplay");

				while(tokens[i]!=NULL)
				{
					strcpy(cmd.argv[1] ,tokens[i]);
					run((void *)&cmd);
					i++;
				}
				if(i==1)
				{
					fprintf(stderr, "Invalid Number of Arguments\n");
					break;
				}

			}
			//parallel 
			//command arguments setup
			else if(strcmp(tokens[0],"getpl")==0)
			{
				int i = 0;
				//set up arguments
				while(tokens[i]!=NULL)
				{	
					cmd.argv[i] = new char[MAX_TOKEN_SIZE];
					strcpy(cmd.argv[i], tokens[i]);
					i++;
				}
				if(i<2)
				{
					fprintf(stderr,"Invalid Number of Arguments\n");
				}
				cmd.argv[i]=NULL;
				run((void *)&cmd);
			}
			else{ 
				int i = 0;
				while(tokens[i]!=NULL)
					{	
						cmd.argv[i] = new char[MAX_INPUT_SIZE];
						strcpy(cmd.argv[i] , tokens[i]);
						i++;
					}
				cmd.argv[i] = NULL;
				run((void *)&cmd);
			}
			break;

		case PIPE:
			pcmd.type = PIPE;
			//PIPE in getfl
			//command arguments setup

			if(strcmp(tokens[0], "getfl")==0)
			{	
				//save left part in cmd->left
				pcmd.left = new struct execcmd;
				pcmd.left->type = EXEC;
				int i = 0;
				while(strcmp(tokens[i],"|")!=0)
				{
					pcmd.left->argv[i]= new char[MAX_TOKEN_SIZE];
					strcpy(pcmd.left->argv[i], tokens[i]);
					i++;
				}
				//check if more than one file
				if(i>2)
				{
					fprintf(stderr, "Invalid Number of files given\n");
					break;
				}
				int temp = i;
				//copy ip , port and display args
				pcmd.left->argv[i] = new char[MAX_TOKEN_SIZE];
				pcmd.left->argv[i+1] = new char[MAX_TOKEN_SIZE];
				pcmd.left->argv[i+2] = new char[MAX_TOKEN_SIZE];
				
				strcpy(pcmd.left->argv[i], server_ip);
				strcpy(pcmd.left->argv[i+1], server_port);
				strcpy(pcmd.left->argv[i+2], "display");
				pcmd.left->argv[i+3]=NULL;

				//second save right in pcmd -> right
				pcmd.right = new struct execcmd;
				pcmd.right->type = EXEC;
				i = 0;
				temp++;
				while(tokens[temp+i]!=NULL)
				{
					pcmd.right->argv[i]= new char[MAX_TOKEN_SIZE];
					strcpy(pcmd.right->argv[i], tokens[temp+i]);
					i++;
				}
				run((void *)&pcmd);
				
			}
			else{
				fprintf(stderr, "Unkown Command");
			}
		 
		 break;

		case REDIR:
			rcmd.type = REDIR;
			//getfl and >
			if(strcmp(tokens[0], "getfl")==0)
			{	
				int i = 0;
				//copy all args till >
				while(tokens[i]!=NULL && strcmp(tokens[i],">" )!=0)
				{	
					rcmd.argv[i] = new char[MAX_TOKEN_SIZE];
					strcpy(rcmd.argv[i],tokens[i]);
					i++; 
				}
				//check for extra or less args
				if(i!=2 || tokens[i+1] ==NULL)
				{	
					fprintf(stderr, "Invalid arguments\n");
					break;
				}
				//copy file name and ip port and display attrs
				rcmd.file = new char[64];
				strcpy(rcmd.file, tokens[i+1]);
				rcmd.argv[i] = new char[MAX_TOKEN_SIZE];
				rcmd.argv[i+1] = new char[MAX_TOKEN_SIZE];
				rcmd.argv[i+2] = new char[MAX_TOKEN_SIZE];
				rcmd.argv[i+3] = NULL;
				
				strcpy(rcmd.argv[i], server_ip);
				
				strcpy(rcmd.argv[i+1], server_port);
				strcpy(rcmd.argv[i+2], "display");

				//call run based on the commands setup
				run((void*)&rcmd);
				//free memory
				delete[] rcmd.file;
				
			}
			else{
				fprintf(stderr, "Unkown Command");
			}
		
		 break;

	}

	int j;
	//free all memory
	if(tok==EXEC)
	{
		j = 0;
		while(cmd.argv[j]!=NULL)
		{
			free(cmd.argv[j]);
			j++;
		}
		if(cmd.argv!=NULL)free(cmd.argv);
	}

	//free memory
	if(tok==PIPE)
	{
		j = 0;
		while(pcmd.left->argv[j]!=NULL){
			free(pcmd.left->argv[j]);
			j++;
		}
		if(pcmd.left!=NULL)free(pcmd.left);
		j = 0;
		while(pcmd.right->argv[j]!=NULL){
			free(pcmd.right->argv[j]);j++;
		}
		if(pcmd.right!=NULL)free(pcmd.right);
	}

	//free memory 
	if(tok==REDIR)
	{
		j = 0;
		while(rcmd.argv[j]!=NULL){
			free(rcmd.argv[j]);
			j++;
		}
		if(rcmd.argv!=NULL)free(rcmd.argv);
		
		
	}
}

void sig_handler(int sig)
{	
	int status;
	int pid;
	//handle's CTRL + C
	if(sig==SIGINT)
	{	
		//reap foreground process
		while((pid = waitpid(-1, &status, WNOHANG))>0){}
	}//signal from child exit
	else if(sig == SIGCHLD)
	{	
		//remove zombies
		
		while((pid = waitpid(-1, &status, WNOHANG))>0){
			//if a background process
			if(bpid.find(pid) != bpid.end())
			{
				printf("Background Process %d Completed\n",pid);
				bpid.erase(pid);
			}
		}

	}//termination signal
	else if(sig == SIGTERM)
	{
		//reap zombies and exit
		//reap foreground process
		while((pid = waitpid(-1, &status, WNOHANG))>0){}
		exit(0);
	}
}

int main()
{
	printf("SHELL::");

	printf("Type exit to termminate shell\n");
	//Current Working Directory
	char cwd[MAX_TOKEN_SIZE];
	//command
	char command[MAX_INPUT_SIZE];
		
	char **tokens;

	//builtins mapping
	map< string , FnPtr > builtins;
	map_builtin(builtins);

	//setup signal handler    
	signal(SIGINT, sig_handler);
	signal(SIGCHLD, sig_handler);
	signal(SIGTERM, sig_handler);
	while(1)
	{	
		//print directory
		getcwd(cwd, MAX_TOKEN_SIZE);
		if(cwd==NULL)
			fprintf(stderr,"Couldn't Initiate. Current Directory\n");
		printf(BOLDWHITE "%s $ " RESET, cwd);
		//Clear all streams
		fflush(NULL);


		//Read a Command
		if(fgets(command,MAX_INPUT_SIZE,stdin)==NULL)
			printf("Error reading from input");
		//tokenize
		tokens = tokenize(command);
		if(*tokens == NULL)
		{
			free(tokens);
			continue;
		}
		//search if builtin

		map<string , FnPtr >::iterator it = builtins.find(tokens[0]);
		if( ( it ) != builtins.end() )
		{
			(it->second)(tokens);
		}
		else {
			//external executables 
			parse(tokens);
		}
			
			

		//free allocated memory
		for(int i=0;tokens[i]!=NULL;i++)
		{
	 		free(tokens[i]);
       	}
       	free(tokens);
     }

     //exit
     // while(waitpid(-1, ) >0){

     // }

     exit(0);
		

}
