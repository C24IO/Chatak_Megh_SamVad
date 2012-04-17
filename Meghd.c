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
/*
 * Here in the structures I am using the Host identification as 
 * struct hostent req_host 
 *
 * to put in our value 
 *
 * I will have to do the following steps 
 *
 * 1)	Take host name as a string 
 * 2)	struct hostent *host_ptr = gethostbyname(remote_host_string)	
 * 3)	Then use the fields inside the structure for further usage.
 * 
* */



/*		TO DO 
 *
 *	1) HISTORY THREAD
 *	2) HISTORY THREAD USAGE
 *	3) SCHEDULER THREAD
 *	4) MAKE CHATAK DO THE FLOCK FROM SETTING ALSO 
 *	5) CPU IDLE THREAD
 *	6) MAKE THE MODEL MULTITHREADED
 *	7) REQUEST QUEUE
 *	8) INTERFACE ??
 *
 *
 *
 * */
struct SCHED_DATA{

	char req_host_str[256];
	char free_host_str[256];
	
};





struct REQ_HOSTS{

	char req_host_str[256];
	char *free_hosts_ptr[256];
	int hosts_taken;
	long int rank;
	int nice_val;
	long int req_queue_len;
	unsigned long int report_count;
	int time_fact1;
	int time_fact2;
	int stability_fact;

};

/* The free_host_ptr is the array of pointers which stores the address of the free hosts this host has acquired .
We will also store the number of hosts acquired  in hosts_taken , upto now. */

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


/* The given_to_host store the address of the host this free host has been assigend for
 atmost only one can be assigned */
 
 
typedef struct reqhost_dllist {

	struct reqhost_dllist  *flink;
	struct reqhost_dllist  *blink;
	struct REQ_HOSTS reqhost;

} *reqhostDllist;






typedef struct freehost_dllist {

	struct freehost_dllist  *flink;
	struct freehost_dllist  *blink;
	struct FREE_HOSTS freehost;

} *freehostDllist;

/*Function Prototypes*/




freehostDllist new_dllist_freehost();
void free_dllist_freehost(freehostDllist);
void dll_delete_node_freehost(freehostDllist);
int dll_empty_freehost(freehostDllist);
void dll_append_freehost(freehostDllist, struct FREE_HOSTS);
void dll_prepend_freehost(freehostDllist, struct FREE_HOSTS);
void dll_insert_b_freehost(freehostDllist, struct FREE_HOSTS);
void dll_insert_a_freehost(freehostDllist,struct FREE_HOSTS);
struct FREE_HOSTS  dll_val_freehost(freehostDllist);




reqhostDllist new_dllist_reqhost();
void free_dllist_reqhost(reqhostDllist);
void dll_delete_node_reqhost(reqhostDllist);
int dll_empty_reqhost(reqhostDllist);
void dll_append_reqhost(reqhostDllist, struct REQ_HOSTS);
void dll_prepend_reqhost(reqhostDllist, struct REQ_HOSTS);
void dll_insert_b_reqhost(reqhostDllist, struct REQ_HOSTS);
void dll_insert_a_reqhost(reqhostDllist,struct REQ_HOSTS);
struct REQ_HOSTS  dll_val_req_host(reqhostDllist);





/*Function Prototypes*/

int _get_host_name(char *buffer, int len);
int init_socket(int *sock_fd, int port_no);
ssize_t mod_read(int fd, void *buf, size_t size);
ssize_t mod_write(int fd, void *buf, size_t size);

	

/*# defines and macros for easy operations.*/


#define dll_first(d) ((d)->flink)
#define dll_next(d) ((d)->flink)
#define dll_last(d) ((d)->blink)
#define dll_prev(d) ((d)->blink)
#define dll_nil(d) (d)

#define dll_traverse(ptr, list) \
	  for (ptr = list->flink; ptr != list; ptr = ptr->flink)
#define dll_rtraverse(ptr, list) \
	  for (ptr = list->blink; ptr != list; ptr = ptr->blink)
#define MEGH_PORT      19061
#define REQ_HOST_PORT  19062
#define FREE_HOST_PORT 19063

#define BACK_LOG 10

#define JOB_QUEUE_LENGTH_THRESH 20
#define AVG_CPU_IDLE_THRESH  60
#define HOSTS_FLOCKING_THRESH 2

/* The two threads of the program ...*/

void *req_host_handler( void *);
void *free_host_handler( void *);

/* The Scheduler Thread. */

void *scheduler (void *unsused);

/* Global Variables to be shared between threads */

/*These are the socket file descriptors to call the accept upon in the threads.*/
int req_host_sock, free_host_sock;
extern int errno;	
int errno_temp;


/*Create a new doubly linked list for the requestor hosts here */
	
reqhostDllist req_host_list_1, req_host_list_tmp,  req_host_list_tmp2;
freehostDllist free_host_list_1, free_host_list_tmp, free_host_list_tmp2;
	
/*You will have to protect this guy with a mutex.*/

/*Yes you are right ..... and here are the mutexes.*/

pthread_mutex_t reqhost_list_mutex  = PTHREAD_MUTEX_INITIALIZER ,freehost_list_mutex  = PTHREAD_MUTEX_INITIALIZER ;
pthread_mutex_t req_host_sock_mutex  = PTHREAD_MUTEX_INITIALIZER , free_host_sock_mutex  = PTHREAD_MUTEX_INITIALIZER ;

/*Signal Handlers */


static void sig_hup(int signo);
static void sig_segv(int signo);



int main(void){

	int status = 0;
	
	

	/* pthread things are here .....*/


	pthread_t req_handler_thread, free_handler_thread;
	
	pthread_attr_t req_handler_attr, free_handler_attr;


	/* Initialising the default values for the attributes. */

	
	pthread_attr_init(&req_handler_attr);
	pthread_attr_init(&free_handler_attr);

	/*The initialization of the diffrent DLLS deployed. */	  

	req_host_list_1 = new_dllist_reqhost(); 
	free_host_list_1 = new_dllist_freehost(); 

/*-------------------------------SIGNAL HANDLERS START----------------------------------------------*/	

	signal(SIGHUP, sig_hup);
	signal(SIGSEGV, sig_segv);

	
/*-------------------------------SIGNAL HANDLERS STOP ----------------------------------------------*/	

	/*	
	 * What I am doing here is that I am keeping all the passive part in main and the active
	 * part is done in the threads.
	 *
	 * Here goes the initialisation and listening of the Requstor Host Handler and the initialisation 
	 * and the listening of the Free Host handler follows.
	 *
	 * */
	
	
/*-------------------------------REQUESTOR HOST HANDLER----------------------------------------------*/	
	
	init_socket(&req_host_sock, REQ_HOST_PORT);

	status = listen(req_host_sock, BACK_LOG);
	
	
	if(-1 == status){
		perror("listen() for Requestor Host Handler");
		exit(1);
	}

/*-------------------------------REQUESTOR HOST HANDLER INIT END----------------------------------------------*/	

/*-------------------------------FREE HOST HANDLER----------------------------------------------*/	

	init_socket(&free_host_sock, FREE_HOST_PORT);

	status = listen(free_host_sock, BACK_LOG);
	
	
	if(-1 == status){
		perror("listen() fo Free Host Handler");
		exit(1);
	}

/*-------------------------------FREE HOST HANDLER INIT END----------------------------------------------*/	
	




/*The Thread creation*/


	status = pthread_create(&req_handler_thread, &req_handler_attr, &req_host_handler, NULL);
	
	if( status != 0 ){
		perror("req_handler_thread creation error.");
		exit(1);
	}

	status = pthread_create(&free_handler_thread, &free_handler_attr, &free_host_handler, NULL);
	
	if( status != 0 ){
		perror("free_handler_thread creation error.");
		exit(1);
	}

	printf("\n\nTHE THREAD IDS ARE REQUEST_HANDLER : %ld ,FREE_HANDLER : %ld  \n\n", req_handler_thread, free_handler_thread);

	status = pthread_join( req_handler_thread , NULL);
	status = pthread_join( free_handler_thread , NULL);

/* Shall I create another thread to wait for these two to join ??? **/
	
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
			
void *req_host_handler( void * req_params){

	/* 
	 * Here the data is being passed from the calling function as a void pointer so typecast the pointer
	 * in the variables declaration ssection to avoid problems with it later.
	 *
	 * You can also pass NULL as the argument from the calling function.
	 *
	 * When returnting the data , you can return NULL or any address just remember to typecast it onthe other side. 
	 */ 
	
	/*
	 * free_host_sock and req_host_sock has been initialised and is being listened upon, now here I am 
	 * calling the accept. 
	 * 
	 * What I have to decide here after stabilising is that should I deploy a semaphore here 
	 * for the socket file descriptors or not ?
	 *
	 * */
	
	/*Here I have to make a temporary varible to hold the value of the requestor_host what info it is sending and then append it to the dll.*/

	struct REQ_HOSTS req_host_tmp;
	struct REQ_HOSTS req_host_continuity;
	int i = 0, flag = 0 ;


	for(;;){

		struct sockaddr_in chatak_name = {0};
		int chatakReader, chatakLength = sizeof(chatak_name);
		char buffer[256] = {0};	
		flag = 0;

		(void) memset(&chatak_name, 0 , sizeof(chatak_name));


		chatakReader = accept(req_host_sock, (struct sockaddr *) &chatak_name ,(socklen_t *) &chatakLength);
		
		if(-1 == chatakReader){
			perror("accept()");
			exit(1);
		}
	
		if( -1 == getpeername(chatakReader, (struct sockaddr *) &chatak_name, &chatakLength) ){

			perror("getpeername()");
		}

		else{

			printf("REQUESTOR HOST Connection Request arrived from %s \n", inet_ntoa(chatak_name.sin_addr));
		}

		/* Now these things are to be put inside the doubly linked list. */
		/*
		 * The order in which they are reported :
		 * 
		 * 1) Requestor Host hostname
		 * 2) Requestor Host Queue length
		 * 3) Requestor Host Time Factor 1
		 * 4) Requestor Host Time Factor 2 
		 * 5) Requestor Host Stability Factor 
		 *
		 * What remains is : rank and the nice_val these values determined by the scheduler thread.\
		 * I think I will also make the scheduler thread decide the value of TIME FACTOR 2
		 * 
		 * */

/*xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxCRITICAL SECTIONxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx*/
		
		pthread_mutex_lock(&reqhost_list_mutex);

			
		pthread_mutex_lock(&req_host_sock_mutex);
		 /* 1) Requestor Host hostname*/
		mod_read(chatakReader,buffer,sizeof(buffer));

		/*
		 * Here Basically we find out if there is previous host entry of this host 
		 * in this DLL if yes we delete that entry and then insert new values.
		 *
		 * */

		dll_traverse(req_host_list_tmp, req_host_list_1 ){

	/*I think I may have got the bug , this had me hanging for 2 days straight, man it was a big one ......
	 *
	 *	It may be for a reason because I am solving many problems here such as :
	 *
	 *	We have to maintain the continuity with the last instance of the reporting which is already inside the CDLLS
	 *	
	 *	The report count will be maintained here 
	 *	The values which are to be carried on to this instance of reporting will be initialised here.
	 *	The values which are to be refreshed will be done so downstairs.
	 *	
	 *	The use of the continuity variable is for sucha thing.
	 *
	 *	carry forward :
	 *
	 *	req_host_continuity.*free_hosts_ptr
	 * 	req_host_continuity.hosts_taken
	 * 
	 *	manipulate :
	 *
	 *      req_host_continuity.time_fact1
	 *      req_host_continuity.rank
	 *      req_host_continuity.nice_val
	 *	req_host_continuity.stability_fact
	 *	req_host_continuity.report_count
         *
	 *	refresh :
	 *
	 * 	req_host_continuity.time_fact2
	 *  	req_host_continuity.req_queue_len
         * 
	 *	
	 *
	 * */
			
			/* If its old , already reported before*/	
			if (!strcmp(req_host_list_tmp->reqhost.req_host_str,buffer)){

				memset(&req_host_continuity , 0 , sizeof(req_host_continuity));
				printf("\nIts old %s", buffer);

				/*req_host_continuity.req_host_str*/
				/*req_host_continuity.req_queue_len*/
				/*req_host_continuity.time_fact2*/

				for( i = 0 ; i < 256 ; i++){
					
					req_host_continuity.free_hosts_ptr[i] = req_host_list_tmp->reqhost.free_hosts_ptr[i];

				}

				req_host_continuity.hosts_taken =  req_host_list_tmp->reqhost.hosts_taken;

				/* Manipulate */

				req_host_continuity.report_count =  1 + (req_host_list_tmp->reqhost.report_count );
					

			
				req_host_continuity.rank = rand() / 100000;
				req_host_continuity.nice_val = rand() / 100000; 
				req_host_continuity.time_fact1 = rand() / 100000; 
				req_host_continuity.stability_fact = rand() / 100000;
				
				req_host_continuity.time_fact2 = rand() / 100000; 


				/*delete the node*/
				req_host_list_tmp2 =  req_host_list_tmp->flink;
				dll_delete_node_reqhost((req_host_list_tmp));
				req_host_list_tmp =  req_host_list_tmp2;

				/* Set a flag that it was found and values taken care of .*/

				flag = 1;


			}


																				  
		}



		if(!flag){
			
			/* If its new , not found in the CDLLS, reporting for the first time */	

				memset(&req_host_continuity , 0 , sizeof(req_host_continuity));

				/* req_host_continuity.req_host_str*/
				
			        /* req_host_continuity.req_queue_len*/
					 
			       /*  req_host_continuity.time_fact2 TODO NOT USED INIT TO RANDOM */

				printf("\nIts new %s", buffer);
  				
				for( i = 0 ; i < 256 ; i++){req_host_continuity.free_hosts_ptr[i] = NULL;}
   

				 req_host_continuity.hosts_taken = 0;

				 req_host_continuity.report_count = 0;
				 
				 req_host_continuity.rank = 1;
				 req_host_continuity.nice_val = 1;
				 req_host_continuity.time_fact1 = 1 ;
				 req_host_continuity.stability_fact = 1;
				 req_host_continuity.time_fact2 = 1;

				 flag = 0;
		}
		
		strcpy(req_host_tmp.req_host_str,buffer);
		memset(buffer,0,sizeof(buffer));

		 /* 2) Requestor Host Queue length*/
		mod_read(chatakReader,buffer,sizeof(buffer));
		req_host_tmp.req_queue_len = atol(buffer);
		memset(buffer,0,sizeof(buffer));


		/*Reading ends here so unlock the socket.*/

		pthread_mutex_unlock(&req_host_sock_mutex);
		

		/* Putting the values here which are manipulated and carry forwarded but not refreshed*/
		
		req_host_tmp.rank =  req_host_continuity.rank ;
		req_host_tmp.time_fact1 =  req_host_continuity.time_fact1;
		req_host_tmp.time_fact2 =  req_host_continuity.time_fact2;
		req_host_tmp.nice_val =  req_host_continuity.nice_val  ;
		req_host_tmp.stability_fact =  req_host_continuity.stability_fact ;
		req_host_tmp.hosts_taken =  req_host_continuity.hosts_taken;

		
		req_host_tmp.report_count =  req_host_continuity.report_count;
		
		
		/*Now finally add the node to the DLL we have made. :*/
		
		dll_append_reqhost(req_host_list_1 , req_host_tmp);

		pthread_mutex_unlock(&reqhost_list_mutex);

		/*To see if all is ok .....*/



		pthread_mutex_lock(&reqhost_list_mutex);
		printf("\n\n------------------------------------------------------------------------\n\n");

		dll_traverse(req_host_list_tmp, req_host_list_1 ) printf("\n%s queue_length  -> %ld\nhosts_taken %d\nrank %d\nnice_val %d\nreport_count %d\ntime_fact1 %d\ntime_fact2 %d\nstability_fact %d\n ",

				req_host_list_tmp->reqhost.req_host_str, 
				req_host_list_tmp->reqhost.req_queue_len,
				req_host_list_tmp->reqhost.hosts_taken,
				req_host_list_tmp->reqhost.rank,
				req_host_list_tmp->reqhost.nice_val,
				req_host_list_tmp->reqhost.report_count,
				req_host_list_tmp->reqhost.time_fact1,
				req_host_list_tmp->reqhost.time_fact2,
				req_host_list_tmp->reqhost.stability_fact
				);
		
		pthread_mutex_unlock(&reqhost_list_mutex);
		/*
		 * ssize_t recv(int s, void *buf, size_t len, int flags); 
		 * use the following flags MSG_OOB MSG_PEEK MSG_WAITALL MSG_DONTWAIT 
		 *
		 * what this MSG_WAITALL  will do is that it will wait for the whole buffer to fill up 
		 * what this MSG_DONTWAIT  will do is that it will not wait for the whole buffer to fill up 
		 * and return immideately.
		 *
		 * Man this DONTWAIT is not defined by POSIX !!!!!
		 *
		 *
		 *
		recv(chatakReader,buffer,sizeof(buffer),MSG_DONTWAIT);
		puts(buffer);*/
		memset(buffer,0,sizeof(buffer));


		/* 
		 * Here the scheduler will work while giving away addresses to the requestor 
		 * host.
		 * 
		 * 
		 * */
	
		scheduler(NULL);

		
		pthread_mutex_lock(&reqhost_list_mutex);

		
		printf("\n\n-----------------------SCHEDULED DATA------------------------------\n\n");

		dll_traverse(req_host_list_tmp, req_host_list_1 ){

			for( i = 0 ; i < req_host_list_tmp->reqhost.hosts_taken ;  i++){

				if(req_host_list_tmp->reqhost.free_hosts_ptr[i]){ printf("b"); break;}

				puts(req_host_list_tmp->reqhost.free_hosts_ptr[i]);
			}

			printf("\nNEXT\n");
		}
		
		pthread_mutex_unlock(&reqhost_list_mutex);

		strcpy(buffer, "158.144.64.53");

		mod_write(chatakReader, buffer, strlen(buffer) );
		
		close(chatakReader);
	}
			

	
}





void *free_host_handler( void * free_params){



	/* 
	 * Here the data is being passed from the calling function as a void pointer so typecast the pointer
	 * in the variables declaration ssection to avoid problems with it later.
	 *
	 * You can also pass NULL as the argument from the calling function.
	 *
	 * When returnting the data , you can return NULL or any address just remember to typecast it onthe other side. 
	 * 
	 */
	
	/*
	 * free_host_sock and req_host_sock has been initialised and is being listened upon, now here I am 
	 * calling the accept. 
	 * 
	 * What I have to decide here after stabilising is that should I deploy a semaphore here 
	 * for the socket file descriptors or not ?
	 *
	 * */

	struct FREE_HOSTS free_host_tmp;

	
	for(;;){

		struct sockaddr_in chatak_name = {0};
		int chatakReader, chatakLength = sizeof(chatak_name);
		char buffer[256] = {'\0'};
		char buffer2[256] = {'\0'};
		int i = 0;

/*		free_host_tmp = (struct FREE_HOSTS *) malloc(sizeof(struct FREE_HOSTS));

		(void) memset(free_host_tmp, 0 , sizeof(struct FREE_HOSTS));*/
		(void) memset(&chatak_name, 0 , sizeof(chatak_name));


		chatakReader = accept(free_host_sock, (struct sockaddr *) &chatak_name ,(socklen_t *) &chatakLength);
		
		if(-1 == chatakReader){
			perror("accept()");
			exit(1);
		}
	
		if( -1 == getpeername(chatakReader, (struct sockaddr *) &chatak_name, &chatakLength) ){

			perror("getpeername()");
		}

		else{

			printf("FREE HOST Connection Request arrived from %s \n", inet_ntoa(chatak_name.sin_addr));
		}
		
		
		pthread_mutex_lock(&freehost_list_mutex);


		pthread_mutex_lock(&free_host_sock_mutex);
		
		/* 1) Free Host hostname*/
		mod_read(chatakReader,buffer,sizeof(buffer));

		/*
		* Here basically we find out if there is previous host entry of this host 
		* in this DLL if yes we delete that entry and then insert new values.
		*
		* */

/* PROBLEM :
 *	
 *	The strcmp function gets a very lagre value when the hostname is changed or the 
 *	thread is done from a free host handler to a requesting host handler. 
 *	This value is what cause the illegal seek errno=29 error.
 *
 *
 *	 			      hostname minix
 *	 root@minix:/cogito/Threaded# hostname minix.ecom.tifr.res.in
 *	 root@minix:/cogito/Threaded# hostname minix.it.mitp
 *	 root@minix:/cogito/Threaded# hostname minix
 *
 *	 now error....
 *	 
 * */
		
		
		dll_traverse(free_host_list_tmp, free_host_list_1 ){

			/*printf(" \n\nI am Slepping \n\n");*/
			sleep(0);
			
			
			if (!strcmp(free_host_list_tmp->freehost.free_host_str,buffer)){
				
	/*I think I may have got the bug , this had me hanging for 2 days straight, man it was a big one ......*/
				
				free_host_list_tmp2 =  free_host_list_tmp->flink;
				dll_delete_node_freehost((free_host_list_tmp));
				free_host_list_tmp =  free_host_list_tmp2;


			}

		}


		
		strcpy(free_host_tmp.free_host_str,buffer);
		memset(buffer,0,sizeof(buffer));
		/* CPU KFlops */
		
		mod_read(chatakReader,buffer,sizeof(buffer));
		free_host_tmp.cpu_kflops = atol(buffer);
		memset(buffer,0,sizeof(buffer));
		
		/*CPU Mips */		
		mod_read(chatakReader,buffer,sizeof(buffer));
		free_host_tmp.cpu_mips = atol(buffer);
		memset(buffer,0,sizeof(buffer));
		
		/*Host Memory*/		
		mod_read(chatakReader,buffer,sizeof(buffer));
		free_host_tmp.memory = atol(buffer);
		memset(buffer,0,sizeof(buffer));

		/*The cpu avg idle reported by the Reporter Thread of the Chatak.*/


		mod_read(chatakReader,buffer,sizeof(buffer));
		free_host_tmp.avg_cpu_idle = atof(buffer);
		memset(buffer,0,sizeof(buffer));

		/*Reading ends here so we unlock the free host socket mutex*/


		pthread_mutex_unlock(&free_host_sock_mutex);

		

		/*putting the dummy values here */

		free_host_tmp.rank = 0;
		free_host_tmp.stability_fact = 0;
		free_host_tmp.hosts_flocking = 0;
		free_host_tmp.nice_val = 0;

		for(i = 0 ; i < 256 ; i++)
			free_host_tmp.given_to_host[i] = NULL;
		
		/*Now finally add the node to the DLL we have made. :*/
		
		dll_append_freehost(free_host_list_1 , free_host_tmp);


		/*To see if all is ok .....*/

		pthread_mutex_unlock(&freehost_list_mutex);


		pthread_mutex_lock(&freehost_list_mutex);
		printf("\n------------------------------------------------------------------------");

		dll_traverse(free_host_list_tmp, free_host_list_1 ) printf("\n%s\n kflops %ld Mips %ld Memory %ld AVG CPU IDLE %f \n",
		free_host_list_tmp->freehost.free_host_str, 
		free_host_list_tmp->freehost.cpu_kflops, 
		free_host_list_tmp->freehost.cpu_mips ,
		free_host_list_tmp->freehost.memory ,
		free_host_list_tmp->freehost.avg_cpu_idle);
		
		pthread_mutex_unlock(&freehost_list_mutex);
		

		close(chatakReader);
		
	}
			
}


int init_socket(int *sock_fd, int port_no){


	int lis_sock = *sock_fd,	   
	    status = 0,
	    on = 0,
	    port = port_no;

	struct hostent *hostptr = NULL;

	char hostname[80] = {'\0'};

	struct sockaddr_in megh_name = {0};


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



	(void) memset(&megh_name, 0 , sizeof(megh_name));

	(void) memcpy(&megh_name.sin_addr, hostptr->h_addr, hostptr->h_length);

	/* for 0.0.0.0:9918 
	 *
	  megh_name.sin_addr.s_addr=htonl(INADDR_ANY);
	 * 
	 * */
		
	megh_name.sin_addr.s_addr=htonl(INADDR_ANY);
	megh_name.sin_family = AF_INET;

	megh_name.sin_port = htons(port);

	status = bind( lis_sock, (struct sockaddr *) &megh_name, sizeof(megh_name));
	errno_temp = errno;

	if(-1 == status){
		
		perror("bind()");
		printf("\nThis is the errno %d \n This is the error message %s\n",errno_temp,strerror(errno_temp));
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
		
		

	
	
	

ssize_t mod_read(int fd, void *buf, size_t size) {
	ssize_t retval;
	while (retval = read(fd, buf, size), retval == -1 && errno == EINTR) ;
	return retval;
}



ssize_t mod_write(int fd, void *buf, size_t size) {
	char *bufp;
	size_t bytestowrite;	
	ssize_t byteswritten;
	size_t totalbytes;
	
	for (bufp = buf, bytestowrite = size, totalbytes = 0; bytestowrite > 0;bufp += byteswritten, bytestowrite -= byteswritten) {
		byteswritten = write(fd, bufp, bytestowrite);

		if ((byteswritten) == -1 && (errno != EINTR))
			return -1;
		if (byteswritten == -1)
			byteswritten = 0;
		totalbytes += byteswritten;
	}
	
	return totalbytes;
}
        
/*
* Each list contains a sentinal node, so that     
*  * the first item in list l is l->flink.  If l is  
*   * empty, then l->flink = l->blink = l.            
*/


reqhostDllist new_dllist_reqhost()
{
	reqhostDllist d;
	  
	d = (reqhostDllist) malloc (sizeof(struct reqhost_dllist ));
	d->flink = d;
	d->blink = d;
	return d;
}
 

void dll_delete_node_reqhost(reqhostDllist node)		/* Deletes an arbitrary iterm */
{
	node->flink->blink = node->blink;
	node->blink->flink = node->flink;
	free(node);
}

int dll_empty_reqhost(reqhostDllist l)
{
	return (l->flink == l);
}
 
void free_dllist_reqhost(reqhostDllist l)
{
	while (!dll_empty_reqhost(l)) {
		dll_delete_node_reqhost(dll_first(l));
	}
	free(l);
}
 





void dll_insert_b_reqhost(reqhostDllist node,struct REQ_HOSTS v)       /* Inserts before a given node */
{
	reqhostDllist newnode;
	  
	newnode = (reqhostDllist) malloc (sizeof(struct reqhost_dllist ));
	newnode->reqhost = v;
	newnode->flink = node;
	newnode->blink = node->blink;
	newnode->flink->blink = newnode;	  
	newnode->blink->flink = newnode;
}

void dll_insert_a_reqhost(reqhostDllist n,struct REQ_HOSTS reqhost)        /* Inserts  REQ_HOSTS reqhost  after a given node */
{
	dll_insert_b_reqhost(n->flink, reqhost);
}






void dll_append_reqhost(reqhostDllist l, struct REQ_HOSTS reqhost)     /* Inserts  REQ_HOSTS reqhost at the end of the list */
{
	dll_insert_b_reqhost(l, reqhost);
}





void dll_prepend_reqhost(reqhostDllist l,struct REQ_HOSTS reqhost)    /* Inserts  REQ_HOSTS reqhost at the beginning of the list */
{
	dll_insert_b_reqhost(l->flink, reqhost);
}





struct REQ_HOSTS dll_val_reqhost(reqhostDllist l) /* gives value of  REQ_HOSTS reqhost */
{
	return l->reqhost;
}




/*-------------------------------------------------------FREE HOST DLL HANDLING FUNCTIONS--------------------------------------------*/


/*
* Each list contains a sentinal node, so that     
*  * the first item in list l is l->flink.  If l is  
*   * empty, then l->flink = l->blink = l.            
*/


freehostDllist new_dllist_freehost()
{
	freehostDllist d;
	  
	d = (freehostDllist) malloc (sizeof(struct freehost_dllist ));
	d->flink = d;
	d->blink = d;
	return d;
}
 

void dll_delete_node_freehost(freehostDllist node)		/* Deletes an arbitrary iterm */
{
	node->flink->blink = node->blink;
	node->blink->flink = node->flink;
	free(node);
}

int dll_empty_freehost(freehostDllist l)
{
	return (l->flink == l);
}
 
void free_dllist_freehost(freehostDllist l)
{
	while (!dll_empty_freehost(l)) {
		dll_delete_node_freehost(dll_first(l));
	}
	free(l);
}
 





void dll_insert_b_freehost(freehostDllist node,struct FREE_HOSTS v)       /* Inserts before a given node */
{
	freehostDllist newnode;
	 
	struct FREE_HOSTS *tempnode;

        tempnode = (struct FREE_HOSTS *) malloc (sizeof (struct FREE_HOSTS) );
        
	
 	*tempnode = v;
	
	
	newnode = (freehostDllist) malloc (sizeof(struct freehost_dllist ));
	newnode->freehost = *tempnode;
	newnode->flink = node;
	newnode->blink = node->blink;
	newnode->flink->blink = newnode;	  
	newnode->blink->flink = newnode;
}

void dll_insert_a_freehost(freehostDllist n,struct FREE_HOSTS freehost)        /* Inserts FREE_HOSTS freehost  after a given node */
{
	dll_insert_b_freehost(n->flink, freehost);
}






void dll_append_freehost(freehostDllist l, struct FREE_HOSTS freehost)     /* Inserts FREE_HOSTS freehost at the end of the list */
{
	dll_insert_b_freehost(l, freehost);
}





void dll_prepend_freehost(freehostDllist l,struct FREE_HOSTS freehost)    /* Inserts FREE_HOSTS freehost at the beginning of the list */
{
	dll_insert_b_freehost(l->flink, freehost);
}





struct FREE_HOSTS dll_val_freehost(freehostDllist l) /* gives value of FREE_HOSTS freehost */
{
	return l->freehost;
}

/*The Scheduler thread will work onthe two dlls and then it will give out approproate addressess */


/*I am now trying to put a signal handler for the signal SIGHUP to give me the total size of the CDLLs */

static void sig_hup(int signo){

	printf("\n\nHi I caught the SIGNAL %d \n\n",signo);
}
	



static void sig_segv(int signo){

	errno_temp = errno;
	perror("SIGSEGV");
	printf("\nThis is the errno %d \n This is the error message %s\n",errno_temp,strerror(errno_temp));


	return;

}



/* THis is the main thread which will be called by the Requestor Host Handler Thread.
 * It will be run by the requestor host when the request for a host comes in.
 * It will
 *
 * 	Work on the two CDLLS using mutexes
 * 	After working on them using a specific method, will feed the data in the REQ_HOST CDLLS
 * 	It will put no of hosts in the respective member fields
 * 	It will adjust the respective entries in the Free Host CDLL also
 * 	Then it will give back control to the Req Host Thread.
 *
 * Then when control comes to the Requestor Host Handler Thread, 
 * It will
 *
 * 	get the control
 * 	find the requesting host in the list
 * 	see how many hosts are alloted
 * 	send first the no of hosts alloted 
 * 	and then send the hosts one by one over the ether
 * 	This can be used to put up multiple entries in the FLOCK_TO field in the chatak
 *
 * If I design two algos there will be two functions, or a switch in between
 * I may include the time factor in one of them,
 * time factor means the amount of time a host has got the flockable host.
 *
 *
 * 
 * We can see if we can take care of the FLOCK_FROM field in the respective chataks.
 *
 * NO NEED TO PASS ARGUMENTS IT IS WORKING ON COMMON DATA THE CDLLS
 * 
 * 	
 * 	*/

void *scheduler (void *unsused){



	char *host_str_temp;


	/*Scheduling with FCFS policy
	 *
	 * 	Take the first requesting host and give it the first Free host available.
	 *
	 * 	WRT the following constrataints
	 *
	 * 	The Requesting Host does not have more than 4  flockable hosts
	 * 	The Requesting Host fulfills the threshold of idle jobs
	 *	
	 *
	 * 	The Free Host does not have more than 2 serviced hosts 
	 *
	 *
	 * 	
	 *
	 * */
		pthread_mutex_lock(&freehost_list_mutex);
		pthread_mutex_lock(&reqhost_list_mutex);

		printf("\n INSIDE 1 \n");
		
		dll_traverse(req_host_list_tmp, req_host_list_1 ){

				
			if ((req_host_list_tmp->reqhost.hosts_taken <  4 ) && (req_host_list_tmp->reqhost.req_queue_len > JOB_QUEUE_LENGTH_THRESH) ){
		printf("\n INSIDE 2 \n");

				dll_traverse(free_host_list_tmp, free_host_list_1 ){
					
					
					if ((free_host_list_tmp->freehost.hosts_flocking <  HOSTS_FLOCKING_THRESH ) && \
							(free_host_list_tmp->freehost.avg_cpu_idle > AVG_CPU_IDLE_THRESH) ){
		printf("\n INSIDE 3 \n");

						host_str_temp = (char *) malloc (sizeof(req_host_list_tmp->reqhost.req_host_str));
						strcpy(host_str_temp , free_host_list_tmp->freehost.free_host_str);
						req_host_list_tmp->reqhost.free_hosts_ptr[req_host_list_tmp->reqhost.hosts_taken] = host_str_temp;
						req_host_list_tmp->reqhost.hosts_taken += 1;
					
				}

			}

		}

		}
		
		pthread_mutex_unlock(&freehost_list_mutex);
		pthread_mutex_unlock(&reqhost_list_mutex);
	
}	
