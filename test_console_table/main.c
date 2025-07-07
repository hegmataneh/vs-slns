#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>

// Shared data structure
typedef struct {
    int temperature;
    int humidity;
    int pressure;
    int light_level;
    bool running;
    char user_input[100];
    pthread_mutex_t lock;
} SensorData;

// Function to clear the console screen
void clear_screen() {
    printf("\033[H\033[J"); // ANSI escape codes to clear screen
}

// Function to display the table
void display_table(SensorData *data) {
    printf("\n+---------------------+------------+\n");
    printf("| Parameter           | Value      |\n");
    printf("+---------------------+------------+\n");
    printf("| Temperature         | %-10d |\n", data->temperature);
    printf("| Humidity            | %-10d |\n", data->humidity);
    printf("| Pressure            | %-10d |\n", data->pressure);
    printf("| Light Level         | %-10d |\n", data->light_level);
    printf("+---------------------+------------+\n");
    printf("| Last Input          | %-10s |\n", data->user_input);
    printf("+---------------------+------------+\n\n");
    
    printf("Enter command (type 'exit' to quit): ");
    fflush(stdout); // Ensure prompt is displayed
}

// Thread function to simulate sensor data updates
void* sensor_thread(void *arg) {
    SensorData *data = (SensorData *)arg;
    
    while (data->running) {
        // Lock the mutex before updating shared data
        pthread_mutex_lock(&data->lock);
        
        // Simulate changing sensor values
        data->temperature = 20 + rand() % 15;  // 20-35
        data->humidity = 40 + rand() % 40;     // 40-80
        data->pressure = 980 + rand() % 40;    // 980-1020
        data->light_level = rand() % 100;      // 0-100
        
        // Unlock the mutex
        pthread_mutex_unlock(&data->lock);
        
        // Wait for a second before next update
        sleep(1);
    }
    
    return NULL;
}

int main() {
    SensorData data = {
        .temperature = 0,
        .humidity = 0,
        .pressure = 0,
        .light_level = 0,
        .running = true,
        .user_input = {0}
    };
    
    // Initialize mutex
    if (pthread_mutex_init(&data.lock, NULL) != 0) {
        perror("Mutex initialization failed");
        return 1;
    }
    
    // Seed random number generator
    srand(time(NULL));
    
    // Create sensor thread
    pthread_t thread_id;
    if (pthread_create(&thread_id, NULL, sensor_thread, &data) != 0) {
        perror("Thread creation failed");
        return 1;
    }
    
    // Main loop for display and input
    while (data.running) {
        clear_screen();
        
        // Lock mutex before reading shared data
        pthread_mutex_lock(&data.lock);
        display_table(&data);
        pthread_mutex_unlock(&data.lock);
        
        // Get user input
        if (fgets(data.user_input, sizeof(data.user_input), stdin) != NULL) {
            // Remove newline character
            data.user_input[strcspn(data.user_input, "\n")] = '\0';
            
            // Check for exit command
            if (strcmp(data.user_input, "exit") == 0) {
                data.running = false;
            }
        }
    }
    
    // Wait for sensor thread to finish
    pthread_join(thread_id, NULL);
    
    // Clean up mutex
    pthread_mutex_destroy(&data.lock);
    
    printf("Program terminated.\n");
    return 0;
}