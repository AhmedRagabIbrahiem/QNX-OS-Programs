/*
 * Additional benchmark focusing on cache effects
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/neutrino.h>
#include <time.h>
#include <sched.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/syspage.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>

#define ARRAY_SIZE 10000  // Reduced for better cache fit
#define ITERATIONS 100

int shared_array_0[ARRAY_SIZE];
int shared_array_1[ARRAY_SIZE];

typedef struct {
    int id;
    int cpu_mask;
    char* name;
    double time;
} bench_data_t;

void* cache_test_thread_0(void* arg) {
    bench_data_t* data = (bench_data_t*)arg;
    pthread_setname_np(pthread_self(), data->name);
    
    // Set CPU affinity
    if (data->cpu_mask) {
        if (ThreadCtl(_NTO_TCTL_RUNMASK, (void*)(uintptr_t)data->cpu_mask) == -1) {
            perror("ThreadCtl failed");
            return NULL;
        }
    }
    
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    // Add volatile to prevent compiler optimizations
    volatile int sum = 0;
    for (int iter = 0; iter < ITERATIONS; iter++) {
        for (int i = 0; i < ARRAY_SIZE; i++) {
            shared_array_0[i] = i * iter;
            sum += shared_array_0[i];
        }
    }
    
    clock_gettime(CLOCK_MONOTONIC, &end);
    
    data->time = (end.tv_sec - start.tv_sec) + 
                (end.tv_nsec - start.tv_nsec) / 1e9;
    
    return (void*)(intptr_t)sum;
}

void* cache_test_thread_1(void* arg) {
    bench_data_t* data = (bench_data_t*)arg;
    pthread_setname_np(pthread_self(), data->name);
    
    // Set CPU affinity
    if (data->cpu_mask) {
        if (ThreadCtl(_NTO_TCTL_RUNMASK, (void*)(uintptr_t)data->cpu_mask) == -1) {
            perror("ThreadCtl failed");
            return NULL;
        }
    }
    
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    volatile int sum = 0;
    for (int iter = 0; iter < ITERATIONS; iter++) {
        for (int i = 0; i < ARRAY_SIZE; i++) {
            // Read before write to create more contention
            sum += shared_array_0[i];
            shared_array_0[i] = i * iter;
        }
    }
    
    clock_gettime(CLOCK_MONOTONIC, &end);
    
    data->time = (end.tv_sec - start.tv_sec) + 
                (end.tv_nsec - start.tv_nsec) / 1e9;
    
    return (void*)(intptr_t)sum;
}

void* cache_test_thread_2(void* arg) {
    bench_data_t* data = (bench_data_t*)arg;
    pthread_setname_np(pthread_self(), data->name);
    
    if (data->cpu_mask) {
        if (ThreadCtl(_NTO_TCTL_RUNMASK, (void*)(uintptr_t)data->cpu_mask) == -1) {
            perror("ThreadCtl failed");
            return NULL;
        }
    }
    
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    volatile int sum = 0;
    for (int iter = 0; iter < ITERATIONS; iter++) {
        for (int i = 0; i < ARRAY_SIZE; i++) {
            shared_array_1[i] = i * iter;
            sum += shared_array_1[i];
        }
    }
    
    clock_gettime(CLOCK_MONOTONIC, &end);
    
    data->time = (end.tv_sec - start.tv_sec) + 
                (end.tv_nsec - start.tv_nsec) / 1e9;
    
    return (void*)(intptr_t)sum;
}

void* cache_test_thread_3(void* arg) {
    bench_data_t* data = (bench_data_t*)arg;
    pthread_setname_np(pthread_self(), data->name);
    
    if (data->cpu_mask) {
        if (ThreadCtl(_NTO_TCTL_RUNMASK, (void*)(uintptr_t)data->cpu_mask) == -1) {
            perror("ThreadCtl failed");
            return NULL;
        }
    }
    
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    volatile int sum = 0;
    for (int iter = 0; iter < ITERATIONS; iter++) {
        for (int i = 0; i < ARRAY_SIZE; i++) {
            sum += shared_array_1[i];
            shared_array_1[i] = i * iter;
        }
    }
    
    clock_gettime(CLOCK_MONOTONIC, &end);
    
    data->time = (end.tv_sec - start.tv_sec) + 
                (end.tv_nsec - start.tv_nsec) / 1e9;
    
    return (void*)(intptr_t)sum;
}

void run_cache_benchmark(int test_case) {
    const int num_threads = 4;
    pthread_t threads[num_threads];
    bench_data_t data[num_threads];
    
    printf("\n=== Cache Locality Benchmark - Test Case %d ===\n", test_case);
    
    // Initialize arrays
    memset(shared_array_0, 0, sizeof(shared_array_0));
    memset(shared_array_1, 0, sizeof(shared_array_1));
    
    // Configure based on test case
    int cpu_masks[4];
    switch(test_case) {
        case 1:  // Good locality - sharing threads on same CPU
            // Threads 0 & 1 (share array_0) on CPU 0
            // Threads 2 & 3 (share array_1) on CPU 1
            cpu_masks[0] = 0x01; cpu_masks[1] = 0x01;
            cpu_masks[2] = 0x02; cpu_masks[3] = 0x02;
            break;
        case 2:  // Poor locality - sharing threads on different CPUs
            // Threads 0 & 1 (share array_0) on different CPUs
            // Threads 2 & 3 (share array_1) on different CPUs
            cpu_masks[0] = 0x01; cpu_masks[1] = 0x02;
            cpu_masks[2] = 0x01; cpu_masks[3] = 0x02;
            break;
    }
    
    // Create threads
    void* (*thread_functions[4]) (void*) = {
        cache_test_thread_0,
        cache_test_thread_1,
        cache_test_thread_2,
        cache_test_thread_3
    };
    
    for (int i = 0; i < num_threads; i++) {
        data[i].id = i;
        data[i].cpu_mask = cpu_masks[i];
        data[i].name = (i < 2) ? "CacheTest_0" : "CacheTest_1";
        pthread_create(&threads[i], NULL, thread_functions[i], &data[i]);
    }
    
    // Wait for completion
    double total_time = 0;
    for (int i = 0; i < num_threads; i++) {
        void* result;
        pthread_join(threads[i], &result);
        total_time += data[i].time;
        printf("Thread %d (mask 0x%02x): %.3f seconds\n", 
               i, data[i].cpu_mask, data[i].time);
    }
    
    printf("Total time: %.3f seconds\n", total_time);
    printf("Average per thread: %.3f seconds\n", total_time / num_threads);
    
    // Calculate speedup
    static double good_locality_time = 0;
    if (test_case == 1) {
        good_locality_time = total_time;
    } else if (test_case == 2 && good_locality_time > 0) {
        double speedup = good_locality_time / total_time;
        printf("Speedup (good/poor): %.2fx\n", speedup);
    }
}

int main() {
    int num_cpus = _syspage_ptr->num_cpu;
    printf("System has %d CPUs\n", num_cpus);
    printf("Cache Locality Affinity Benchmark\n");
    printf("=================================\n\n");
    
    if (num_cpus < 2) {
        printf("Need at least 2 CPUs for this test!\n");
        return 1;
    }
    
    // Warm up
    printf("Warming up...\n");
    for (int i = 0; i < ARRAY_SIZE; i++) {
        shared_array_0[i] = i;
        shared_array_1[i] = i;
    }
    
   

    // Run tests
    printf("\nTest 1: Sharing threads on same CPU (good cache sharing)\n");
    run_cache_benchmark(1);
    
    sleep(0.5);  // Let system settle

    printf("\nTest 2: Sharing threads on different CPUs (poor cache sharing)\n");
    run_cache_benchmark(2);

    
    
    
    
    return 0;
}