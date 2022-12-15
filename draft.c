// http://samzan.net/174085?ysclid=lbovbvu6cb13766593

/*
Реализация представления буфера на языке Си может иметь вид:
#define BUFFER_SIZE 1000

typedef struct {

. . .

} item;

item buffer[BUFFER_SIZE];

int in = 0;

int out = 0;


Реализация схемы алгоритма процесса-производителя имеет вид:
item nextProduced; // следующий генерируемый элемент

 while (1) { // бесконечный цикл

while (((in + 1) % BUFFER_SIZE) == out)

 ; // ждать, пока буфер переполнен

buffer[in] = nextProduced; // генерация элемента

in = (in + 1) % BUFFER_SIZE;

}

Соответственно, реализация процесса-потребителя будет иметь вид:
item nextConsumed; // следующий используемый элемент

while (1) { // бесконечный цикл

while (in == out)

 ; // ждать, пока буфер пуст

nextConsumed = buffer[out]; // использование элемента

out = (out + 1) % BUFFER_SIZE;

}

*/

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

typedef struct {
    bool isCurve; // if true -> won't take
    int quantityLevel; // after sharpering increases
} pin;

int MAX_WORKERS = 10000;
int MAX_PINS = 100000;

int NUM_OF_THREADS = 3;

char *fileInputName = "input.txt";
char *fileOutputName = "output.txt";

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

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

// написать многопоточное приложение, моделирующее работу цеха с передачей булавок тремя рабочими
// для трёх разных процессов (проверка кривизны, заточка, контроль качества)
// в качестве входных данных принимать число рабочих цеха и число булавок, которые они должны обработать
// булавки передаются по одной штуке между рабочими

// приложение должно выводить на экран информацию о том,
// какая булавка находится у какого рабочего в данный момент времени

// option = 1 -- command line console_input
// option = 2 -- console console_input
// option = 3 -- file console_input
// option = else -- random generation

// argv[0] -- exe file name
// agrv[1] -- option
// agrv[2] -- fileInputName
// agrv[3] -- fileOutputName
// agrv[4] -- workers
// agrv[5] -- pins

// input

void console_input(int *workers, int *nails) {
    printf("Enter number of workers: ");
    scanf("%d", workers);
    printf("Enter number of nails: ");
    scanf("%d", nails);
}

int file_input(int *workers, int *nails, char *fileName) {
    FILE *file;
    if ((file = fopen(fileName, "r")) == NULL) {
        printf("Unable to open file '%s'\n", fileName);
        return 1;
    }
    if (fscanf(file, "%d", workers) < 1) {
        printf ("Reading file '%s' error\n", fileName);
        fclose(file);
        return 1;
    }
    if (fscanf(file, "%d", nails) < 1) {
        printf("Reading file '%s' error\n", fileName);
        fclose(file);
        return 1;
    }
    if (*workers > MAX_WORKERS) {
        printf("Number of workers is too big. Max number = %d\n", MAX_WORKERS);
        fclose(file);
        return 1;
    }
    if (*nails > MAX_PINS) {
        printf("Number of pins is too big. Max number = %d\n", MAX_PINS);
        fclose(file);
        return 1;
    }
    fclose(file);
    return 0;
}

void random_generation(int *workers, int *nails) {
    srand(time(NULL));
    *workers = rand() % MAX_WORKERS + 1;
    *nails = rand() % MAX_PINS + 1;
}

// output

void console_output(int result) {
    printf("result: %d\n", result);
}

void file_output(double res, const char *result, char *filename) {
    FILE *file;
    if ((file = fopen(filename, "w")) != NULL) {
        fprintf(file, result, res);
        fclose(file);
    }
}

// sequential process (in -> 1 -> 2 -> 3 -> out)

// 1
void* curvature_check(void *arg) {
    pin curPin = (pin) arg;
    if (pin.isCurve) {
        return; // next
    }
    
    // give right to next worker
    
    int *nail = (int *) arg;
    printf("Curvature check: %d\n", *nail);
    *nail += 1;
}

// 2
void* sharpening(void *arg) {
    int *nail = (int *) arg;
    printf("Sharpening: %d\n", *nail);
    *nail += 1;    
}

// 3
void* quality_control(void *arg, int* cnt) {
    int *nail = (int *) arg;
    printf("Quality control: %d\n", *nail);
    *nail += 1;
    // one more was processed
    *cnt += 1;
}

int main(int argc, char** argv) {
    char *arg;
    int option;
    struct timespec start, end;
    int64_t elapsed_ns;
    char *fileInput, *fileOutput;
    int workers = 0;
    int nails = 0;

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
            workers = atoi(argv[4]);
            nails = atoi(argv[5]);
        } else if (option == 2) {
            console_input(workers, nails);
        } else if (option == 3) {
            if (file_input(workers, nails,fileInput)) {
                return 1;
            }
        } else {
            random_generation(workers, nails);
        }
    } else {
        printf("No arguments\n");
        return 0;
    }

    printf("Input number of workers: %d, number of pins: %d\n", workers, nails);
    
    clock_gettime(CLOCK_MONOTONIC, &start);

    //
    
    int cnt = 0; // number of processed pins (nails)

    pthread_t thread_curvature_check, thread_sharpening, thread_quality_control;
    pthread_t threads[] = {thread_curvature_check, thread_sharpening, thread_quality_control};

    //pthread_create(&thread_curvature_check, NULL, curvature_check, NULL);

    pthread_mutex_init(&mutex, NULL);

    for (int i = 0; i < NUM_OF_THREADS; i++) {
        //pthread_mutex_lock(&mutex);
        //printf("Nail %d is in curvature check\n", i);
        //pthread_mutex_unlock(&mutex);
        pthread_create(&threads[i], NULL, curvature_check, NULL);
    }
    
    //

    clock_gettime(CLOCK_MONOTONIC, &end);
    elapsed_ns = timespec_difference(end, start);
    printf("Elapsed: %ld ns\n", elapsed_ns);
    
    console_output(result, fileOutput);
    file_output(result, fileOutput);

    return 0;
}
