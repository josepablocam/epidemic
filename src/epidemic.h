#ifndef _EPIDEMIC
#define _EPIDEMIC
#define randf(n) (((float) rand() / (float) RAND_MAX) * (n))

typedef enum { HEALTHY, SICK, DEAD } Health;

typedef struct Queue_elem {
	int val;
	struct Queue_elem *next;
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
	int days_sick;
} Person;

typedef struct Disease {
	float base_infection_rate;
	float exposure_infection_rate;
	float mortality;
	float immunity;
	int sick_len;
} Disease;


//Queue utilities
Queue *init_queue();
void enqueue(Queue *q, int v);
int dequeue(Queue *q);
int is_empty(Queue *q);
int is_new_elem(Queue *q, int v);

//Epidemic simulation
void print_usage();
void free_population();
void create_population(int connectivity, float immunity);
Person *create_person(int connectivity, float immunity);
void make_connections(int self);
int is_new_edge(int i, int j);
void create_disease(float base_infection_rate, float exposure_infection_rate, float mortality, float immunity, int sick_len, int sim_len);
void expose(Person *p);
void endure(Person *p);
#endif