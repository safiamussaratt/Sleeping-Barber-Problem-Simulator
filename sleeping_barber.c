# sleeping barber problem simulation with priority customers and enhanced logging

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <stdarg.h>  
#include <time.h>

#define NUM_CHAIRS 3  // Smaller number for better visualization
#define SIMULATION_SPEED 0.5  // Speed multiplier (lower is faster)
#define FILE_NAME "log.txt"
#define FIFO_PATH "/tmp/prodcon_fifo"
#define BUFFER_SIZE 256

sem_t customers;
sem_t barber;
sem_t mutex;    
sem_t print_mutex;  

int customers_served = 0;
int customers_left = 0;

// Customer structure to hold customer ID and priority
typedef struct {
    int id;
    int priority;
} Customer;

Customer waiting_customers[NUM_CHAIRS];
int waiting_count = 0;

void print_status(const char *format, ...) {
    sem_wait(&print_mutex);
   
    va_list args;
    va_start(args, format);
    vprintf(format, args);  
    va_end(args);

    system("clear");
    printf("\n|   Barber Shop Status   |\n");
    printf("|========================|\n");
    printf("| Customers waiting: %d/%d |\n", waiting_count, NUM_CHAIRS);
    printf("| Customers served: %d\t |\n", customers_served);
    printf("| Customers left: %d\t |\n", customers_left);
    printf("|========================|\n");

   
    sem_post(&print_mutex);
}

void* barber_thread(void* arg) {
    while (1) {
        print_status("Barber: Checking for customers...");
        sem_wait(&customers);

        if (waiting_count == 0) {
            print_status("Barber: No customers, waiting...");
        }

        if (waiting_count == 0) break;

        sem_wait(&mutex);
        // Find the highest priority customer (lowest priority number)
        int index = 0;
        for (int i = 1; i < waiting_count; i++) {
            if (waiting_customers[i].priority < waiting_customers[index].priority) {
                index = i;
            }
        }
        Customer chosen = waiting_customers[index];

        // Remove customer from waiting list
        for (int i = index; i < waiting_count - 1; i++) {
            waiting_customers[i] = waiting_customers[i + 1];
        }
        waiting_count--;

        customers_served++;
        sem_post(&barber);
        sem_post(&mutex);

        print_status("Barber: Cutting hair of Customer %d (Priority %d)...", chosen.id, chosen.priority);
        sleep(2 / SIMULATION_SPEED);
        print_status("Barber: Finished haircut of Customer %d", chosen.id);
    }

    print_status("Barber: Shop closed, going home");
    return NULL;
}

void* customer_thread(void* arg) {
    Customer* info = (Customer*)arg;
    int id = info->id;
    int priority = info->priority;

    sem_wait(&mutex);
    if (waiting_count < NUM_CHAIRS) {
        waiting_customers[waiting_count].id = id;
        waiting_customers[waiting_count].priority = priority;
        waiting_count++;

        print_status("Customer %d (Priority %d): Arrived and sitting down", id, priority);
        sem_post(&customers);
        sem_post(&mutex);

        sem_wait(&barber);
        // Haircut happens here
    } else {
        customers_left++;
        print_status("Customer %d (Priority %d): No chairs available, leaving", id, priority);
        sem_post(&mutex);
    }

    free(info);
    return NULL;
}

void run_test_case(int test_case) {
    int fd = open(FIFO_PATH, O_WRONLY);
    if (fd < 0) {
        perror("Producer: open");
        exit(1);
    }

    pthread_t barber_tid;
    pthread_t customer_tids[100];  
    int customer_id = 0;

    sem_init(&customers, 0, 0);
    sem_init(&barber, 0, 0);
    sem_init(&mutex, 0, 1);
    sem_init(&print_mutex, 0, 1);

    customers_served = 0;
    customers_left = 0;
    waiting_count = 0;

    pthread_create(&barber_tid, NULL, barber_thread, NULL);
    printf("\n=== Starting Test Case %d ===\n", test_case);

    switch(test_case) {
        case 1: // No customers
            print_status("\nTest 1: No customers will arrive\n");
            sleep(5/SIMULATION_SPEED);
            break;
       
        case 2: // Single customer
            print_status("\nTest 2: One customer will arrive\n");
            {
                int *id = malloc(sizeof(int));
                *id = ++customer_id;
                Customer* info = malloc(sizeof(Customer));
                info->id = customer_id;
                info->priority = rand() % 3;

                pthread_create(&customer_tids[0], NULL, customer_thread, info);
                sleep(2/SIMULATION_SPEED);
            }
            break;
       
        case 3: // Multiple customers with available chairs
            print_status("\nTest 3: 3 customers arriving with 3 chairs\n");
            for (int i = 0; i < 3; i++) {
                int *id = malloc(sizeof(int));
                *id = ++customer_id;
                Customer* info = malloc(sizeof(Customer));
                info->id = customer_id;
                info->priority = rand() % 3;

                pthread_create(&customer_tids[i], NULL, customer_thread, info);
                sleep(1/SIMULATION_SPEED);
            }
            sleep(8/SIMULATION_SPEED);
            break;
       
        case 4: // All chairs occupied
            print_status("\nTest 4: 5 customers arriving with 3 chairs\n");
            for (int i = 0; i < 5; i++) {
                int *id = malloc(sizeof(int));
                *id = ++customer_id;
                Customer* info = malloc(sizeof(Customer));
                info->id = customer_id;
                info->priority = rand() % 3;

                pthread_create(&customer_tids[i], NULL, customer_thread, info);
                sleep(1/SIMULATION_SPEED);
            }
            sleep(10/SIMULATION_SPEED);
            break;
       
        case 5: // Continuous stream
            print_status("\nTest 5: Continuous customers for 15 seconds\n");
            {
                time_t start = time(NULL);
                while (time(NULL) - start < 15/SIMULATION_SPEED) {
                    int *id = malloc(sizeof(int));
                    *id = ++customer_id;
                    Customer* info = malloc(sizeof(Customer));
                    info->id = customer_id;
                    info->priority = rand() % 3;

                    pthread_create(&customer_tids[customer_id - 1], NULL, customer_thread, info);
                    sleep((1 + rand() % 3)/SIMULATION_SPEED);
                }
                sleep(5/SIMULATION_SPEED);
            }
            break;

	case 6: // Customer Number Input
            print_status("Test 6: Input number of customers");
            {
                int num_customers;
                printf("Enter number of customers: ");
                scanf("%d", &num_customers);

                for (int i = 0; i < num_customers; i++) {
                    int *id = malloc(sizeof(int));
                    *id = ++customer_id;
                    Customer* info = malloc(sizeof(Customer));
                    info->id = customer_id;
                    info->priority = rand() % 3;

                    pthread_create(&customer_tids[i], NULL, customer_thread, info);
                    sleep(1/SIMULATION_SPEED);
                }
                sleep(10/SIMULATION_SPEED);
            }
            break;
    }

    // Close the shop
    sem_post(&customers);  // Wake barber to exit

    pthread_join(barber_tid, NULL);
    for (int i = 0; i < customer_id; i++) {
        pthread_join(customer_tids[i], NULL);
    }

    sem_destroy(&customers);
    sem_destroy(&barber);
    sem_destroy(&mutex);
    sem_destroy(&print_mutex);

    // Open file in append mode to avoid overwriting
    FILE *log_file = fopen(FILE_NAME, "a");
    if (!log_file) {
        perror("Failed to open log file");
        exit(1);
    }

    fprintf(log_file, "\n=== Test Case Completed: %d ===\n", test_case);
    fprintf(log_file, "Total customers served: %d\n", customers_served);
    fprintf(log_file, "Total customers left: %d\n", customers_left);
    fclose(log_file);
}

void run_producer() {
    int choice;    

    do {
        printf("\n  Sleeping Barber Problem - Test Cases\n");
        printf("----------------------------------------\n");
        printf("1. No customers arrive\n");
        printf("2. One customer arrives\n");
        printf("3. Multiple customers (chairs available)\n");
        printf("4. All chairs occupied\n");
        printf("5. Continuous stream of customers\n");
        printf("6. Input based\n");
        printf("7. See data log\n");
        printf("0. Exit\n");
        printf("----------------------------------------\n");
        printf("Enter test case number: ");
        scanf("%d", &choice);

        if (choice >= 1 && choice <= 6) {
            run_test_case(choice);
        } else if (choice == 7) {
            printf("\nReading data log\n");
            system("clear");

            FILE *log = fopen(FILE_NAME, "r");
            if (!log) {
                 perror("Consumer: fopen log.txt");
                 exit(1);
            }

           char line[256];  
           while (fgets(line, sizeof(line), log)) {
                 printf("%s", line);
           }

           fclose(log);
     
        } else if (choice != 0) {
            printf("Invalid choice. Please try again.\n");
        }
    } while (choice != 0);
   
    printf("Exiting program...\n");

    return;
}

void run_consumer() {
    int fd = open(FIFO_PATH, O_RDONLY);
    if (fd < 0) {
        perror("Consumer: open");
        exit(1);
    }

    FILE *log = fopen("log.txt", "a");  
    if (!log) {
        perror("Consumer: fopen log.txt");
        exit(1);
    }

    time_t now = time(NULL);
    struct tm *local = localtime(&now);
    fprintf(log, "Log time: %02d-%02d-%04d %02d:%02d:%02d\n",
            local->tm_mday,
            local->tm_mon + 1,
            local->tm_year + 1900,
            local->tm_hour,
            local->tm_min,
            local->tm_sec);
    fflush(log);

    char buffer[BUFFER_SIZE];
    int bytes_read;

    while ((bytes_read = read(fd, buffer, sizeof(buffer)-1)) > 0) {
        buffer[bytes_read] = '\0';
        fprintf(log, "%s", buffer);
        fflush(log);
    }

    fclose(log);
    close(fd);
    exit(0);
}

int main() {
    if (mkfifo(FIFO_PATH, 0666) == -1) {
        perror("mkfifo");
        // If it already exists, continue
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        exit(1);
    }

    if (pid == 0) {
        run_producer();
    } else {
        pid_t pid2 = fork();
        if (pid2 < 0) {
            perror("fork failed");
            exit(1);
        }

        if (pid2 == 0) {
            run_consumer();
        } else {
            wait(NULL); // Wait for producer
            wait(NULL); // Wait for consumer
            unlink(FIFO_PATH);  // Clean up the pipe
            printf("Producer and Consumer done. Log written to log.txt\n");
        }
    }

    return 0;
}