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
#include <string.h>
#include <string>

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
char server_ip[8];
char server_port[5];
int status;
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
int server_setup(char **args)
{	
	if(args[1]==NULL || args[2] == NULL)
	{	fprintf(stderr, "Invalid Arguments");
		return -1;
	}
	strcpy(server_ip,args[1]);
	strcpy(server_port,args[2]);
	return 0;
}

//map builtins
void map_builtin(map<string , FnPtr > &builtins)
{
	builtins["cd"] = cd_builtin;
	builtins["server"] = server_setup;
}

//commands struct

struct cmd{
	int type;
};

struct execcmd{
	int type;
	char **argv;
};
struct redircmd {
	int type;
	//pointer to command
	struct cmd *cmd;
	//pointer to file argument
	char *file;
	char *efile;
	//mode of redirection
	int mode;
	int fd;
};

struct pipecmd{
	int type;
	struct cmd *left;
	struct cmd *right;
};
void map_exec(map< string, string> &executables)
{
	executables["ls"] = "/bin/ls";

}


void run(void *cmd)
{	
	//pipe
	int p[2];
	if(cmd ==NULL)
		exit(0);
	struct execcmd *ecmd;
	
	map<string ,string> executables;
	map_exec(executables);

	switch(((struct cmd *)cmd)->type)
	{
		case EXEC:
			ecmd = (struct execcmd *)cmd;
			//check if no command
			if(ecmd-> argv[0] == NULL)
				fprintf(stderr, "Exec Error\n");
			else if(fork()==0) 
				execv(ecmd->argv[0], ecmd->argv);
			else wait(&status);
			break;
		case PIPE:
			break;
		case REDIR:
			break;
		case BACK:
			break;
	}
}
//

//Command Parsing
char **type(char **start)
{	
	char **sep = start;
	int i = 0;
	while(sep[i]!=NULL)
	{
		if(strcmp(sep[i],">" )==0)
			return &sep[i];
		else if(strcmp(sep[i], "|")==0)
			return &sep[i];
		else if(strcmp(sep[i],"getbg")==0)
			return &sep[i];
		sep[i];
		i++;
	}
	return &sep[i];
}
int gettoken(char **sep)
{		
	if(sep[0]==NULL)
		return EXEC;
	if(strcmp(sep[0],">")==0)
		return REDIR;
	else if(strcmp(sep[0],"|")==0)
		return PIPE;
	else if(strcmp(sep[0],"getbg")==0)
		return BACK;
}
struct cmd * parse(char **tokens)
{	
	//seperator
	char **sep = type(tokens);
	//token like piping rediricting
	int tok = gettoken(sep);
	struct execcmd cmd;

	switch(tok)
	{
		case EXEC:
			cmd.type = EXEC;
			if(strcmp(tokens[0],"getfl")==0)
			{
				int i = 0;
				while(tokens[i]!=NULL)
				{	
					cmd.argv[i] = new char[MAX_INPUT_SIZE];
					*cmd.argv[i]=*tokens[i];
					i++;
				}
				cmd.argv[i] = new char[MAX_INPUT_SIZE];
				*cmd.argv[i] = *server_ip;
				cmd.argv[i+1] = new char[MAX_INPUT_SIZE];
				*cmd.argv[i+1] = *server_port;
				cmd.argv[i+2] = NULL;
				run((void *)&cmd);
				
			}
			else if(strcmp(tokens[0],"getsq")==0)
			{	
				int i =1;
				cmd.argv[0] = new char[MAX_INPUT_SIZE];
				*cmd.argv[0] = *tokens[0];
				cmd.argv[1] = new char[MAX_INPUT_SIZE];
				while(tokens[i]!=NULL)
				{
					*cmd.argv[1] = *tokens[i];
					run((void *)&cmd);
				}
			}
			else if(strcmp(tokens[0],"getpl")==0){

			}
			else{ 
				int i = 0;
				while(tokens[i]!=NULL)
					{	
						cmd.argv[i] = new char[MAX_INPUT_SIZE];
						*cmd.argv[i] = *tokens[i];
						i++;
					}
				cmd.argv[i] = NULL;
			}
			run((void *)&cmd);
			break;
		

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

		//search if builtin
		map<string , FnPtr >::iterator it = builtins.find(tokens[0]);
		if( ( it ) != builtins.end() )
		{
			(it->second)(tokens);
		}
		else {
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
