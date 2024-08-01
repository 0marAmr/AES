#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <sched.h>

/*
 * Rate Monotonic Scheduling:       SCHED_FIFO      "Highest priority goes to smallest period"
 * Round Robin Scheduling:          SCHED_RR        "All are of the same priority"
 * Custom (Preemptive) Scheduling:  PREEM     "Priorities are user defined"
 */
#define  SCHEDULING_POLICY SCHED_FIFO

struct task_parameters{
    long period;
    int task_id;
    unsigned int n1;
    unsigned int n2;
};

// Function to compare timespec values
int timespec_cmp(struct timespec *a, struct timespec *b)
{
	if (a->tv_sec > b->tv_sec)
		return 1;
	else if (a->tv_sec < b->tv_sec)
		return -1;
	else if (a->tv_sec == b->tv_sec)
	{
		if (a->tv_nsec > b->tv_nsec)
			return 1;
		else if (a->tv_nsec == b->tv_nsec)
			return 0;
		else
			return -1;
	}
}

// Function to add microseconds to a timespec
void timespec_add_us(struct timespec *t, long us) {
    t->tv_nsec += us * 1000;
    if (t->tv_nsec >= 1000000000) {
        t->tv_nsec -= 1000000000;
        t->tv_sec += 1;
    }
}

// Function representing a periodic task with CPU-intensive operations
void *periodic_task(void *arg) {
    // Casting the argument
    struct task_parameters *params = (struct task_parameters *)arg;

    long period_us = params->period;
    int task_id = params->task_id;

    struct timespec wake_up_time, now;

    // Initialize the next execution time
    clock_gettime(CLOCK_REALTIME, &wake_up_time);

    while (1) {
        // Calculate the next absolute time
        timespec_add_us(&wake_up_time, period_us);

        unsigned  n1 = params->n1;  
        unsigned  n2 = params->n2;

        for (int i = 0; i < n1; i++) {
            for (int j = 0; j < n2; j++) {
                volatile double a = j / 2.0;  // Volatile to prevent compiler optimization
            }
        }

        // Check for deadline satisfaction
        clock_gettime(CLOCK_REALTIME, &now);

        if (timespec_cmp(&now, &wake_up_time) > 0) {
            printf("Deadline missed for task number %d\n", task_id);
        }
        else {
            printf("Task %d has met the deadline\n", task_id);
        }
        // Sleep until the next absolute time
        clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &wake_up_time, NULL);
    }
    // Irrelevant since a periodic task shall never return
    return NULL;
}


int main() {
	int th1,th2,th3;
    pthread_t thread1, thread2, thread3;
    pthread_attr_t th_attr1, th_attr2, th_attr3;
    struct sched_param my_sched_param;
    
    struct task_parameters task1_param = {
        .period = 100000,  // 100 ms
        .task_id = 1,      
        .n1 = 3000,         
        .n2 = 3000          
    };

    struct task_parameters task2_param = {
        .period = 200000,  // 200 ms
        .task_id = 2,      
        .n1 = 4000,         
        .n2 = 6000          
    };

    struct task_parameters task3_param = {
        .period = 300000,  // 300 ms
        .task_id = 3,      
        .n1 = 4000,         
        .n2 = 8000          
    };

    cpu_set_t my_cpu_set;
    CPU_ZERO(&my_cpu_set);      // Initialize to empty set
    CPU_SET(1, &my_cpu_set);     // Add CPU 1 to my_cpu_set

    // Thread 1 Attributes
    pthread_attr_init(&th_attr1);                                           // Initialize attributes for Thread 1
    pthread_attr_setinheritsched(&th_attr1, PTHREAD_EXPLICIT_SCHED);        // Set explicit scheduling inheritance
    #if(SCHEDULING_POLICY == SCHED_RR)
    pthread_attr_setschedpolicy(&th_attr1, SCHED_RR);               // Set the scheduling policy for Thread 1
    #else
    pthread_attr_setschedpolicy(&th_attr1, SCHED_FIFO);              // Set the scheduling policy for Thread 1
    #endif
    pthread_attr_setaffinity_np(&th_attr1, sizeof(cpu_set_t), &my_cpu_set); // Set CPU affinity for Thread 1
    // setting thread 1 priority
    #if(SCHEDULING_POLICY == SCHED_FIFO)
        my_sched_param.sched_priority = 30;
    #elif(SCHEDULING_POLICY == SCHED_RR)
        my_sched_param.sched_priority = 10;
    #else
        my_sched_param.sched_priority = 5;  // custom priority
    #endif
    pthread_attr_setschedparam(&th_attr1, &my_sched_param);

    // Thread 2 Attributes
    pthread_attr_init(&th_attr2);                                           // Initialize attributes for Thread 2
    pthread_attr_setinheritsched(&th_attr2, PTHREAD_EXPLICIT_SCHED);        // Set explicit scheduling inheritance
    #if(SCHEDULING_POLICY == SCHED_RR)
    pthread_attr_setschedpolicy(&th_attr2, SCHED_RR);                       // Set the scheduling policy for Thread 3
    #else
    pthread_attr_setschedpolicy(&th_attr2, SCHED_FIFO);                     // Set the scheduling policy for Thread 2
    #endif
    pthread_attr_setaffinity_np(&th_attr2, sizeof(cpu_set_t), &my_cpu_set); // Set CPU affinity for Thread 2
    // setting thread 2 priority
    #if(SCHEDULING_POLICY == SCHED_FIFO)
        my_sched_param.sched_priority = 20;
    #elif(SCHEDULING_POLICY == SCHED_RR)
        my_sched_param.sched_priority = 10;
    #else
        my_sched_param.sched_priority = 20;  // custom priority
    #endif
    pthread_attr_setschedparam(&th_attr2, &my_sched_param);

    // Thread 3 Attributes
    pthread_attr_init(&th_attr3);                                           // Initialize attributes for Thread 3
    pthread_attr_setinheritsched(&th_attr3, PTHREAD_EXPLICIT_SCHED);        // Set explicit scheduling inheritance
    #if(SCHEDULING_POLICY == SCHED_RR)
    pthread_attr_setschedpolicy(&th_attr3, SCHED_RR);               // Set the scheduling policy for Thread 3
    #else
    pthread_attr_setschedpolicy(&th_attr3, SCHED_FIFO);              // Set the scheduling policy for Thread 3
    #endif
    pthread_attr_setaffinity_np(&th_attr3, sizeof(cpu_set_t), &my_cpu_set); // Set CPU affinity for Thread 3
    // setting thread 3 priority
    #if(SCHEDULING_POLICY == SCHED_FIFO)
        my_sched_param.sched_priority = 10;
    #elif(SCHEDULING_POLICY == SCHED_RR)
        my_sched_param.sched_priority = 10;
    #else
        my_sched_param.sched_priority = 10;  // custom priority
    #endif
    pthread_attr_setschedparam(&th_attr3, &my_sched_param);



    // Create three threads with different periods, task IDs, and n values
    th1 = pthread_create(&thread1, &th_attr1, &periodic_task, &task1_param);
    th2 = pthread_create(&thread2, &th_attr2, &periodic_task, &task2_param);
    th3 = pthread_create(&thread3, &th_attr3, &periodic_task, &task3_param);

    // Wait for threads to finish
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    pthread_join(thread3, NULL);

    // Destroy thread attributes
    pthread_attr_destroy(&th_attr1);
    pthread_attr_destroy(&th_attr2);
    pthread_attr_destroy(&th_attr3);	

    return 0;
}
