#include "automaton.h"

typedef struct msg_t
{
	long mtype;
	short edge[ZONE_SIDE_SIZE];
} Message; 

const unsigned short MSG_LEN = sizeof(Message) - sizeof(long);

typedef struct Zone_t {
	short** zone;
	size_t iteration;
} Zone;

/*
 * FUNCTION copy_edge :
 *		This function creates a new table and copies the content of the edge sent as parameter
 *
 *		PARAMETERS :
 *			- A pointer on the edge we want to copy
 *
 *		RETURNS :
 *			- NULL: if the argument was invalid
 *			- The copy of the edge otherwise
 */
static short* copy_edge(short* edge);


/*
 * FUNCTION free_edge :
 *		This function frees the memory allocated to a table "edge"
 *
 *		PARAMETERS :
 *			- A pointer on the edge we want to delete
 *
 */
static void free_edge(short** edge);

/*
 * FUNCTION free_zone :
 *		This function frees the memory allocated to a zone
 *
 *		PARAMETERS :
 *			- A pointer on the zone to delete
 *
 */
static void free_zone(void* zone);

/*
 * FUNCTION init_zone :
 *		This function creates a Zone, filling its fields with the parameters.
 *
 *		PARAMETERS :
 *			- iteration : the number of cycle already done
 *			- **zone : the table of states of nodes
 *		RETURNS
 *			- The Zone created
 *
 */
static Zone* init_zone(size_t iteration, short** zone);

/*
 * FUNCTION compute_next :
 *		This function compute the next state of the zone
 *
 *		PARAMETERS :
 *			- **prev_state : the last state of the zone
 *			- **received_edge : the state of the nodes of the direct neighbours
 *			- iteration : the number of cycles already computed
 *			- process_number : the id of the process: an integer between 0 and 15
 *		RETURNS
 *			- The new Zone 
 *
 */
static Zone* compute_next(short** prev_state, short** received_edge, size_t iteration, int process_number);

/*
 * FUNCTION send_edge :
 *		This function sends the state of the nodes touching nodes of the neighbours.
 *
 *		PARAMETERS :
 *			- **curr_state : the state of the nodes
 *			- *send_qids : the id of the queues we are sending the message
 *			- process_number : the process number
 *		RETURNS
 *			- -1 if an error occurred 
 *			- 0 otherwise
 *
 */
static int send_edges(short** curr_state, int* send_qids, size_t process_number);

/*
 * FUNCTION send_edge :
 *		This function receives the messages from the neighbours and save it .
 *
 *		PARAMETERS :
 *			- **edge : the table in which we want to save the states
 *			- *receive_qid : the id of the queues correponding to this process
 *			- process_number : the process number
 *		RETURNS
 *			- -1 if an error occurred 
 *			- 0 otherwise
 *
 */
static int receive_edges(short** edges, int receive_qid, int process_number);

void free_zone(void* zone) 
{
	Zone* z = zone;

	for(size_t i = 0; i < ZONE_SIDE_SIZE; i++)
		free(z->zone[i]);

	free(z->zone);
	free(z);
}

Zone* init_zone(size_t iteration, short** zone) 
{
	Zone* z = malloc(sizeof(Zone));

	if(z == NULL)
		return NULL;

	if(zone == NULL)
	{
		z->zone = malloc(ZONE_SIDE_SIZE * sizeof(short*));

		if(z->zone == NULL)
			return NULL;

		for(size_t i = 0; i < ZONE_SIDE_SIZE; i++)
			z->zone[i] = malloc(ZONE_SIDE_SIZE * sizeof(short));
	}
	else
		z->zone = zone;

	z->iteration = iteration;

	return z;
}

static short* copy_edge(short* edge)
{
	if(edge == NULL)
		return NULL;

	short* to_return = malloc(ZONE_SIDE_SIZE * sizeof(short));

	for(size_t i = 0; i < ZONE_SIDE_SIZE; i++)
		to_return[i] = edge[i];

	return to_return;
}

static void free_edge(short** edge)
{
	if(*edge == NULL)
		return;
	free(*edge);
}

int send_edges(short** curr_state, int* send_qids, size_t process_number)
{
	int error = 0;
	char dir[4] = {'r', 'l', 'b', 't'};
	// send_qids order : r l b t
	Message msg;

	// send message to the right, to the left, to the bottom and to the top
	for(size_t i = 0; i < 4; i++)
	{
		msg.mtype = i + 1;
		
		if(send_qids[i] != -1)//check for neighbour
		{
			// copying zone edge in message struct
			for(size_t j = 0; j < ZONE_SIDE_SIZE; j++)
			{
				// selecting edge according to the neighbour
				switch(i)
				{
					case 0 : // right
						msg.edge[j] = curr_state[j][ZONE_SIDE_SIZE - 1];
						break;
					case 1 : // left
						msg.edge[j] = curr_state[j][0];
						break;
					case 2 : // bottom
						msg.edge[j] = curr_state[ZONE_SIDE_SIZE - 1][j];
						break;
					case 3 : // top
						msg.edge[j] = curr_state[0][j];
				}
			}

			// sending message
			if(msgsnd(send_qids[i], &msg, MSG_LEN, 0) < 0)
			{
				fprintf(stderr, "Process %zu : Cannot send message %zu (to %c) : %s\n", process_number, i, dir[i], strerror(errno));
				error = -1;
			}
		}
	}

	return error; // -1 if an error has occured
}

int receive_edges(short** edges, int receive_qid, int process_number)
{
	/*
		mtype send receive
		  1     r     l
		  2     l     r
		  3     b     t
		  4     t     b
	*/ 
	char from[4] = {'l', 'r', 't', 'b'};
	bool neighbours[4];

	// check if neighbours exist
	neighbours[0] = process_number % 4 != 0;
	neighbours[1] = process_number % 4 != 3;
	neighbours[2] = process_number >= 4;
	neighbours[3] = process_number <= 11;

	Message msg;

	for(int i = 0; i < 4; i++)
	{
		// send message if neighbour exists
		if(neighbours[i])
		{	
			if(msgrcv(receive_qid, &msg, MSG_LEN, i + 1, 0) < 0)
			{
				fprintf(stderr, "Cannot receive message %i (from %c) : %s\n", i, from[i], strerror(errno));
				return -1;
			}

			edges[i] = copy_edge(msg.edge);
		}
		else
			edges[i] = NULL;
	}
	return 0;
}

Zone* compute_next(short** prev_state, short** received_edge, size_t iteration, int process_number)
{	
	// 'received_edge' lines corresponds to : 0 -> l ; 1 -> r; 2 -> t; 3 -> b
	Zone* z = init_zone(iteration, NULL);

	if(z == NULL)
		return NULL;

	// index in the received edge array
	const unsigned short LEF = 0, RIG = 1, TOP = 2, BOT = 3;

	for(size_t i = 0; i < ZONE_SIDE_SIZE; i++)
	{
		for(size_t j = 0; j < ZONE_SIDE_SIZE; j++)
		{
			short top = -1, bottom = -1, right = -1, left = -1;

			// set top state
			if(i != 0)
				top = prev_state[i - 1][j];
			else if(received_edge[TOP] != NULL)
				top = received_edge[TOP][j];

			// set bottom state
			if(i != ZONE_SIDE_SIZE - 1)
				bottom = prev_state[i + 1][j];
			else if(received_edge[BOT] != NULL)
				bottom = received_edge[BOT][j];

			// set right state
			if(j != ZONE_SIDE_SIZE - 1)
				left = prev_state[i][j + 1];
			else if(received_edge[RIG] != NULL)
				left = received_edge[RIG][i];
				
			// set left state
			if(j != 0)
				right = prev_state[i][j - 1];
			else if(received_edge[LEF] != NULL)
				right = received_edge[LEF][i];

			short incr_state = (prev_state[i][j] + 1) % MAX_STATE;

			// update
			z->zone[i][j] = prev_state[i][j];

			if(top == incr_state || bottom == incr_state
				 || left == incr_state || right == incr_state)
				z->zone[i][j] = incr_state;
		}
	}

	return z;
}

int automaton_process(size_t process_number, short** init_state, int* send_qids, int receive_qid, 
				int sem_table_id, int sem_iter_id, int shm_table_id,
				int shm_iter_id, long max_iteration, size_t step)
{
	if(init_state == NULL || send_qids == NULL || process_number > 15 || step == 0)
	{
		fprintf(stderr, "Error : bad argument (function 'automaton_process')\n");
		return -1;
	}
	
	List* zone_states = ListNew(free_zone); 

	if(zone_states == NULL)
	{
		fprintf(stderr, "Error : cannot create list (process %zu)\n", process_number);
		return -1;
	}

	long current_iteration = 1; // current iteration (=> [n])
	
	if(max_iteration <= 1)
		max_iteration = MAX_ITERATION;

	Zone *prev_zone = init_zone(0, init_state), // zone state at iteration [n - 1]
		 *current_zone = NULL; // zone state at iteration ([n])

	short **neighbour_edge; // contains the edge cells received from the neighbours (iteration [n - 1])

	if((neighbour_edge = malloc(4 * sizeof(short*))) == NULL)
	{
		fprintf(stderr, "Cannot allocate memory (process %zu)\n", process_number);
		ListDispose(zone_states, true);
		return -1;
	}
	
	while(current_iteration < max_iteration)
	{
		// send edges at state [n - 1] to neighbours
		if(send_edges(prev_zone->zone, send_qids, process_number) < 0) 
			return -1;
		
		// receive neighbours' edge cells state at iteration [n - 1]
		if(receive_edges(neighbour_edge, receive_qid, process_number) < 0)
			return -1;	
		// compute next state [n] 
		if((current_zone = compute_next(prev_zone->zone, neighbour_edge, current_iteration, process_number)) == NULL)
			return -1;

		for(size_t i = 0; i < 4; i++)
			free_edge(&neighbour_edge[i]);

		// save state [n - 1] in the list if it matches the incrementation step		
		if(!((current_iteration - 1) % step))
			ListInsertAtBegin(zone_states, prev_zone);
		else 
			free_zone(prev_zone);

		prev_zone = current_zone;
		current_zone = NULL;

		// update shared array (if possible)  
		if(!ListEmpty(zone_states))
		{	
			// get the oldest zone in the list
			Node* oldest_zone_node = ListGetTail(zone_states);
			Zone* oldest_zone = NodeGetData(oldest_zone_node);

			// find the iteration being built in the shared memory
			int shm_iter = get_current_iteration(shm_iter_id, sem_iter_id);
			
			if(shm_iter == -1)
				break;

			// if the oldest zone's iteration matches the iteration being built => update shared table
			if(oldest_zone->iteration == (size_t) shm_iter)
			{
				update_shared_table(shm_table_id, oldest_zone->zone, process_number, sem_table_id);
				ListRemoveNode(zone_states, oldest_zone_node, true);
			}
		}
		
		// The List of Zones can't be too long, if it is, we stop computing states and display till the number 
		// of state computed is reduced to MAX_ADVANCEMENT / 2
		if(ListGetNumElem(zone_states) > MAX_ADVANCEMENT)
		{
			for(size_t i = 0; i < MAX_ADVANCEMENT/2; i++)
			{
				Node* oldest_node = ListGetTail(zone_states);
				Zone* oldest_zone = NodeGetData(oldest_node);
				
				update_shared_table(shm_table_id, oldest_zone->zone, process_number, sem_table_id);
				ListRemoveNode(zone_states, oldest_node, true);
			}
		}

		current_iteration++;
	}


	// save or free the zone of the last iteration
	if((current_iteration - 1) % step)
		ListInsertAtBegin(zone_states, prev_zone);
	else
		free_zone(prev_zone);

	// send to shared memory the states still stored in the list
	while(!ListEmpty(zone_states))
	{
		Node* oldest_node = ListGetTail(zone_states);
		Zone* oldest_zone = NodeGetData(oldest_node);
			
		update_shared_table(shm_table_id, oldest_zone->zone, process_number, sem_table_id);
		ListRemoveNode(zone_states, oldest_node, true);
	}

	ListDispose(zone_states, true);
	free(neighbour_edge);

	return 0;
}