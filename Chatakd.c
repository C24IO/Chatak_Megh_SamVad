#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <fcntl.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/utsname.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <errno.h>


/*For the cpu idle avg thread the global variables. It will update a global variable CPU_AVG every REPsecs. */

#define REP 5

/*Function prototypes for them */

struct cpu_info cpu_stats(void);
void get_number_of_cpus(void);
void handle_error(const char *string, int error);
void *cpu_avg_update (void *unused);
	
	
struct cpu_info {
	unsigned long long user;
	unsigned long long system;
	unsigned long long idle;
	unsigned long long iowait;
} new_cpu, old_cpu;

FILE *cpufp;                    /* /proc/stat */
char buffer[256];               /* Temporary buffer for parsing */


unsigned int n_partitions;      /* Number of partitions */
unsigned int ncpu;              /* Number of processors */
unsigned int kernel;            /* Kernel: 4 (2.4, /proc/partitions)*/


/*The global variable for the cpu timings.*/

struct cpu_avg {

	double user;
	double system;
	double idle;
	double iowait;
} CPU_AVG, CPU_AVG_temp; 
	


/*And the protecting mutexes for it .*/

pthread_mutex_t cpu_avg_mutex  = PTHREAD_MUTEX_INITIALIZER;



/*These two structures form the Parameters of the Scheduler
 *
 * OLD ONE
struct REQ_HOSTS{

	char req_host_str[256];
	long int rank;
	int nice_val;
	long int req_queue_len;
	int time_fact1;
	int time_fact2;
	int stability_fact;

};

*/

struct REQ_HOSTS{
	
	char req_host_str[256];
	char *free_hosts_ptr[256];
	int hosts_taken;
	long int rank;
	int nice_val;
	unsigned long int report_count;
	long int req_queue_len;
	int time_fact1;
	int time_fact2;
	int stability_fact;

};


/* OLD ONE
struct FREE_HOSTS{

	char free_host_str[256];
	long int rank;
	int nice_val;
	unsigned long int cpu_kflops;
	unsigned long int cpu_mips;
	unsigned long int memory;
	double avg_cpu_idle;
	int stability_fact;
};


*/


struct FREE_HOSTS{
	
	char free_host_str[256];
	char *given_to_host[256];
	int hosts_flocking;
	long int rank;
	int nice_val;
	unsigned long int cpu_kflops;
	unsigned long int cpu_mips;
	unsigned long int memory;
	unsigned long int report_count;
	double avg_cpu_idle;
	int stability_fact;
};




#define MEGH_PORT      19061
#define REQ_HOST_PORT  19062
#define FREE_HOST_PORT 19063

#define BACK_LOG 10

#define REMOTE_HOST "10.1.30.25"

/*Function Prototypes*/

int _get_host_name(char *buffer, int len);
int init_socket(int *sock_fd, int port_no);
void *reporter_thread(void *unused_void_ptr);	
void *request_thread(void *unused_void_ptr);
void *cpu_avg_thread(void * unused_void_ptr);
ssize_t mod_read(int fd, void *buf, size_t size);
ssize_t mod_write(int fd, void *buf, size_t size);
/* socket descs for the requestor and the reconfigurer. */
int reconfig_thread_sock, requestor_thread_sock;

extern int errno;
int errno_temp;


int main(void){

	int status = 0;

	FILE  *file_in = NULL  ;
	char *buff = NULL;
	size_t n = 0;
	 int i = 0 ;
	
	pthread_t request_host_thread, report_host_thread, cpu_idle_thread;

	
        pthread_attr_t request_thread_attr, report_thread_attr, cpu_thread_attr;
	

         /* Initializing the default values for the attributes. */


        pthread_attr_init(&request_thread_attr);
        pthread_attr_init(&report_thread_attr);
        pthread_attr_init(&cpu_thread_attr);


/*The Thread creation*/

	/* 
	 * The cpu thread will be outside and detached , 
	 * the reconfig and request thread will be inside 
	 * and waited upon and restarted if necessary.
	 * */

        status = pthread_create(&cpu_idle_thread, &cpu_thread_attr, &cpu_avg_update, NULL);

	if( status != 0 ){
		perror("cpu_idle_thread creation error.");
		exit(1);
	}

	/* 
	 * Now detach it from the main body. we will take in the value from a global value updated by the thread
	 * I dont think we will need a muktex here.
	 * */

	pthread_detach(cpu_idle_thread);
	
/* TO DO 
 *
 * 1) mechanism to discover all the Meghs on the network 
 * 2) The function for the proper display of error messages using following things:
 *		
 *		extern int errno;
 *
 *		If the value of errno should be preserved across a library call, it must be saved.
 *
 *		char *strerror(int errnum);
 * 
 * 
 * */
	
	
	
	
	
	
/* 
 * All these things are for the solving of the problem of passing data from a shell script to the memory of a C file via File.
 * Guess there is no other way.
 * I have done a numerical value,
 * I have done a Character String Value, 
 * That's it I think this is enough for data transfer.
 * 
 * */
	
/*	
		
	system("condor_q | grep I  | wc -l > /tmp/cvh_tmp ");

	
	file_in = fopen("/tmp/cvh_tmp", "r");

	getdelim(&buff, &n , (int) ' ', file_in);			

	puts(buff);
	
	printf("READ : %d\n\n",atoi(buff));
	
	fclose(file_in);


 	system("condor_status -l  | grep -e \"^COLLECTOR_HOST_STRING\" > /tmp/cvh_tmp1");
	
	file_in = fopen("/tmp/cvh_tmp1", "r");
	
	getdelim(&buff, &n , (int) '\n', file_in);			

	puts((const char *) buff);

	for(i = 0 ; (  (char ) * (buff + i ) != '\0' ) ; i++){

		if( (char ) * (buff + i ) == '\"'   ){
		
			i++;

			 while( (char ) * (buff + i ) != '\"'){

				 putchar((char ) * (buff + i ));
				 i++;
			 }


		}
			
	}
		
*/
		
/*	fclose(file_in);*/




	/*
	 * Now Basically I am going to put in a detached which will keep accessing the proc fs
	 * and enable me to calculate the Average CPU Idle time.
	 *
	 * This value will be updated to a global variable to be passed the Megh.
	 *
	 *
	 * Make this one thread
	 *
	 * And also for the handling of the other situation,
	 * make a infinite for loop inside that , 
	 * make a switch between 2 threads. 
	 *
	 * one for the request of a host, run lex ,reconfigure , reschedule and 
	 *
	 * other for the reporting of status to Megh.
	 *
	 * switch will be made on the basis of Job Queue Length.
	 * 
	 * ISOLATE THIS SWITCH SO AS TO MAKE THE DECISIONS BASED ON SOME OTHER THINGS AS WELL
	 *
	 * >= 10 request for host , == 0 report as free.
	 *
	 * In the main program wait for the completion of these threads.
	 *
	 * THis will make the procedure rentrant.
	 *
	 * Swicth can be done on : 
	 * Activity = "Busy"
	 * State = "Claimed"
	 * CpuBusyTime = 317
	 * CpuIsBusy = TRUE
	 * 
	 *
	 * */

	/*The promised LOOP */
	/*Loop continuously checks the status of the condor_master, and decides what to do .*/

	for(;;){

		/*SWITCH*/

		sleep(5);
			

		
		/*This thing behaves diffrently in diffrent platforms, 
		 * I was getting erro in the get delim function and then in the 
		 * atoi function, but now the strtol function seems to work fine.
		 * Lets see ....*/
        	system("condor_q | grep I  | wc -l > /tmp/cvh_tmp1 ");
		file_in = fopen("/tmp/cvh_tmp1", "r");
		
/*		(size_t ) getdelim(&buff, &n , (int) ' ', file_in);*/
		(size_t ) getline(&buff, &n , file_in);
		fclose(file_in);
		system("rm -rf /tmp/cvh_tmp1");
		
/*		i = atoi(buff);*/
		i = strtol(buff,NULL,10);

		/*For the adjustement of the job queue length*/
		i = i-1;



		/* See the queue length ..... and decide what to do .*/
		
		if(i >= 10 ){
			
			status = pthread_create(&request_host_thread, &request_thread_attr, &request_thread, NULL);
			errno_temp = errno;
			
			if( status != 0 ){
				perror("\nrequest_host_thread creation error.\n");
				printf("\nThe exact errno is %d and the error is %s\n",errno_temp, strerror(errno_temp));
				exit(1);
			}
		
			
			/*Wait for completion */
			status = pthread_join( request_host_thread , NULL);


		}
		
		else if (i == 0 ){
			
			status = pthread_create(&report_host_thread, &report_thread_attr, &reporter_thread, NULL);
			errno_temp = errno;

			if( status != 0 ){
				perror("\nreconfig_host_thread creation error.\n");
				printf("\nThe exact errno is %d and the error is %s\n",errno_temp, strerror(errno_temp));
				exit(1);
			}


			/*Wait for completion */
			
			status = pthread_join( report_host_thread , NULL);
			
		}
	}



		

		
		

	exit(0);
}

int _get_host_name(char *buffer, int length){

	struct utsname sysname;
	int status = 0;

	memset(&sysname, 0 , sizeof(sysname));
	status = uname(&sysname);

	if( -1 != status ){

			strncpy(buffer, sysname.nodename, length);
			
		}

	return(status);

}
			


int init_socket(int *sock_fd, int port_no){


	int lis_sock = *sock_fd,	   
	    status = 0,
	    on = 0,
	    port = port_no;

	struct hostent *hostptr = NULL;

	char hostname[80] = {'\0'};

	struct sockaddr_in chatak_name = {0};


	lis_sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	if(-1 == lis_sock)
	{
		perror("Error in Socket Creation......");
		exit(1);
	}

	/* For turning off the adress check and let the port numbers be reused .... **/

		
	on = 1;

	status = setsockopt(lis_sock, SOL_SOCKET, SO_REUSEADDR, (const char *) &on, sizeof(on));

	if(-1 == status ){

		perror("setsockopt(.....,SO_REUSEADDR,.......)");
	}

	{
		struct linger linger = {0};

		linger.l_onoff = 1;
		linger.l_linger = 0;

		status = setsockopt( lis_sock, 
				SOL_SOCKET, SO_LINGER, 
				(const char *) &linger, 
				sizeof(linger) );

		
		if(-1 == lis_sock){
			
		perror("setsockopt(......SO_LINGER.......)");
		exit(1);
		}

	}


	status = _get_host_name(hostname, sizeof(hostname));  

	if(-1 == status){
		perror("_get_host_name()");
		exit(1);
	}

	
	hostptr = gethostbyname(hostname);

	
	if(NULL == hostptr){
		perror("gethostbyname()");
	
		exit(1);
	}



	(void) memset(&chatak_name, 0 , sizeof(chatak_name));

	(void) memcpy(&chatak_name.sin_addr, hostptr->h_addr, hostptr->h_length);

	/* for 0.0.0.0:9918 
	 *
	 * chatak_name.sin_addr.s_addr=htonl(INADDR_ANY);
	 * 
	 * */
		
	chatak_name.sin_family = AF_INET;

	chatak_name.sin_port = htons(port);

	status = bind( lis_sock, (struct sockaddr *) &chatak_name, sizeof(chatak_name));


	if(-1 == status){
		
		perror("bind()");
		exit(1);
	}


	*sock_fd = lis_sock;
	return(status);
	
}

int daemonize(void){

	struct rlimit resourceLimit = {0};
	int status = -1;
	int fileDesc = -1;
	int i = 0;

	status = fork();
	
	switch(status){

		case -1:
			perror("Daemonizing fork() error....");
			exit(1);

		case 0:
			break;

		default: 
			exit(0);

	}


	resourceLimit.rlim_max = 0;

/* 
 * Here instead of doing close 1 2 3, elegantly call the resourcelimit function and close all the open file descriptors, 
 * remember to call it before the program gets functioning or you will be the fool. 
 *
 * */
	
	status = getrlimit(RLIMIT_NOFILE, &resourceLimit);


	if(-1 == status){
		
		perror("Daemonizing getrlimit() error.... :-( ");
		
		exit(1);
	}


	if( 0 == resourceLimit.rlim_max){
/*		fprintf(logfile,"Max Number of open file descriptors is 0 ????? !!!!!\n\n\n");*/
		perror("Max Number of open file descriptors is 0 ????? !!!!!\n\n\n");

		exit(1);
	}

	for(i = 0 ; i< resourceLimit.rlim_max ; i++){

		(void) close(i);
	}


	status = setsid();


	if(-1 == status){
		
		perror("Daemonizing setsid()");
		exit(1);
	}


	status = fork();

	switch(status){

		case -1:
			perror("Daemonizing fork() after setsid()");
			exit(1);

		case 0:
			break;

		default:
			exit(0);

	}

	chdir("/");

	umask(0);

	fileDesc = open("/dev/null", O_RDWR);

	(void) dup(fileDesc);
	(void) dup(fileDesc);


	return 0;

}
		
		

	
	
	

/*-----------------------THE THREADS OF JATAKD---------------------------------*/
/*Delete this thing a new one and a working one has been created .*/
void *cpu_avg_thread(void * unused_void_ptr){

	FILE *op_log;


	op_log = tmpfile();

	fprintf(op_log,"scanning the cpu state ......\n\n\n");
}


/*Requestor thread it will do :
 *
 * 1)  prepare a socket 
 * 2)  connect to Megh on specific port 
 * 3)  send  ip of condor_master , its queue length, one time factor indicating its busy state average (int)
 * 4)  recive one ip 
 * 5)  test ip 
 * 6)  if found bad report 
 * 7)  if good report
 * 8)  take ip rewrite config files
 * 9)  reconfig the master
 * 10) reschedule the master
 * 11) exit
 * 
 * */

void *request_thread(void *unused_void_ptr){

	/*In this see if you give the port as 0 does it get a random port or not*/


	int chatakSocket,
	    MeghPort,
	    status = 0;

	struct hostent *hostPtr  = NULL;

	struct sockaddr_in serverName = {0};

	char buffer[256] = "";
	char nc_str[40] = "\0";
	char flock_to_param[40] = "\0";
	char flock_from_param[40] = "\0";

	char *remoteHost = NULL;
	int i = 0;

	struct utsname sysname = { 0 };


	size_t n = 0;
	FILE *file_in ;
	char *buff = NULL;
	/* Here I have to put the remote host address, i have still to devlop the Megh finding 
	 * mechanism . so hard coding it.
	 * 
	 * */
	remoteHost = REMOTE_HOST;
	/*remoteHost = "minix"*/
	MeghPort = REQ_HOST_PORT;

	chatakSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);


        if(-1 == chatakSocket){
		perror("Requestor thread socket() error. ");
		exit(1);
	}

	/*resolving the remote server's ip address or server name*/	
   
	  
	hostPtr = gethostbyname(remoteHost);

	if(NULL == hostPtr){

		hostPtr = gethostbyaddr(remoteHost, strlen(remoteHost), AF_INET);

		if(NULL == hostPtr){

			perror("Error resolving the Megh Address inside the Reqestor Host Thread.");
			exit(1);
		}
	}

	serverName.sin_family = AF_INET;
	serverName.sin_port = htons(MeghPort);

	(void) memcpy( & serverName.sin_addr, hostPtr->h_addr, hostPtr->h_length);

	/*Finally connecting to the Meghd*/

	status = connect(chatakSocket, (struct sockaddr *) &serverName, sizeof(serverName));

	errno_temp = errno;

        if(-1 == status){        
		perror("Requestor thread connect() error. ");
                printf("\nThis is the errno %d \n This is the error message %s\n",errno_temp,strerror(errno_temp));
		exit(1);
	}

	/*Now here do the read write and all the stuff you want to do */
/*
 * 3)  send  ip of condor_master , its queue length, one time factor indicating its busy state average (int) , 
 *     one time factor indicating the stability of machine derived from the function utime
 * 4)  recive one ip 
 * 5)  test ip 
 * 6)  if found bad report 
 * 7)  if good report
 * 8)  take ip rewrite config files
 * 9)  reconfig the master
 * 10) reschedule the master
*/

/*  use select to check if writable, then write , use select to check if readble then read.
 *
	int select(int n, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout); 
 
 
	FD_CLR(int fd, fd_set *set); 
  	FD_ISSET(int fd, fd_set *set); 
  	FD_SET(int fd, fd_set *set); 
  	FD_ZERO(fd_set *set);*


	actually simply its like this , select will take file desc in eachof its mask,
	then it will tell using macro FD_ISSET if that particular fd is ready for read or write or exception on it

	1) FD_ZERO(fd_set mask);
	2) FD_SET(file_desc_of_use , &what_will_it_be_?_read_write_execp_mask);
	3) then put a time out, NULL -> blocks,  indefinately 0 -> polls 


	put "(fd_set *) 0 " in masks you dont want to deal with.
  	

	{
		fd_set readmask,writemask,exceptmask;
		struct timeval timeout;

		FD_ZERO(&readmask);
		FD_ZERO(&writemask);
		FD_ZERO(&exceptmask);

		
		FD_SET(chatakSocket, &readmask);
		FD_SET(chatakSocket, &writemask);


		select(chatakSocket, &readmask,&writemask,&execeptmask,&timeout);
	
	
	}

*/

	/*Now write the info to the Megh, YOU HAVE TO DEFINE YOUR OWN APPLICATION LEVEL PROTOCOL..........*/


	/*First the ip of this machine using uname.*/




	 status = uname(&sysname);

	 if( -1 != status){

		 strcpy(buffer, sysname.nodename);
	 }

	 
	status =  mod_write(chatakSocket, buffer, strlen(buffer) );

	/*now the queue length of this Condor Master */	
        system("condor_q | grep I  | wc -l > /tmp/cvh_tmp ");

	file_in = fopen("/tmp/cvh_tmp", "r");
	
/*	getdelim(&buff, &n , (int) ' ', file_in);*/
	(size_t ) getline(&buff, &n , file_in);
	  

	fclose(file_in);
	system("rm -rf /tmp/cvh_tmp ");
/*	i = atoi(buff);*/
	i = strtol(buff,NULL, 10);

	/*For the adjustement of the job queue length*/
	i = i-1;

			

	/*THis thing had me hanging by for 3 days and its so small ---> to convert an integer to a string.*/

	sprintf(buffer,"%d",i);
	puts(buffer); 
	status =  mod_write(chatakSocket, buffer, strlen(buffer) );
	 


	memset(&buffer,'\0', sizeof(buffer));
	mod_read(chatakSocket,buffer,sizeof(buffer));

	puts(buffer);
	/*Here you are getting the address now you have to write it to the configuration files and reconfigure them.*/
	  
	






	
	close(chatakSocket);


	/*	Now here we get the value from the Megh and write it to the configuration files and reconfigure and reschedule the Condor
	 *
	 *	The issue to be resolved is what to do with the multiple values ?  of the FLOCK_TO fields and FLOCK_FROM fileds
	 *	The issue is to do the configuration of the FLOCK_FROM value also. 
	 *	
	 *	
	 *
	 * */
	
	memset(nc_str,'\0',sizeof(nc_str));
	strcat(nc_str, "nc -z -w 4  ");
	strcat(nc_str, buffer  );
	strcat(nc_str, " 9618"  );

 	i = WEXITSTATUS(system(nc_str) );
 

	memset(nc_str,'\0',sizeof(nc_str));
	strcat(nc_str, "nc -z -w 4  ");
	strcat(nc_str, buffer  );
	strcat(nc_str, " 9614"  );

	/* see if the ports 9618 & 9614 are open on the free host, ie. if it is a Condor Master*/
	

	
	/*if((i == 0 ) && (  WEXITSTATUS(system(nc_str) ) == 0)){*/
		
		memset(flock_to_param,'\0',sizeof(nc_str));
		/*system("cp -v /home/condor/condor_config .");*/
		strcat(flock_to_param, "./flock_to_lexer "        );
		strcat(flock_to_param, "FLOCK_TO "     );	 
		strcat(flock_to_param,   buffer);
		puts(flock_to_param);
/*		system(flock_to_param);
		system("cat N_condor_config | grep FLOCK_ ");
		system("cp -v /home/condor/condor_config /home/condor/condor_config.lexer");
		system("cp -v  N_condor_config /home/condor/condor_config");
		system("/usr/local/condor/sbin/condor_reconfig -full -all");
		sleep(10);
		system("/usr/local/condor/sbin/condor_reschedule  -all");
	}*/


}
  
		


/* The reporter thread will do :
 * 
 * 1) prepare a socket and connect to a specific port on Megh
 * 2) report host name
 * 3) report cpu power
 * 4) report avg cpu idle time
 * 5) report memory size
 * 6) exit
 *
 * Much more things have been added on 08-04-2005
 * */

	

	
	
void *reporter_thread(void *unused_void_ptr){

	/*In this see if you give the port as 0 does it get a random port or not. */


	int chatakSocket,
	    MeghPort,
	    status = 0;
	int i  = 0 ;
	struct hostent *hostPtr  = NULL;

	struct sockaddr_in serverName = {0};

	char buffer[256] = "";

	char *remoteHost = NULL;
	
	struct utsname sysname = {0};

	size_t n = 0;
	FILE *file_in ;
	char *buff = NULL;
	/* Here I have to put the remote host address, i have still to devlop the Megh finding 
	 * mechanism . so hard coding it.
	 * 
	 * */
	remoteHost = REMOTE_HOST;
	/*remoteHost = "minix"*/
	MeghPort = FREE_HOST_PORT;

	chatakSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);


        if(-1 == chatakSocket){
		perror("Requestor thread socket() error. ");
		exit(1);
	}

	/*resolving the remote server's ip address or server name*/	
   
	  
	hostPtr = gethostbyname(remoteHost);

	if(NULL == hostPtr){

		hostPtr = gethostbyaddr(remoteHost, strlen(remoteHost), AF_INET);

		if(NULL == hostPtr){

			perror("Error resolving the Megh Address inside the Reqestor Host Thread.");
			exit(1);
		}
	}

	serverName.sin_family = AF_INET;
	serverName.sin_port = htons(MeghPort);

	(void) memcpy( & serverName.sin_addr, hostPtr->h_addr, hostPtr->h_length);

	/*Finally connecting to the Meghd*/

	status = connect(chatakSocket, (struct sockaddr *) &serverName, sizeof(serverName));
	errno_temp = errno;
        if(-1 == status){        
		perror("Requestor thread connect() error ");
                printf("\nThis is the errno %d \n\nThis is the error message %s\n\n",errno_temp,strerror(errno_temp));
		exit(1);
	}

	/*Now here do the read write and all the stuff you want to do */

	
	/*First the ip of this machine using uname.*/

	status = uname(&sysname);
	
	if( -1 != status){
		
		strcpy(buffer, sysname.nodename);
	}
	status =  mod_write(chatakSocket, buffer, strlen(buffer) );
	errno_temp = errno;
	
	if( status != strlen(buffer)){
		perror("\nrequest_host_thread creation error.\n");						        
		printf("\nThe exact errno is %d and the error is %s\n",errno_temp, strerror(errno_temp));
		exit(1);
	
	}

	/*Now report the KFlops and the MIPS , and the MEMORY of the machine CPU.*/
	
	system("condor_status -l | grep KFlops | cut --delimiter=\" \" -f 3 >  /tmp/cvh_tmp2 ");
	
	file_in = fopen("/tmp/cvh_tmp2", "r");
	getdelim(&buff, &n , (int) ' ', file_in);
	
	fclose(file_in);
	system("rm -rf /tmp/cvh_tmp2 ");
	
	printf("\nKflops = %s",buff);
	status =  mod_write(chatakSocket, buff, strlen(buff) );


	system("condor_status -l | grep Mips | cut --delimiter=\" \" -f 3 >  /tmp/cvh_tmp3 ");
	
	file_in = fopen("/tmp/cvh_tmp3", "r");
	getdelim(&buff, &n , (int) ' ', file_in);
	
	fclose(file_in);
	system("rm -rf /tmp/cvh_tmp3 ");
	
	printf("\nMips = %s",buff);
	status =  mod_write(chatakSocket, buff, strlen(buff) );

	system(" condor_status -l | grep -e \"^Memory\" | cut --delimiter=\" \" -f 3 >  /tmp/cvh_tmp4 ");
	
	file_in = fopen("/tmp/cvh_tmp4", "r");
	getdelim(&buff, &n , (int) ' ', file_in);
	
	fclose(file_in);
	system("rm -rf /tmp/cvh_tmp4 ");
	
	printf("\nMemory = %s",buff);
	status =  mod_write(chatakSocket, buff, strlen(buff) );


	/*Now remaining the stability factor and the cpu avg idle factor reporting.*/

	/*The stability factor to be calculated for history, TODO this thing in future after the history thread is done. */

	/*Now reporting the CPU AVG IDLE Value, here I can if needed can report 3 more values iowait, user, system times but doing only one now.*/

	/*CPU_AVG.idle is float then convert it to string, send it and in Megh convert back to float and save.*/	


	sleep(1);
	pthread_mutex_lock(&cpu_avg_mutex);	
	memset(&buffer,'\0', sizeof(buffer));
	sprintf(buffer,"%f",CPU_AVG.idle);
/*	puts(buffer);i*/
	status =  mod_write(chatakSocket, buffer, strlen(buffer) );
/*	printf("\nSTATUS %d",status);*/
	pthread_mutex_unlock(&cpu_avg_mutex);	

	
	close(chatakSocket);

	return(0);

}

ssize_t mod_read(int fd, void *buf, size_t size) {
	ssize_t retval;
	while (retval = read(fd, buf, size), retval == -1 && errno == EINTR);
	return retval;
}

ssize_t mod_write(int fd, void *buf, size_t size) {   
	
	char *bufp;
	size_t bytestowrite;
	ssize_t byteswritten;
	size_t totalbytes;

	for (bufp = buf, bytestowrite = size, totalbytes = 0;bytestowrite > 0;bufp += byteswritten, bytestowrite -= byteswritten){
		byteswritten = write(fd, bufp, bytestowrite);
		if ((byteswritten) == -1 && (errno != EINTR))
			return -1;
		if (byteswritten == -1)
			byteswritten = 0;
		totalbytes += byteswritten;
	}
	return totalbytes;

}

/* The CPU Avg Idle thread functions are here */

void handle_error(const char *string, int error)
{
	if (error) {
		fputs("iostat: ", stderr);
		if (errno)
			perror(string);
		else
			fprintf(stderr, "%s\n", string);
		exit(EXIT_FAILURE);
	}
}

void get_number_of_cpus(void)
{
	FILE *ncpufp = fopen("/proc/cpuinfo", "r");

	handle_error("Can't open /proc/cpuinfo", !ncpufp);
	
	while (fgets(buffer, sizeof(buffer), ncpufp)) {
		
		if (!strncmp(buffer, "processor\t:", 11))
			
			ncpu++;
	}
	
	fclose(ncpufp);
	
	handle_error("Error parsing /proc/cpuinfo", !ncpu);
}


/* This will be the main cpu avg thread */
struct cpu_info cpu_stats(void)
{
	double total;
	struct cpu_info cpu;
	int i = 0 ;

	memset(&CPU_AVG_temp, 0 , sizeof(CPU_AVG_temp));
	while (1) {



	for( i = 0 ; i < REP ; i++){


		
	sleep(1);	
		
		
	setlinebuf(stdout);
	get_number_of_cpus();
	
	memset((void *)&old_cpu,0,sizeof(old_cpu));
	memset((void *)&new_cpu,0,sizeof(new_cpu));

	cpufp = fopen("/proc/stat", "r");
	handle_error("Can't open /proc/stat", !cpufp);

	rewind(cpufp);
	while (fgets(buffer, sizeof(buffer), cpufp)) {
		if (!strncmp(buffer, "cpu ", 4)) {
			int items;
			unsigned long long nice, irq, softirq;

			items = sscanf(buffer,
				       "cpu %llu %llu %llu %llu %llu %llu %llu",
				       &new_cpu.user, &nice,
				       &new_cpu.system,
				       &new_cpu.idle,
				       &new_cpu.iowait,
				       &irq, &softirq);

			new_cpu.user += nice;
			if (items == 4)
				new_cpu.iowait = 0;
			if (items == 7)
				new_cpu.system += irq + softirq;

		}
	}

/*	old_cpu = new_cpu;

	sleep(0);

	rewind(cpufp);
	while (fgets(buffer, sizeof(buffer), cpufp)) {
	if (!strncmp(buffer, "cpu ", 4)) {
	int items;
	unsigned long long nice, irq, softirq;

	items = sscanf(buffer,
	"cpu %llu %llu %llu %llu %llu %llu %llu",
	&new_cpu.user, &nice,
	&new_cpu.system,
	&new_cpu.idle,
	&new_cpu.iowait,
	&irq, &softirq);

	new_cpu.user += nice;
	if (items == 4)
	new_cpu.iowait = 0;
	if (items == 7)
	new_cpu.system += irq + softirq;

}
}

*/	



	cpu.user = new_cpu.user - old_cpu.user ;
	cpu.system = new_cpu.system - old_cpu.system ;
	cpu.idle = new_cpu.idle - old_cpu.idle ;
	cpu.iowait = new_cpu.iowait - old_cpu.iowait ;

	total = (cpu.user + cpu.system + cpu.idle + cpu.iowait) / 100.0;


	/*printf("%3.3f %3.3f ", cpu.user /total , cpu.system /total);
	printf("%3.3f ", cpu.iowait /total);
	printf("%3.3f \n ", cpu.idle/total );*/


	/* I had the values printed first but now i want to update a global variable and pass the value to the Meghd*/
	/*
	printf("%f %f ", cpu.user /total , cpu.system /total);
	printf("%f ", cpu.iowait /total);
	printf("%f \n ", cpu.idle/total );*/

	fclose(cpufp);

	CPU_AVG_temp.user += cpu.user / total;
	CPU_AVG_temp.system += cpu.system / total;
	CPU_AVG_temp.iowait += cpu.iowait / total;
	CPU_AVG_temp.idle += cpu.idle / total;
	
	
	}

	pthread_mutex_lock(&cpu_avg_mutex);	

	CPU_AVG.user = CPU_AVG_temp.user / REP ;
	CPU_AVG.system = CPU_AVG_temp.system / REP ;
	CPU_AVG.idle = CPU_AVG_temp.idle / REP ;
	CPU_AVG.iowait = CPU_AVG_temp.iowait / REP ;

	memset(&CPU_AVG_temp, 0 , sizeof(CPU_AVG_temp));
	pthread_mutex_unlock(&cpu_avg_mutex);	

/*	printf("\n 3 user  %f idle  %f iowait  %f system  %f", CPU_AVG.user, CPU_AVG.idle, CPU_AVG.iowait, CPU_AVG.system);*/
	
	}
	return(cpu);		

	
}

/*The Thread for cpu avg times calculations and updating of the global variable.*/





/* TODO Make the function cpu_stats itself the thread .*/



void *cpu_avg_update (void *unused){

	long int i = 0;

	struct cpu_info cpu_info_temp, cpu_info_totals;

	cpu_info_temp = cpu_stats();


/*	while(1){
		
	memset(&cpu_info_temp, 0 , sizeof(cpu_info_temp));
	memset(&cpu_info_totals, 0 , sizeof(cpu_info_totals));



	for(i = 0 ; i < 1 ; i++ ){

		cpu_info_temp = cpu_stats();

		cpu_info_temp.user += cpu_info_totals.user;
		cpu_info_temp.system += cpu_info_totals.system;
		cpu_info_temp.idle += cpu_info_totals.idle;
		cpu_info_temp.iowait += cpu_info_totals.iowait;
	printf("\n 1 user  %f idle  %f iowait  %f system  %f", cpu_info_temp.user, cpu_info_temp.idle, cpu_info_temp.iowait, cpu_info_temp.system);
	printf("\n 2 user  %f idle  %f iowait  %f system  %f", cpu_info_totals.user, cpu_info_totals.idle, cpu_info_totals.iowait, cpu_info_totals.system);
	sleep(1);
	}

	cpu_info_totals.user = cpu_info_totals.user /  100;
	cpu_info_totals.idle = cpu_info_totals.idle /  100;
	cpu_info_totals.iowait = cpu_info_totals.iowait /  100;
	cpu_info_totals.system = cpu_info_totals.system /  100;

	pthread_mutex_lock(&cpu_avg_mutex);	
		
	CPU_AVG = cpu_info_totals;

	pthread_mutex_unlock(&cpu_avg_mutex);	

	printf("\n 3 user  %f idle  %f iowait  %f system  %f", CPU_AVG.user, CPU_AVG.idle, CPU_AVG.iowait, CPU_AVG.system);
	}*/

	
}

	















	




