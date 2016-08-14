#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
int main(int argc,char *argv[])
{
	DIR *dp;
	int dir_len = 64;
	char *list = argv[1];
	char cwd[dir_len];

	if(argv[1]==NULL)
	{
		 getcwd(cwd, dir_len);
		 list = cwd;
	}
	else if(argv[2]!=NULL)
		{
			fprintf(stderr,"Invalid Arguments");return -1;
		}

	struct dirent *sd; dp=opendir(list); while((sd=readdir(dp))!=NULL)
	{
		printf("%s\t",sd->d_name);
	}
	closedir(dp);
	printf("\n");
	return 0;
}
