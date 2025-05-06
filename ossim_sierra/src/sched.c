
#include "queue.h"
#include "sched.h"
#include <pthread.h>
#include "red_black_tree.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
static struct queue_t ready_queue;
static struct queue_t run_queue;
pthread_mutex_t queue_lock;

static struct queue_t running_list;
static int target_latency;

#ifdef CFS_SCHED
static struct RBTree *pcb_tree;
static int compare(struct pcb_t *a, struct pcb_t *b) {
	if (a->vruntime < b->vruntime) return -1;
	else if (a->vruntime > b->vruntime) return 1;
	else return a->pid - b->pid;
}
static double total_weight = 0;
#elif MLQ_SCHED
static struct queue_t mlq_ready_queue[MAX_PRIO];
static int slot[MAX_PRIO];
#endif

// int queue_empty(void) {
// #ifdef MLQ_SCHED
// 	unsigned long prio;
// 	for (prio = 0; prio < MAX_PRIO; prio++)
// 		if(!empty(&mlq_ready_queue[prio])) 
// 			return -1;
// #endif
// 	return (empty(&ready_queue) && empty(&run_queue));
// }

void init_scheduler(int time_slot_num) {
#ifdef CFS_SCHED
	initializeRBTree(&pcb_tree, compare);
#elif MLQ_SCHED
    int i ;

	for (i = 0; i < MAX_PRIO; i ++) {
		mlq_ready_queue[i].size = 0;
		slot[i] = MAX_PRIO - i; 
	}
#endif
	ready_queue.size = 0;
	run_queue.size = 0;
	target_latency = time_slot_num;
	pthread_mutex_init(&queue_lock, NULL);
}
//////////////////////////////////////////////////////////////////////////////////////
#ifdef CFS_SCHED
struct pcb_t * get_cfs_proc(void) {
	struct pcb_t * proc = NULL;
	/*TODO: get a process from PRIORITY [ready_queue].
	 * Remember to use lock to protect the queue.
	 * */

	pthread_mutex_lock(&queue_lock);

	if (removeminRBTree(pcb_tree, &proc) == 0) {
		proc->running_list = &running_list;
		enqueue(&running_list, proc);

		proc->time_slice = round(proc->weight / total_weight * target_latency);
		if (proc->time_slice < 1) proc->time_slice = 1;

		total_weight -= proc->weight;
	}

	pthread_mutex_unlock(&queue_lock);

	return proc;	
}

void put_cfs_proc(struct pcb_t * proc) {
	pthread_mutex_lock(&queue_lock);
	insertRBTree(pcb_tree, proc);
	pthread_mutex_unlock(&queue_lock);
}

void add_cfs_proc(struct pcb_t * proc) {
	pthread_mutex_lock(&queue_lock);
	total_weight += proc->weight;
	insertRBTree(pcb_tree, proc);
	proc->vruntime = 0;
	pthread_mutex_unlock(&queue_lock);	
}

struct pcb_t * get_proc(void) {
	return get_cfs_proc();
}

void put_proc(struct pcb_t * proc) {

	pthread_mutex_lock(&queue_lock);

	total_weight += proc->weight;
	remove_proc(&running_list, proc);
	proc->running_list = NULL;
	proc->vruntime += 1.0 * proc->time_slice / proc->weight;
	insertRBTree(pcb_tree, proc);

	pthread_mutex_unlock(&queue_lock);
}

void add_proc(struct pcb_t * proc) {

	pthread_mutex_lock(&queue_lock);

	proc->pcb_tree = pcb_tree;
	total_weight += proc->weight;
	proc->vruntime = 0;
	insertRBTree(pcb_tree, proc);

	pthread_mutex_unlock(&queue_lock);	
}
///////////////////////////////////////////////////////////////////////////////
#elif MLQ_SCHED
/* 
 *  Stateful design for routine calling
 *  based on the priority and our MLQ policy
 *  We implement stateful here using transition technique
 *  State representation   prio = 0 .. MAX_PRIO, curr_slot = 0..(MAX_PRIO - prio)
 */
struct pcb_t * get_mlq_proc(void) {
	static int reset_slot = 0;
	struct pcb_t * proc = NULL;
	/*TODO: get a process from PRIORITY [ready_queue].
	 * Remember to use lock to protect the queue.
	 * */

	pthread_mutex_lock(&queue_lock);

	for (int i = 0; i < MAX_PRIO; i++) {
		if (slot[i] == 0) continue;
		proc = dequeue(&mlq_ready_queue[i]);
		if (proc != NULL) {
			int proc_time_slot = proc->code->size - proc->pc;

			if (proc_time_slot > slot[i]) proc_time_slot = slot[i];
			if (proc_time_slot > time_slot) proc_time_slot = time_slot;

			slot[i] -= proc_time_slot;
			proc->time_slot = proc_time_slot;
			reset_slot = 1;
			break;
		}
	}

	if (proc != NULL) {
		enqueue(&running_list, proc);
		proc->running_list = &running_list;
	}
	else if (reset_slot == 1) {
		for (int i = 0; i < MAX_PRIO; i++)
			slot[i] = MAX_PRIO - i;
	}

	pthread_mutex_unlock(&queue_lock);

	return proc;	
}

void put_mlq_proc(struct pcb_t * proc) {
	pthread_mutex_lock(&queue_lock);
	enqueue(&mlq_ready_queue[proc->prio], proc);
	pthread_mutex_unlock(&queue_lock);
}

void add_mlq_proc(struct pcb_t * proc) {
	pthread_mutex_lock(&queue_lock);
	enqueue(&mlq_ready_queue[proc->prio], proc);
	pthread_mutex_unlock(&queue_lock);	
}

struct pcb_t * get_proc(void) {
	return get_mlq_proc();
}

void put_proc(struct pcb_t * proc) {
	proc->ready_queue = &ready_queue;
	proc->mlq_ready_queue = mlq_ready_queue;
	// proc->running_list = & running_list;

	/* TODO: put running proc to running_list */

	pthread_mutex_lock(&queue_lock);
	enqueue(&mlq_ready_queue[proc->prio], proc);
	remove_proc(&running_list, proc);
	proc->running_list = NULL;
	pthread_mutex_unlock(&queue_lock);

	return;
}

void add_proc(struct pcb_t * proc) {
	proc->ready_queue = &ready_queue;
	proc->mlq_ready_queue = mlq_ready_queue;
	// proc->running_list = & running_list;

	/* TODO: put running proc to running_list */

	// pthread_mutex_lock(&queue_lock);
	// enqueue(&running_list, proc);
	// pthread_mutex_unlock(&queue_lock);

	return add_mlq_proc(proc);
}
#else
struct pcb_t * get_proc(void) {
	struct pcb_t * proc = NULL;
	/*TODO: get a process from [ready_queue].
	 * Remember to use lock to protect the queue.
	 * */
	pthread_mutex_lock(&queue_lock);

	proc = dequeue(&ready_queue);
	if (proc != NULL) {
		proc->running_list = &running_list;
		enqueue(&running_list, proc);
		proc->time_slot = time_slot;
	}

	pthread_mutex_unlock(&queue_lock);
	return proc;
}

void put_proc(struct pcb_t * proc) {
	proc->ready_queue = &ready_queue;
	// proc->running_list = & running_list;

	/* TODO: put running proc to running_list */

	// pthread_mutex_lock(&queue_lock);
	// enqueue(&running_list, proc);
	// pthread_mutex_unlock(&queue_lock);

	pthread_mutex_lock(&queue_lock);

	enqueue(&ready_queue, proc);

	remove_proc(&running_list, proc);
	proc->running_list = NULL;

	pthread_mutex_unlock(&queue_lock);
}

void add_proc(struct pcb_t * proc) {
	proc->ready_queue = &ready_queue;
	// proc->running_list = & running_list;

	/* TODO: put running proc to running_list */

	// pthread_mutex_lock(&queue_lock);
	// enqueue(&running_list, proc);
	// pthread_mutex_unlock(&queue_lock);

	pthread_mutex_lock(&queue_lock);
	enqueue(&ready_queue, proc);
	pthread_mutex_unlock(&queue_lock);	
}
#endif


void delete_pcb(struct pcb_t *proc)
{
    if (proc->page_table != NULL) free(proc->page_table);
    if (proc->code != NULL) {
        if (proc->code->text != NULL) free(proc->code->text);
        free(proc->code);
    }
    #ifdef MM_PAGING
        if (proc->mm != NULL) {
            // Delete the paging ??????????????
            if (proc->mm->pgd != NULL) free(proc->mm->pgd);
			struct vm_rg_struct* head = proc->mm->mmap->vm_freerg_list;
			while (head != NULL) {
				struct vm_rg_struct* tmp = head;
				head = head->rg_next;
				free(tmp);
			}
			if (proc->mm->mmap != NULL) {
				free(proc->mm->mmap);
			}
			free(proc->mm);
        }
    #endif
}

void remove_pcb(struct pcb_t *proc) {	
	pthread_mutex_lock(&queue_lock);

	if (proc->running_list != NULL) {
		remove_proc(proc->running_list, proc);
	}
	else {
		// #ifdef MLQ_SCHED
		// 	remove_proc(&mlq_ready_queue[proc->prio], proc);
		// #else
		// 	remove_proc(&ready_queue, proc);
		// #endif
	}

	delete_pcb(proc);
	free(proc);
	
	pthread_mutex_unlock(&queue_lock);
}