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
int server_port = -1;
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
	server_port = atoi(args[2]);
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
	char *argv[MAX_TOKEN_SIZE];
	char *eargv[MAX_TOKEN_SIZE];
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

struct backcmd {
	int type;
	struct cmd *left;
	struct cmd *right;
};

void run(struct cmd *cmd)
{	
	//pipe
	int p[2]
	if(cmd ==NULL)
		exit(0);

	switch(cmd->type)
	{
		case EXEC:
			struct execcmd *ecmd = (struct execcmd *)cmd;
			//check if no command
			if(ecmd-> argv[0] == NULL)
				fprintf(stderr, "Exec Error\n");
			else if(fork()==0) 
				execv(ecmd->argv[0], argv);
			else wait(&status);
		case PIPE:
		case REDIR:
		case BACK:
	}
}
//Command Parsing
char **type(char **start)
{	
	char **sep = start
	while(sep!=NULL)
	{
		if(strcmp(start[0],'>' )==0)
			return sep;
		else if(strcmp(start[0], '|')==0)
			return sep;
		else if(strcmp(start[0],'getbg')==0)
			return sep;
		sep++;
	}
	return sep;
}
int gettoken(char **sep)
{		
	if(sep==NULL)
		return EXEC;
	if(strcmp(sep[0],'>')==0)
		return REDIR;
	else if(strcmp(sep[0],'|')==0)
		return PIPE;
	else if(strcmp(sep[0],'getbg')==0)
		return BACK;
}
struct cmd * parse(char **tokens)
{
	char **sep = type(start);
	int tok = gettoken(sep);
	
	struct *cmd = execcmd();
	switch(tok)
	{
		case EXEC:
			

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
			runcmd(tokens);
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
