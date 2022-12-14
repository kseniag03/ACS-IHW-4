#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
//#include <posix>

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
// agrv[5] -- nails

//написать функцию ввода данных через консоль
void console_input(int *workers, int *nails) {
    printf("Enter number of workers: ");
    scanf("%d", workers);
    printf("Enter number of nails: ");
    scanf("%d", nails);
}

//написать функцию ввода данных через файл
void file_input(int *workers, int *nails, char *fileInputName) {
    FILE *fileInput = fopen(fileInputName, "r");
    if (fileInput == NULL) {
        printf("Error opening file");
        exit(1);
    }
    fscanf(fileInput, "%d", workers);
    fscanf(fileInput, "%d", nails);
    fclose(fileInput);
}

//написать функцию генерации данных
void random_generation(int *workers, int *nails) {
    srand(time(NULL));
    *workers = rand() % 10 + 1;
    *nails = rand() % 100 + 1;
}

void* curvature_check(void *arg) {
    int *nail = (int *) arg;
    printf("Curvature check: %d\n", *nail);
    *nail += 1;
}

void* sharpening(void *arg) {
    int *nail = (int *) arg;
    printf("Sharpening: %d\n", *nail);
    *nail += 1;
}

void* quality_control(void *arg) {
    int *nail = (int *) arg;
    printf("Quality control: %d\n", *nail);
    *nail += 1;
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
            file_input(workers, nails,fileInput);
        } else {
            random_generation(workers, nails);
        }
    } else {
        printf("No arguments\n");
        return 0;
    }

    printf("Input value: %lf, eps: %lf\n", workers, nails);

    pthread_t thread_curvature_check, thread_sharpening, thread_quality_control;
    int NUM_OF_THREADS = 3;
    pthread_t threads[] = {thread_curvature_check, thread_sharpening, thread_quality_control};

    //pthread_create(&thread_curvature_check, NULL, curvature_check, NULL);

    pthread_mutex_init(&mutex, NULL);

    for (int i = 0; i < NUM_OF_THREADS; i++) {
        //pthread_mutex_lock(&mutex);
        //printf("Nail %d is in curvature check\n", i);
        //pthread_mutex_unlock(&mutex);
        pthread_create(&threads[i], NULL, curvature_check, NULL);
    }

    clock_gettime(CLOCK_MONOTONIC, &start);

    //

    clock_gettime(CLOCK_MONOTONIC, &end);
    elapsed_ns = timespec_difference(end, start);
    printf("Elapsed: %ld ns\n", elapsed_ns);
    
    //console_output(result, fileOutput);
    //file_output(result, fileOutput);

    return 0;
}
