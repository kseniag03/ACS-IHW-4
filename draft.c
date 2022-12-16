#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

const int MAX_PINS = 100;
const int NUM_OF_THREADS = 3;

const char *fileInputName = "input.txt";
const char *fileOutputName = "output.txt";

int readI = 0;
int writeI = 0;
int count = 0;

typedef struct {
    int isCurve; // if 1 -> won't take
    int quantityLevel; // after sharpening increases
} Pin;

Pin pinBuffer[100000];

int shouldContinue = 0;
int improvement = 0;
int processedPins = 0;

sem_t  empty; // buffer is empty
sem_t  full; // buffer is full

pthread_mutex_t mutex; // мьютекс для условных переменных
// поток-писатель блокируется этой условной переменной,
// когда количество занятых ячеек становится = bufSize
pthread_cond_t not_full ;
// поток-читатель блокируется этой условной переменной,
// когда количество занятых ячеек становится равно 0
pthread_cond_t not_empty ;

int curvature_check(Pin pin) {
    printf("Curvature check: ");
    if (!pin.isCurve) {
        printf("Pin is straight\n");
        return 1;
    }
    printf("Pin is curved\n");
    return 0;
}

int sharpening(Pin *pin) {
    printf("Sharpening: ");
    srand(time(NULL));
    int improve = rand() % 100 + 1;
    pin->quantityLevel += improve;
    printf("Pin is sharpened by %d\n", improve);
    return improve;
}

void quality_control(Pin pin) {
    printf("Quality control: ");
    if (pin.quantityLevel > 50) {
        printf("Pin is good\n");
        ++processedPins;
    } else {
        printf("Pin is bad\n");
    }
}

void *Producer(void *param) {
    int pNum = *((int*)param);
    int i = pNum;
    while (i--) {
        srand(time(NULL));
        int isCurve = rand() % 2;
        int quantityLevel = rand() % 100 + 10;
        Pin nextProduced = { isCurve, quantityLevel };
        // writing protection
        pthread_mutex_lock(&mutex);
        // wait until buffer is not full
        while (count == MAX_PINS ) {
            pthread_cond_wait(&not_full, &mutex);
        }
        pinBuffer[writeI] = nextProduced;
        printf("Generated pin with quantity level = %d to cell [%d]\n", nextProduced.quantityLevel, writeI);
        writeI = (writeI + 1) % MAX_PINS;
        count++;
        pthread_mutex_unlock(&mutex);
        // wake up readers
        pthread_cond_broadcast(&not_empty);
        sleep(10);
    }
    return NULL;
}

void *Consumer_curvature_check(void *param) {
    int cur = *((int*)param);
    Pin result;
    while (1) {
        // reading protection
        pthread_mutex_lock(&mutex);
        // wait until buffer is not empty
        while (count == 0) {
            printf("Consumer %d is waiting for pin\n", cur);
            printf("...");
            pthread_cond_wait(&not_empty, &mutex);
        }
        result = pinBuffer[readI];
        printf("Consumer %d: Reads pin with quantity level = %d from cell [%d]\t", cur, result.quantityLevel, readI);
        printf("pin is %s\n", ((!result.isCurve) ? "straight" : "curved"));
        shouldContinue = curvature_check(result);
        if (!shouldContinue) {
            count-- ;
            readI = (readI + 1) % MAX_PINS;
        }
        pthread_mutex_unlock(&mutex);
        // wake up writers
        pthread_cond_broadcast(&not_full);
        sleep(2);
    }
}

void *Consumer_sharpening(void *param) {
    int cur = *((int*)param);
    Pin result;
    while (1) {
        // reading protection
        pthread_mutex_lock(&mutex);
        // wait until buffer is not empty
        while (count == 0) {
            printf("Consumer %d is waiting for pin...\n", cur);
            printf("...");
            pthread_cond_wait(&not_empty, &mutex);
        }
        if (shouldContinue) {
            result = pinBuffer[readI];
            printf("Consumer %d: Reads pin with quantity level = %d from cell [%d]\n", cur, result.quantityLevel, readI);
            improvement = sharpening(&pinBuffer[readI]);
            printf("Consumer %d: new quantity level = %d from cell [%d]\n", cur, result.quantityLevel, readI);
        }
        pthread_mutex_unlock(&mutex);
        // wake up writers
        pthread_cond_broadcast(&not_full);
        sleep(2);
    }
}

void *Consumer_quality_control(void *param) {
    int cur = *((int*)param);
    Pin result;
    while (1) {
        // reading protection
        pthread_mutex_lock(&mutex);
        // wait until buffer is not empty
        while (count == 0) {
            printf("Consumer %d is waiting for pin...\n", cur);
            printf("...");
            pthread_cond_wait(&not_empty, &mutex);
        }
        if (shouldContinue && improvement > 0) {
            result = pinBuffer[readI];
            count--;
            printf("Consumer %d: Reads pin with quantity level = %d from cell [%d]\n", cur, result.quantityLevel, readI);
            quality_control(pinBuffer[readI]);
            shouldContinue = 0;
            improvement = 0; // make flags zero
            readI = (readI + 1) % MAX_PINS;
        }
        pthread_mutex_unlock(&mutex);
        // wake up writers
        pthread_cond_broadcast(&not_full);
        sleep(2);
    }
}

// option = 1 -- command_line_input
// option = 2 -- console_input
// option = 3 -- file_input
// option = else -- random generation

// argv[0] -- exe file name
// agrv[1] -- option
// agrv[2] -- fileInputName
// agrv[3] -- fileOutputName
// agrv[4] -- pins

// input

void console_input(int *pins) {
    printf("Enter number of pins: ");
    scanf("%d", pins);
}

int file_input(int *pins, const char *fileName) {
    FILE *file;
    if ((file = fopen(fileName, "r")) == NULL) {
        printf("Unable to open file '%s'\n", fileName);
        return 1;
    }
    if (fscanf(file, "%d", pins) < 1) {
        printf("Reading file '%s' error\n", fileName);
        fclose(file);
        return 1;
    }
    if (*pins > MAX_PINS) {
        printf("Number of pins is too big. Max number = %d\n", MAX_PINS);
        fclose(file);
        return 1;
    }
    fclose(file);
    return 0;
}

void random_generation(int *pins) {
    srand(time(NULL));
    *pins = rand() % MAX_PINS + 1;
}

// output

void console_output(int cnt, const char *result) {
    printf(result, cnt);
}

void file_output(int cnt, const char *result, const char *filename) {
    FILE *file;
    if ((file = fopen(filename, "w")) != NULL) {
        fprintf(file, result, cnt);
        fclose(file);
    }
}

// time counter

int64_t timespec_difference(struct timespec a, struct timespec b) {
    int64_t timeA, timeB;
    timeA = a.tv_sec;
    timeA *= 1000000000;
    timeA += a.tv_nsec;
    timeB = b.tv_sec;
    timeB *= 1000000000;
    timeB += b.tv_nsec;
    return timeA - timeB;
}

int main(int argc, char** argv) {
    int option;
    struct timespec start, end;
    int64_t elapsed_ns;
    const char *fileInput, *fileOutput;
    int pins = 0;

    if (argc > 1) {
        if (argc <= 2) {
            fileInput = fileInputName;
        } else {
            fileInput = argv[2];
        }
        if (argc <= 3) {
            fileOutput = fileOutputName;
        } else {
            fileOutput = argv[3];
        }
        option = atoi(argv[1]);
        if (option == 1) {
            pins = atoi(argv[4]);
        } else if (option == 2) {
            console_input(&pins);
        } else if (option == 3) {
            if (file_input(&pins, fileInput)) {
                return 1;
            }
        } else {
            random_generation(&pins);
        }
    } else {
        printf("No arguments\n");
        return 0;
    }
    printf("Input number of pins: %d\n\n", pins);

    clock_gettime(CLOCK_MONOTONIC, &start);

    srand(time(NULL));
    int i ;
    // init mutex and condition variables
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&not_full, NULL);
    pthread_cond_init(&not_empty, NULL);
    // init mutex and semaphores
    sem_init(&empty, 0, MAX_PINS);
    sem_init(&full, 0, 0);
    // consumer threads creation
    pthread_t threadC[3];
    int consumers[3];
    for (i = 0 ; i < 3 ; i++) {
        consumers[i] = i + 1;
        if ((i + 1) % NUM_OF_THREADS == 1) { // curve: 1 4 7 10...
            pthread_create(&threadC[i],NULL,Consumer_curvature_check,(void*)(consumers + i));
        } else if ((i + 1) % NUM_OF_THREADS == 2) { // sharp: 2 5 8 11...
            pthread_create(&threadC[i],NULL,Consumer_sharpening,(void*)(consumers + i));
        } else { // quality: 3 6 9 12...
            pthread_create(&threadC[i],NULL,Consumer_quality_control,(void*)(consumers + i));
        }
    }
    // main thread is producer
    Producer((void*)&pins);

    clock_gettime(CLOCK_MONOTONIC, &end);
    elapsed_ns = timespec_difference(end, start);
    printf("\nElapsed: %lld ns\n", elapsed_ns);
    console_output(processedPins, "The final number of processed pins: %d \n");
    file_output(processedPins, "The final number of processed pins: %d \n", fileOutput);
    return 0;
}
