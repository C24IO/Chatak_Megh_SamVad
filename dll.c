struct REQ_HOSTS{

	        struct hostent *req_host;
		long int rank;
		int nice_val;
		long int req_queue_len;

};


/*use this for the finding out of the ipaddress here struct hostent *gethostbyname(const char *name); */

struct FREE_HOSTS{

	        struct hostnet *free_host;
		long int rank;
		int nice_val;
		int cpu_power;
		long int memory;
		double avg_cpu_idle;
};



typedef struct reqhost_dllist {

	struct dllist *flink;
	struct dllist *blink;
	REQ_HOSTS reqhost;

} *reqhostDllist;

typedef struct freehost_dllist {

	struct dllist *flink;
	struct dllist *blink;
	FREE_HOSTS freehost;

} *freehostDllist;


/*Function Prototypes*/


/*Common functions */
Dllist new_dllist();
void free_dllist(Dllist);

void dll_delete_node(Dllist);
int dll_empty(Dllist);


/*for requestor hosts*/

void dll_append_req_hosts(Dllist, REQ_HOSTS);
void dll_prepend_req_hosts(Dllist, REQ_HOSTS);
void dll_insert_b_req_hosts(Dllist, REQ_HOSTS);
void dll_insert_a_req_hosts(Dllist, REQ_HOSTS);


REQ_HOSTS  dll_val_req_host(Dllist);

/*for free hosts */

void dll_append_free_hosts(Dllist, FREE_HOSTS);
void dll_prepend_free_hosts(Dllist, FREE_HOSTS);
void dll_insert_b_free_hosts(Dllist, FREE_HOSTS);
void dll_insert_a_free_hosts(Dllist, FREE_HOSTS);


FREE_HOSTS dll_val_free_host(Dllist);


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

#endif





/*
* Each list contains a sentinal node, so that     
*  * the first item in list l is l->flink.  If l is  
*   * empty, then l->flink = l->blink = l.            
*/


Dllist new_dllist()
{
	  Dllist d;
	  
	  d = (Dllist) malloc (sizeof(struct dllist));
	  d->flink = d;
	  d->blink = d;
	  return d;
}
 






void dll_insert_b(Dllist node, Jval v)       /* Inserts before a given node */
{
	  Dllist newnode;
	  
	  newnode = (Dllist) malloc (sizeof(struct dllist));
	  newnode->val = v;
	  newnode->flink = node;
	  newnode->blink = node->blink;
	  newnode->flink->blink = newnode;	  
	  newnode->blink->flink = newnode;
}

void dll_insert_a_req_hosts(Dllist n, REQ_HOSTS reqhost)        /* Inserts  REQ_HOSTS reqhost  after a given node */
{
	  dll_insert_b(n->flink, reqhost);
}

void dll_insert_a_free_hosts(Dllist n, FREE_HOSTS freehost)        /* Inserts FREE_HOSTS free_host after a given node */
{
	  dll_insert_b(n->flink, freehost);
}




void dll_append_req_hosts(Dllist l, REQ_HOSTS reqhost)     /* Inserts  REQ_HOSTS reqhost at the end of the list */
{
	  dll_insert_b(l, reqhost);
}

void dll_append_free_hosts(Dllist l, FREE_HOSTS freehost)     /* Inserts FREE_HOSTS free_host at the end of the list */
{
	  dll_insert_b(l, freehost);
}




void dll_prepend_req_hosts(Dllist l, REQ_HOSTS reqhost)    /* Inserts  REQ_HOSTS reqhost at the beginning of the list */
{
	  dll_insert_b(l->flink, reqhost);
}

void dll_prepend_free_hosts(Dllist l, FREE_HOSTS free_host)    /* Inserts FREE_HOSTS free_host at the beginning of the list */
{
	  dll_insert_b(l->flink, free_host);
}



REQ_HOSTS dll_val_req_hosts(Dllist l) /* gives value of  REQ_HOSTS reqhost */
{
	  return l->reqhost;
}


FREE_HOSTS dll_val_free_hosts(Dllist l) /* gives value of FREE_HOSTS freehost */
{
	  return l->freehost;
}



void dll_delete_node(Dllist node)		/* Deletes an arbitrary iterm */
{
	  node->flink->blink = node->blink;
	  node->blink->flink = node->flink;
	  free(node);
}

int dll_empty(Dllist l)
{
	  return (l->flink == l);
}
 
void free_dllist(Dllist l)
{
	  while (!dll_empty(l)) {
		      dll_delete_node(dll_first(l));
		        }
	    free(l);
}
