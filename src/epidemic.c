/*
	Epidemic is meant to be a (naive) simulator of the way a disease might spread across a network.
	Our simulator consists of a population of N individuals. Each individual can have a certain number of edges to other individuals
	(i.e. friendships, acquaintances, family etc). The number of edges E for each is generated from an uniform distribution [0, alpha], 
	where alpha is a parameter provided to moderate the "connectivity" of a society. If people are allowed to have more connections (i.e. alpha higher)
	this is represents a more densely connected society. The number of edges is meant to be random, to represent the fact that different people
	have different degrees of social interaction.

	A virus is provided with 5 parameters:
		1 - a baseline infection rate: i.e. what % of people in the population will randomly become infected by this virus (w/o contact with sick etc)
		2 - a degree of infectiousness: what % of people in contact with a sick person will get sick themselves?
		3 - a mortality rate: what is the chance of dieing during the sickness?
		4 - the length of sickness: how long is one sick?
		5 - % of population that is immune to the disease;
		
*/
	

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#define randf(n) ((float)rand() / (float)(RAND_MAX))*(n);

typedef enum { HEALTHY, SICK, DEAD } Health;

typedef struct Queue_elem
{
	int val;
	Queue_elem *next;
	Queue_elem *prev;
} Queue_elem;

typedef struct Queue
{
	Queue_elem *head;
	Queue_elem *tail;
}

typedef struct Person {
	Health state;
	int num_connections;
	int new_connections_rem;
	Queue *connections;
	int is_immune;
	int days_sick_left;
}

typedef struct Disease {
	float base_infection_rate;
	float exposure_infection_rate;
	float mortality;
	float immunity;
	int sick_len;
} Disease;


Person **POP = NULL; //our population
int POP_SIZE; //the size of our population
int CONNECT_RETRY = 4; //number of times we'll attempt to retry to create a connection
int DAYS_TO_SIMULATE = 100;
enum {VERBOSE, CSV, TSV} OUTPUTFORMAT;

Disease DISEASE; //our disease

int main(argc, char *argv[])
{
	int day, i;
	int sick, dead, healthy;
	
	create_population(connectivity, immunity);
	create_disease(base_infection_rate, exposure_infection_rate, mortality, immunity, sick_len);
	
	for(day = 0; day < DAYS_TO_SIMULATE; day++)
	{
		sick = dead = healthy = 0;
		
		for(i = 0; i < POP_SIZE; i++)
		{
			endure(pop[i]);
			
			sick += pop[i]->state == SICK;
			dead += pop[i]->state == DEAD;
			healthy += pop[i]->state == HEALTHY;
		}
		
		dump_results(OUTPUT_FORMAT, day, sick, dead, healthy);
	}
	
	return 0;
	
}


void dump_results(int output, int day, int sick, int dead, int healthy)
{
	switch(OUTPUT_FORMAT)
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
	int i;
	Person *p;
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
		
		if(p->new_connections_rem > 0)
		{
			p->connections = init_queue();
		}
		else
		{
			p->connections = NULL;
		}
		
		p->is_immune = randf(1) <= immunity;
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
	int i, success = 0, potential;

	while(POP[self]->new_connections_rem > 0)
	{
		for(i = 0; i < CONNECT_RETRY && !succcess; i++)
		{
			potential = randf(POP_SIZE - 1);
			
			//not the same person, and the person can admit another connection
			if(potential != self && POP[potential]->new_connections_rem > 0)
			{
				enqueue(POP[self]->connections, potential);
				POP[self]->num_connections++;
				
				enqueue(POP[potential]->connections, self); 
				POP[potential]->num_connections++;
				POP[potential]->new_connections_rem--;
				
				success = 1;
			}	
		}
		
		POP[self]->new_connections_rem--; //we decrease the number of remaining connections regardless of whether we were able to allocate the edge, to avoid infinite loops
		success = 0; 
	}
	
}



void create_disease(float base_rate, float exposure_rate, float mortality, float immunity, int len)
{
	DISEASE = (Disease) malloc(sizeof(Deases));
	
	if(base_rate <= 1)
	{
		DISEASE.base_infection_rate = base_rate;
	}
	else
	{
		printf("Base Infection Rate must be <=1\n");
		exit(1);
	}
	
	if(exposure_rate <= 1)
	{
		DISEASE.exposure_infection_rate = exposure_rate;
	}
	else
	{
		printf("Exposure Infection Rate must be <=1\n");
		exit(1);
	}	
	
	if(mortality <= 1)
	{
		DISEASE.mortality = exposure_rate;
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
	
	if(len > 0)
	{
		DISEASE.sick_len = len;
	}
	else
	{
		printf("length of sickness must be >0\n");
		exit(1);
	}
	
	
}

void expose(Person *p)
{
	int infected = 0, i, lim = p->num_connections;
	Person *connection = NULL;
	
	//base line exposure
	infected = randf(1)) <= DISEASE.base_infection_rate;
	
	for(i = 0; i < lim && !infected && !p->is_immune; i++)
	{
		connection = dequeue(p->connections);
		
		if(connection->state == SICK)
		{
		 	infected = infected | randf(1) <= DISEASE.infection_rate);
		}
		
		enqueue(p, connection);
	}
	
	if(infected)
	{
		p->state = SICK;
		p->days_sick_left = DISEASE.spell_length;
	}
		
}

//endure one day of the simulatiom
void endure(Person *p)
{
	int die = randf(1) <= DISEASE.mortality_rate
	

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



