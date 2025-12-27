#include <stdio.h>
#include <sys/neutrino.h>
#include <pthread.h>
#include <sched.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/procmsg.h>
#include <sys/neutrino.h>

pthread_mutex_t mutex =
PTHREAD_MUTEX_INITIALIZER;

char process_name[] = "Client_Server_Process";

typedef struct {

    int buffer[10];
    int buffer_size;
    pthread_cond_t not_empty;   // Signal when data available
    pthread_cond_t not_full;    // Signal when space available
    pthread_mutex_t mutex;

}shared_data;

shared_data shared_buffer = (shared_data){
    .buffer = {0},
    .buffer_size = 0,
    .not_empty = PTHREAD_COND_INITIALIZER,
    .not_full = PTHREAD_COND_INITIALIZER,
    .mutex = PTHREAD_MUTEX_INITIALIZER
};

void* Server_thread(void* arg);
void* Client_thread(void* arg);

int main() {

    int fd = open("/proc/self/status", O_WRONLY);
    if (fd != -1) {
        // Format: "name=ProcessName"
        dprintf(fd, "name=%s\n", process_name);
        close(fd);
        return EXIT_FAILURE;
    }
    printf("Hello QNX!\n");

    //Create a new Server thread
    if (pthread_create(NULL, NULL, Server_thread,NULL) != 0) {
        printf("Failed to create server thread");
        return EXIT_FAILURE;
    }
    //Create a new Client thread
    if (pthread_create(NULL, NULL, Client_thread,NULL) != 0) {
        printf("Failed to create server thread");
        return EXIT_FAILURE;
    }

    sleep(20); // let the threads run


}

void* Server_thread(void* arg) {
    while(1) {
        printf("Server Thread is running...\n");
        

        if (pthread_mutex_lock(&(shared_buffer.mutex)) != 0) {
            printf("Server: Failed to lock mutex");
            return NULL;
        }

        /*Make the conditional wait inside a while loop to avoid spurious wakeups
          This thread will be suspended and the mutex will be released untill other thread 
          Signal or braodcast this condition*/
        while(shared_buffer.buffer_size <= 0)
        {
            pthread_cond_wait(&(shared_buffer.not_empty), &(shared_buffer.mutex));
        }
        printf("Server: Buffer size before processing: %d\n", shared_buffer.buffer_size);
        printf("Server: Processing data...%d\n", shared_buffer.buffer[-- shared_buffer.buffer_size]);
       
        if (-1 == shared_buffer.buffer_size)
        {
            fprintf(stderr, "Server: Buffer underflow!\n");
        }
        else if(shared_buffer.buffer_size < 10)
        {
            printf("send not full signal\n");
            pthread_cond_signal(&(shared_buffer.not_full));
        }
        pthread_mutex_unlock(&(shared_buffer.mutex));

        /*Always the work is done after releasing the mutex to prevent blocking other threads*/
        sleep(4); // Simulate some work greater than the client work time to see the buffer getting full

        
    }
    return NULL;
}

void* Client_thread(void* arg) {
    while(1) {
        printf("Client Thread is running...\n");

        if (pthread_mutex_lock(&(shared_buffer.mutex)) != 0) {
            printf("Client: Failed to lock mutex");
            return NULL;
        }

        /*If the buffer get overflowed, then suspend the client untill the server free some space*/
        while(shared_buffer.buffer_size >= 10)
        {
            fprintf(stderr, "Client: Buffer overflow!\n");
            pthread_cond_wait(&(shared_buffer.not_full), &(shared_buffer.mutex));
        }
        shared_buffer.buffer[shared_buffer.buffer_size ++] = rand() % 100; // Produce random data
        printf("Client: Produced data...%d\n", shared_buffer.buffer[shared_buffer.buffer_size - 1]);
        
        
        if(shared_buffer.buffer_size > 0)
        {
            pthread_cond_signal(&(shared_buffer.not_empty));
        }
        pthread_mutex_unlock(&(shared_buffer.mutex));
        sleep(1); // Simulate some work

        
    }
    return NULL;
}
