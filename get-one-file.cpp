#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/time.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>

#define DATAPACKETS 2048

int download_made[1000000];
double download_time[1000000];
double avg_response_time[1000000];

long int download = 0;

struct data
{
    struct hostent *he;
	char file_name[100];
    int port;
    int num_thread;
    int display;
    int thread_id;
};  


void *download_file(void* dt)
{

    struct data *vec;
    vec = (struct data *)dt;

    struct hostent *he;
    he = vec->he;

    if(he == NULL)
    {
        printf("error\n");
        exit(1);
    }

    int port = vec->port;
    
    int display = vec->display;//
    int thread_id = vec->thread_id;

    int file_number;
    struct timeval t_initial, t_final;
    struct timeval x,y;

    int sockfd, numbytes;
    char message[100];
    
    char recv_data[DATAPACKETS];
    
    struct sockaddr_in their_addr;
	
	
        if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        {
            perror("socket");
            exit(1);
        }
  
        their_addr.sin_family = AF_INET; 
        their_addr.sin_port = htons(port);
        their_addr.sin_addr = *((struct in_addr *)he->h_addr);
        bzero(&(their_addr.sin_zero),8);

        if(connect(sockfd, (struct sockaddr *)&their_addr,sizeof(struct sockaddr)) == -1)
        {
            perror("connect");
            exit(1);
        }
		
		strcpy(message,"get ");
		strcat(message,vec->file_name);
        //sprintf(message,"get files/foo0.txt");
       
        send(sockfd,message,100,0);
        gettimeofday(&x,NULL);

        while((numbytes = recv(sockfd,recv_data,DATAPACKETS,0)) > 0)
        {
            if(display==1)printf("%s",recv_data);
			if(numbytes !=-1)download = download + numbytes;
        }
        
        if(numbytes == -1)
        {
            perror("recv error");
        }
        else
        {
            download_made[thread_id]++;
            download_time[thread_id] += ((y.tv_sec*1000000+y.tv_usec)-(x.tv_sec*1000000+x.tv_usec));

			
        }

        close(sockfd);
    
}

int main(int argc, char* argv[])
{
    //printf("apple");
    int total_download=0;
    double time_total=0,t_total, throughput, avg_time;

    struct hostent *he;
    
    he = gethostbyname(argv[2]);

    if(he == NULL)
    {
        printf("unknown host %s, try again! \n", argv[1]);
        exit(1);
    }
	//./executable file_name ip port variable
	char file_name[100];
	strcpy(file_name, argv[1]);
	printf("%s\n",file_name);

    int port = atoi(argv[3]);

    int num_thread = 1;

    char *type = argv[4];

    char *type_d = "display";
    char *type_n = "nodisplay";

    int display;//

    if(strcmp(type,type_d)==0)
    {
        display=1;
    }
    else if(strcmp(type,type_n)==0)
    {
        display=0;
    }
    else
    {
        printf("unknown type %s\n",type);
        exit(1);
    }
    
    pthread_t thread[num_thread];
    
    int i;
    struct data vec[num_thread];

    srand(time(NULL));

    i=0;
    //printf("apple\n");
    //printf("%d\n",num_thread);
    struct timeval t_initial, t_final;
    gettimeofday(&t_initial,NULL);
    while(i<num_thread)
    {
        vec[i].he = (struct hostent*) he;
		strcpy(vec[i].file_name,file_name);
        vec[i].port = port;
        vec[i].num_thread = num_thread;
        vec[i].display = display;
        vec[i].thread_id = i;

        download_made[i]=0;
        download_time[i]=0.0;
        pthread_create(&thread[i], NULL, download_file, &vec[i]);
        i++;
        //printf("ball\n");
    }
    

    
    i=0;
    while(i<num_thread)
    {
        //printf("cat\n");
        //printf("%d\n",thread[i]);
        pthread_join(thread[i], NULL);
        i++;
    }
    gettimeofday(&t_final,NULL);
    i=0;
    
    while(i<num_thread)
    {
        //printf("doll\n");
        avg_response_time[i] = (download_made[i]+0.0)/download_time[i];
        total_download = total_download + download_made[i];
        time_total = time_total + download_time[i];
        i++;
    }

    i=0;
    avg_time = 0;
    while(i<num_thread)
    {
        avg_time = avg_time + avg_response_time[i];
        i++;
    }

    avg_time = avg_time/num_thread;
    avg_time = avg_time;

    t_total = t_final.tv_sec-t_initial.tv_sec;
    throughput = total_download/t_total;

    avg_time = time_total/total_download;
    avg_time = avg_time/1000000;
    
    
    printf("All Files Downloaded! \n");
    printf("Throughput is %f requests/s \n",throughput);
    printf("Average Response Time is %f sec \n",avg_time);
}