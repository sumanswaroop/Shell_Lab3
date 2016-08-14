
int main()
{
	printf("SHELL::");

	while(1)
	{
		printf("Type exit to termminate shell\n");

		printf("$<");
		//Clear all streams
		fflush(NULL);

		//Read a Command
		if(fgets(line, 1024, stdin)==NULL)
			printf("Error reading from input");
		


	}
}