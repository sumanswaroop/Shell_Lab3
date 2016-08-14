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

//map builtins
void map_builtin(map<string , FnPtr > &builtins)
{
	builtins["cd"] = cd_builtin;
}


int main()
{
	printf("SHELL::");

	printf("Type exit to termminate shell\n");
	//Get Current Working Directory
	int dir_len = 64,command_len = 1024;
	char cwd[dir_len];
	char command[command_len];
		
	char **tokens;
	
	//builtins mapping
	map< string , FnPtr > builtins;
	map_builtin(builtins);

	while(1)
	{	
		//print directory
		getcwd(cwd, dir_len);
		if(cwd==NULL)
			fprintf(stderr,"Couldn't Initiate. Current Directory\n");
		printf(BOLDWHITE "%s $ " RESET, cwd);
		//Clear all streams
		fflush(NULL);

		
		//Read a Command
		if(fgets(command,1024,stdin)==NULL)
			printf("Error reading from input");
		//tokenize
		tokens = tokenize(command);

		//search if builtin
		map<string , FnPtr >::iterator it = builtins.find(tokens[0]);
		if( ( it ) != builtins.end() )
		{
			(it->second)(tokens);
		}
		else
			runcmd( parsecmd(tokens) );
			

		//free allocated memory
		for(int i=0;tokens[i]!=NULL;i++)
		{
	 		free(tokens[i]);
       	}
       	free(tokens);
     }

		

}
