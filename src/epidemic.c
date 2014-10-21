#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "epidemic.h"

Person **POP = NULL; //our population
Disease DISEASE; //our disease

int POP_SIZE = 100000; //4/the size of our population
int CONNECT_RETRY = 5; //number of times we'll attempt to retry to create a connection between 2 people
int DAYS_TO_SIMULATE = 100;
OUTPUT_FORMAT OF = CSV;


//TODO: PARSE COMMAND LINE ARGUMENTS
//TODO: cleanup
//TODO: create a couple of scenarios (small population, high connectivity), (large population, low connectivity) (very infectious, deadly etc)


int main(int argc, char *argv[])
{
	int day, i;
	int sick, dead, healthy;
	
	float connectivity = 10, base_infection_rate = 0.001, exposure_infection_rate = 0.30, mortality = 0.80, immunity = 0.02, sick_len = 5;
	
	
	//parse command-line arguments
	
	create_population(connectivity, immunity);
	create_disease(base_infection_rate, exposure_infection_rate, mortality, immunity, sick_len);
	
	for(day = 0; day < DAYS_TO_SIMULATE; day++)
	{
		sick = dead = healthy = 0;
		
		for(i = 0; i < POP_SIZE; i++)
		{
			endure(POP[i]);
			
			sick += POP[i]->state == SICK;
			dead += POP[i]->state == DEAD;
			healthy += POP[i]->state == HEALTHY;
		}
		
		dump_results(OF, day, sick, dead, healthy);
	}
	
	free_population();
	return 0;
	
}


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

void dump_results(OUTPUT_FORMAT output, int day, int sick, int dead, int healthy)
{
	switch(output)
	{
		case VERBOSE:
			printf("Day %d: %d sick, %d dead, %d healthy\n", day, sick, dead, healthy);
			break;
		case CSV:
			printf("%d, %d, %d, %d\n", day, sick, dead, healthy);
			break;
		case TSV:
			printf("%d\t%d\t%d\t%d\n", day, sick, dead, healthy);
			break;
	}
	
}




void create_population(int connectivity, float immunity)
{
	printf("Building population of size %d with connectivity %d and natural immunity %f\n", POP_SIZE, connectivity, immunity);
	int i;
	POP = (Person **)malloc(POP_SIZE * sizeof(Person *));

	for(i = 0; i < POP_SIZE; i++)
	{
		POP[i] = create_person(connectivity, immunity);	
	}
	
	for(i = 0; i < POP_SIZE; i++)
	{
		make_connections(i);
	}

}


Person *create_person(int connectivity, float immunity)
{
	Person *p = (Person *)malloc(sizeof(Person));
	
	if(p != NULL)
	{
		p->new_connections_rem = (int)randf(connectivity);
		p->num_connections = 0;
		p->connections = init_queue();
		p->is_immune = (randf(1) <= immunity);
		p->days_sick_left = 0;
		return p;
	}
	else
	{
		printf("Unable to allocate person\n");
		exit(1);
	}
}



void make_connections(int self)
{
	//printf("making connections:%d\n", self);
	int i, success = 0, potential;

	while(POP[self]->new_connections_rem > 0)
	{ //we decrease the number of remaining connections regardless of whether we were able to allocate the edge, to avoid infinite loops
		
		for(i = 0; i < CONNECT_RETRY; i++)
		{
			potential = (int)randf(POP_SIZE - 1);
			
			//printf("\tedges_rem:%d, potential:%d, try:%d, connections_possible:%d\n", POP[self]->new_connections_rem,  potential, i, POP[potential]->new_connections_rem);
			//not the same person, and the person can admit another connection
			
			if(potential != self && POP[potential]->new_connections_rem > 0 && is_new_edge(self, potential))
			{
				enqueue(POP[self]->connections, potential);
				POP[self]->num_connections++;
				
				enqueue(POP[potential]->connections, self); 
				POP[potential]->num_connections++;
				POP[potential]->new_connections_rem--;
				break;
				
			}
				
		}
		
		POP[self]->new_connections_rem--;
	}

}


int is_new_edge(int self, int other)
{
	int new = 1, i, val;
	Person *p = POP[self];
	Queue *q = p->connections;
	
	if(!isempty(q))
	{
		for(i = 0; i < p->num_connections; i++)
		{
			val = dequeue(q);
			
			if(val == other)
			{
				new = 0;
				break;
			}
			
			enqueue(q, val);
		}
	}
	
	return new;
}



void create_disease(float base_infection_rate, float exposure_infection_rate, float mortality, float immunity, int sick_len)
{	
	printf("Creating disease with %f base infection rate, %f exposure_infection_rate, %f mortality, %f natural immunity and %d sickness length\n",
			base_infection_rate, exposure_infection_rate, mortality, immunity, sick_len);
	
	if(base_infection_rate <= 1)
	{
		DISEASE.base_infection_rate = base_infection_rate;
	}
	else
	{
		printf("Base Infection Rate must be <=1\n");
		exit(1);
	}
	
	if(exposure_infection_rate <= 1)
	{
		DISEASE.exposure_infection_rate = exposure_infection_rate;
	}
	else
	{
		printf("Exposure Infection Rate must be <=1\n");
		exit(1);
	}	
	
	if(mortality <= 1)
	{
		DISEASE.mortality = mortality;
	}
	else
	{
		printf("Mortality Rate must be <=1\n");
		exit(1);
	}	
	
	if(immunity <= 1)
	{
		DISEASE.immunity = immunity;
	}
	else
	{
		printf("immunity must be <=1\n");
		exit(1);
	}	
	
	if(sick_len > 0)
	{
		DISEASE.sick_len = sick_len;
	}
	else
	{
		printf("length of sickness must be >0\n");
		exit(1);
	}
	
	
}

void expose(Person *p)
{
	int infected = 0, i, lim = p->num_connections, connection;
	
	//base line exposure
	infected = randf(1) <= DISEASE.base_infection_rate;
	
	for(i = 0; i < lim && !infected && !p->is_immune; i++)
	{
		connection = dequeue(p->connections);
		
		if(POP[connection]->state == SICK)
		{
		 	infected = infected | (randf(1) <= DISEASE.exposure_infection_rate);
		}
		
		enqueue(p->connections, connection);
	}
	
	if(infected)
	{
		p->state = SICK;
		p->days_sick_left = DISEASE.sick_len;
	}
		
}

//endure one day of the simulatiom
void endure(Person *p)
{
	int die = randf(1) <= DISEASE.mortality;
	

	if(p->state == SICK && die)
	{
		p->state = DEAD;
	}
	else if(p->state == SICK && p->days_sick_left == 0)
	{
		p->state = HEALTHY;
	}
	else if(p->state == HEALTHY)
	{ //we expose healthy people to their network
		expose(p);
	}
	else
	{
		p->days_sick_left--; //will decrease for DEAD too, but we don't care about them anyways
	}
}




/* Basic implementation of a queue through linked lists*/
Queue *init_queue()
{
	Queue *q = (Queue *)malloc(sizeof(Queue));
	
	if(q == NULL)
	{
		printf("cannot allocate a queue\n");
		exit (1);
	}

	q->head = q->tail = NULL;
	return q;
}


void enqueue(Queue *q, int v)
{
	//create a element node for our v
	Queue_elem *e = (Queue_elem *)malloc(sizeof(Queue_elem));
	
	if(e == NULL)
	{
		printf("cannot allocate an element in the queue\n");
		exit(1);
	}
	
	e->next = e->prev = NULL;
	e->val = v;
	
	//place it at the tail of our queue
	if(isempty(q))
	{
		q->head = q->tail = e;
	}
	else
	{
		e->prev = q->tail;
		q->tail->next = e;
		q->tail = e;
	}
	
}

int dequeue(Queue *q)
{
	Queue_elem *e = NULL;
	int val;
	
	if(!isempty(q))
	{
		e = q->head;
		q->head = q->head->next;
		
		if(q->head == NULL)
		{ //our queue only had 1 elem in it
			q->tail = NULL;
		}
		else
		{
			q->head->prev = NULL;
		}
		
		val = e->val;
		free(e);
		return val;
	}
	else
	{
		printf("dequeuing empty queue\n");
		exit(1);
	}
}


int isempty(Queue *q)
{
	return q->head == NULL && q->tail == NULL;
}



