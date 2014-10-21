#ifndef _EPIDEMIC
#define _EPIDEMIC
#define randf(n) (((float) rand() / (float) RAND_MAX) * (n))

//structures in our simulator
typedef enum { HEALTHY, SICK, DEAD } Health;

typedef struct Queue_elem {
	int val;
	struct Queue_elem *next;
	struct Queue_elem *prev;
} Queue_elem;

typedef struct Queue {
	Queue_elem *head;
	Queue_elem *tail;
} Queue;

typedef struct Person {
	Health state;
	int num_connections;
	int new_connections_rem;
	Queue *connections;
	int is_immune;
	int days_sick_left;
} Person;

typedef struct Disease {
	float base_infection_rate;
	float exposure_infection_rate;
	float mortality;
	float immunity;
	int sick_len;
} Disease;

typedef enum {VERBOSE, CSV, TSV} OUTPUT_FORMAT;

//Queue utilities
Queue *init_queue();
void enqueue(Queue *q, int v);
int dequeue(Queue *q);
int isempty(Queue *q);


//Epidemic forward declarations
void free_population();
void create_population(int connectivity, float immunity);
Person *create_person(int connectivity, float immunity);
void make_connections(int self);
int is_new_edge(int i, int j);
void create_disease(float base_infection_rate, float exposure_infection_rate, float mortality, float immunity, int sick_len);
void expose(Person *p);
void endure(Person *p);
void dump_results(OUTPUT_FORMAT output, int day, int sick, int dead, int healthy);


#endif