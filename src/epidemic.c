#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <math.h>
#include "epidemic.h"

Person **POP = NULL; //our population
Disease DISEASE; //our disease

int POP_SIZE; //the size of our population
const int CONNECT_RETRY = 5; //number of times we'll attempt to retry to create a connection between 2 people

int main(int argc, char *argv[])
{
	char c;
	opterr = 0;
	
	//simulation parameters
	float base_infection_rate, exposure_infection_rate, mortality, immunity;
	int connectivity, sim_len, sick_len;
	
	
	if(argc != 17)
	{
		print_usage();
		exit(1);
	}
	
	//parse our options into our simulation parameters
	while((c = getopt(argc, argv, "d:n:c:b:e:m:i:l:")) != -1)
	{
		switch(c)
		{
			case 'd':
				sim_len = atoi(optarg);
				break;
			case 'n':
				POP_SIZE = atoi(optarg);
				break;
			case 'c':
				connectivity = atoi(optarg);
				break;
			case 'b':
				base_infection_rate = atof(optarg);
				break;
			case 'e':
				exposure_infection_rate = atof(optarg);
				break;	
			case 'm':
				mortality = atof(optarg);
				break;		
			case 'i':
				immunity = atof(optarg);
				break;	
			case 'l':
				sick_len = atoi(optarg);
				break;	
			default:
				print_usage();	
				exit(1);																		
		}
	}
	
	int day, i;
	int sick, dead, healthy;
	Person *subject = NULL;


	create_population(connectivity, immunity);
	create_disease(base_infection_rate, exposure_infection_rate, mortality, immunity, sick_len, sim_len);
	
	printf("day,dead,sick,healthy\n");
	
	for(day = 0; day < sim_len; day++)
	{
		sick = dead = healthy = 0;
		
		for(i = 0; i < POP_SIZE; i++)
		{
			subject = POP[i];
			endure(subject);
			
			dead += subject->state == DEAD;
			sick += subject->state == SICK;
			healthy += subject->state == HEALTHY;
		}
		
		printf("%d,%d,%d,%d\n", day + 1, dead, sick, healthy);
	}
	
	free_population();
	return 0;
	
}



/* **** Queue Implementation **** */
/* We implement a simple singly linked-list queue for integers, which we will use to keep a list of edges between nodes in our population */

/* Function: init_queue()
 * Args: None, Returns: Queue pointer
 * Allocates memory for a new queue and returns a pointer to that queue
 */
Queue *init_queue()
{
	Queue *q = (Queue *)malloc(sizeof(Queue));
	
	if(q == NULL)
	{
		printf("Queue Error: Unable to allocate queue\n");
		exit(1);
	}

	q->head = q->tail = NULL;
	return q;
}


/* Function: enqueue
 * Args: a queue, and an integer, Returns: void
 * Allocates a queue member to contain the int. Adds the new element to the queue
 */
void enqueue(Queue *q, int v)
{
	Queue_elem *e = (Queue_elem *)malloc(sizeof(Queue_elem));
	
	if(e == NULL)
	{
		printf("Queue Error: Unable to allocate an element\n");
		exit(1);
	}
	
	e->next = NULL;
	e->val = v;
	
	if(is_empty(q))
	{
		q->head = q->tail = e;
	}
	else
	{
		q->tail->next = e;
		q->tail = e;
	}
	
}


/* Function: dequeue
 * Args: a queue, Returns: int
 * Takes the head of the queue, retrieves the int, removes the element from the queue
 * and returns the int retrieved.
 */
int dequeue(Queue *q)
{
	Queue_elem *e = NULL;
	int val;
	
	if(!is_empty(q))
	{
		e = q->head;
		q->head = q->head->next;
		
		if(q->head == NULL)
		{ //our queue only had 1 elem in it
			q->tail = NULL;
		}
		
		val = e->val;
		free(e);
		return val;
	}
	else
	{
		printf("Queue Error: Dequeueing from an empty queue\n");
		exit(1);
	}
}

/* Function: is_empty
 * Args: a queue, Returns: int
 * returns 1 if a queue is empty, 0 otherwise
 */

int is_empty(Queue *q)
{
	return q->head == NULL && q->tail == NULL;
}


/* Function: is_new_elem
 * Args: a queue, an int, Returns: int
 * returns 1 if the int was not already in the queue, returns 0 otherwise
 */
int is_new_elem(Queue *q, int val)
{
	int is_new = 1;
	Queue_elem *e;
	
	//traverse the queue and compare to elements in it
	for(e = q->head; e != NULL && is_new; e = e->next)
	{
		is_new = e->val != val;
	}
	
	return is_new;
}



/* **** Simulation Implementation **** */
/* A naive simulation of how diseases could spread throughout an interconnected population */

/* Function: print_usage
 * Args: none, returns: void
 * Provides a quick reference message for what command line arguments are necessary. Printed if you provide a wrong
 * number of arguments etc.
 */
void print_usage()
{
	printf("Usage: epidemic_simulator \n"
			"Required Options:\n"
			"-d Duration of simulation\n"
			"-n Integer size of population\n"
			"-c Maximum number of edges between nodes\n"
			"-b Baseline infection rate for disease for entire simulation period\n"
			"-e Exposure infection rate (for each encounter w/ sick)\n"
			"-m Mortality rate for sickness period\n"	
			"-l Length of sickness\n");
}



/* Function: free_population
 * Args: none, returns: void
 * frees all memory allocated for the population (i.e. each person and each person's connection list), 
 * and finally the population array itself
 */
void free_population()
{
	Person *p = NULL;
	Queue *q = NULL;
	int i;
	
	for(i = 0; i < POP_SIZE; i++)
	{
		p = POP[i];
		q = p->connections;
		
		free(q);
		free(p);
	}
	
	free(POP);
}


/* Function: create_population
 * Args: degree of connectivity, immunity of population, returns: void
 * creates a population of size POP_SIZE, and then attempts to build a network where each node has [0, connectivity] edges (stochastically generated)
 * each node (person) it creates has a chance of being immune to the disease, as per the immunity parameter
 */
void create_population(int connectivity, float immunity)
{
	if(POP_SIZE <= 0)
	{
		printf("Epidemic Error: Population size must be >0\n");
		exit(1);
	}
	
	int i;
	POP = (Person **)malloc(POP_SIZE * sizeof(Person *));

	//we must create all people before we can build the network
	for(i = 0; i < POP_SIZE; i++)
	{
		POP[i] = create_person(connectivity, immunity);	
	}
	
	//attempt to create edges for each node in our population
	for(i = 0; i < POP_SIZE; i++)
	{
		make_connections(i);
	}

}

/* Function: create_person
 * Args: degree of connectivity, immunity of population, returns: Person pointer
 * create a person (node) for our population, can have up to connectivity edges and there is an immunnity chance that will be immune
 */
Person *create_person(int connectivity, float immunity)
{
	Person *p = (Person *)malloc(sizeof(Person));
	
	if(p != NULL)
	{
		p->new_connections_rem = (int)randf(connectivity);
		p->num_connections = 0;
		p->connections = init_queue();
		p->is_immune = (randf(1) <= immunity);
		p->days_sick = 0;
		return p;
	}
	else
	{
		printf("Epidemic Error: Unable to allocate person\n");
		exit(1);
	}
}


/* Function: make_connections
 * Args: integer self (index to current node) returns: void
 * tries to create edges to other nodes. It generates an edge with a random number generator. If the potential edge
 * can accept a new connection (and we don't already have this edge), we add it to our node's connections.
 * It tries to generate each edge up to CONNECT_RETRY times
 */
void make_connections(int self)
{
	int i, poss_connect;
	Person *p = POP[self];
	Queue *connections = p->connections;

	while(p->new_connections_rem > 0)
	{ 
		
		for(i = 0; i < CONNECT_RETRY; i++)
		{
			poss_connect = (int)randf(POP_SIZE - 1); //make sure to avoid array out-of-bounds, or RNG is inclusive of the upper bound.
			
			//not the same person, the person can admit another connection, and it is a new edge
			if(poss_connect != self && POP[poss_connect]->new_connections_rem > 0 && is_new_elem(connections, poss_connect))
			{
				
				enqueue(connections, poss_connect);
				p->num_connections++;
				
				//we must also make sure we add ourselves to the connection's edge list and decrease/increase appropriate counters
				enqueue(POP[poss_connect]->connections, self); 
				POP[poss_connect]->num_connections++;
				POP[poss_connect]->new_connections_rem--;
				break;
				
			}
				
		}
		
		//we decrease the number of remaining connections regardless of whether we were able to allocate the edge, to avoid infinite loops
		p->new_connections_rem--;
	}

}


/* Function: create_disease
 * Args: base infection rate, exposure infection rate, mortality rate, immunity, sickness lenght and length of simulation returns: void
 * validates the command line arguments passed in, performs some slight manipulations where appropriate (mortality, base infection rate)
 * such that the probabilities are converted to per-day probabilities. Assigns values into our global DISEASE structure.
 */
void create_disease(float base_infection_rate, float exposure_infection_rate, float mortality, float immunity, int sick_len, int sim_len)
{	
	if(base_infection_rate <= 1)
	{
		//convert to a daily risk of infection over the entire simulation period
		DISEASE.base_infection_rate = 1 - powf(1 - base_infection_rate, 1.0 / (float) sim_len);
	}
	else
	{
		printf("Epidemic Error: Base Infection Rate must be <=1\n");
		exit(1);
	}
	
	if(exposure_infection_rate <= 1)
	{
		DISEASE.exposure_infection_rate = exposure_infection_rate;
	}
	else
	{
		printf("Epidemic Error: Exposure Infection Rate must be <=1\n");
		exit(1);
	}	
	
	if(mortality <= 1)
	{
		//we convert this to the probabilty of death on a daily basis (for the length of the sickness period)
		DISEASE.mortality = 1 - powf(1 - mortality, 1.0 / (float) sick_len);
	}
	else
	{
		printf("Epidemic Error: Mortality Rate must be <=1\n");
		exit(1);
	}	
	
	if(immunity <= 1)
	{
		DISEASE.immunity = immunity;
	}
	else
	{
		printf("Epidemic Error: Immunity must be <=1\n");
		exit(1);
	}	
	
	if(sick_len > 0)
	{
		DISEASE.sick_len = sick_len;
	}
	else
	{
		printf("Epidemic Error: length of sickness must be >0\n");
		exit(1);
	}
	
	
}

/* Function: expose
 * Args: Person pointer, return: void
 * Exposes the person to the disease. There is a baseline exposure that happens regardless of whether there are sick people
 * in your network or not. If not sick from that exposure (and not immune), you are exposed to your network. Upon encountering a sick person
 * we stop scanning your connections list and generate a random number to determine infection status. 
 * If infected and not immune, your state is set to sick and the number of days sick is 0. Note that your number of days sick must be at least 1
 * to pose a risk to others. (I.e you cannot get sick and infect someone in the same simulation period).
 */
void expose(Person *p)
{
	int infected = 0, found_sick_connection = 0, i, lim = p->num_connections, connection;
	Queue *connections = p->connections;
	
	//base line exposure
	infected = (randf(1) <= DISEASE.base_infection_rate) ;
	
	//if base exposure didn't get you, expose to the network
	if(!infected && !p->is_immune)
	{
		for(i = 0; i < lim && !found_sick_connection; i++)
		{
			connection = dequeue(connections);
			found_sick_connection = POP[connection]->state == SICK && POP[connection]->days_sick > 0; //can't infect someone the same period you get infected
			enqueue(connections, connection);	
		}
		
		if(found_sick_connection)
		{
			infected = (randf(1) <= DISEASE.exposure_infection_rate);
		}
	}
	
	if(infected && !p->is_immune)
	{
		p->state = SICK;
		p->days_sick = 0;
	}
		
}

/* Function: endure
 * Args: Person pointer, return: void
 * Simulate one day for a person. If you're sick, you try to kill you with a random number generated. (I.e. death takes prevalence over other transitions).
 * If you're sick and have reached the end of your sickness, you go back to being healthy. If you're healthy, we expose you to the sickness.
 * Otherwise we  increase the count of days you have been sick.
 */
void endure(Person *p)
{
	int die = randf(1) <= DISEASE.mortality;
	
	if(p->state == SICK && die)
	{
		p->state = DEAD;
	}
	else if(p->state == SICK && p->days_sick == DISEASE.sick_len)
	{
		p->state = HEALTHY;
		p->days_sick = 0;
	}
	else if(p->state == HEALTHY)
	{ //we expose healthy people to their network
		expose(p);
	}
	else
	{
		p->days_sick++; //will increase for DEAD too, but we don't care about them anyways
	}
	
}







