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
#include <pthread.h> // Библиотека POSIX Threads

#include <semaphore.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

const int MAX_WORKERS = 10000;
const int MAX_PINS = 100000;

const int NUM_OF_THREADS = 3;

const char *fileInputName = "input.txt";
const char *fileOutputName = "output.txt";



const int bufSize = 10;
int buf[bufSize] ; //буфер
int front = 0 ;    //индекс для чтения из буфера
int rear = 0 ;     //индекс для записи в буфер



typedef struct {
    int isCurve; // if 1 -> won't take
    int quantityLevel; // after sharpening increases
} Pin;

Pin pinBuffer[MAX_PINS];

int in = 0;
int out = 0;

void *producer(void *arg) {
    Pin nextProduced; // следующий генерируемый элемент

    while (1) {
        while (((in + 1) % MAX_PINS) == out) {

        }
        pinBuffer[in] = nextProduced;
        in = (in + 1) % MAX_PINS;
    }
}

void *consumer(void *arg) {
    Pin nextConsumed; // следующий используемый элемент

    while (1) {
        while (in == out) {

        }
        nextConsumed = pinBuffer[out];
        out = (out + 1) % MAX_PINS;
    }
}


pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;


sem_t  empty ; //семафор, отображающий как  буфер пуст
sem_t  full ;  //семафор, отображающий как буфер полон
pthread_mutex_t mutexD ; //мьютекс для операции записи
pthread_mutex_t mutexF ; //мьютекс для операции чтения
unsigned int seed = 101; // инициализатор ГСЧ

//стартовая функция потоков – производителей (писателей)
void *Producer(void *param) {
    int pNum = *((int*)param);
    while (1) {
        //создать элемент для буфера
        int data = rand() % 11 - 5 ;
//поместить элемент в буфер
        pthread_mutex_lock(&mutexD) ; //защита операции записи
        sem_wait(&empty) ; // свободных ячеек уменьшить на 1
        buf[rear] = data ;
        rear = (rear+1)%bufSize ; //критическая секция
        sem_post(&full) ; //занятых ячеек увеличилось на 1
        pthread_mutex_unlock(&mutexD) ;
        printf("Producer %d: Writes value = %d to cell [%d]\n", sleep(2));
    }
    return null;
}

//стартовая функция потоков – потребителей (читателей)
void *Consumer(void *param) {
    int cNum = *((int*)param);
    int result ;
    while (1) {
//извлечь элемент из буфера pthread_mutex_lock(&mutexF) ; //защита чтения //количество занятых ячеек уменьшить на единицу sem_wait(&full) ;
        result = buf[front] ;
        front = (front+1)%bufSize ; //критическая секция //количество свободных ячеек увеличилось на 1 sem_post(&empty) ;
        pthread_mutex_unlock(&mutexF) ;
//обработать полученный элемент
        Printf("Consumer %d: Reads value = %d from cell [%d]\n", cNum, result, front) ;
               , sleep(5));
    }return nullptr; }



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

void console_input(int *workers, int *pins) {
    printf("Enter number of workers: ");
    scanf("%d", workers);
    printf("Enter number of pins: ");
    scanf("%d", pins);
}

int file_input(int *workers, int *pins, const char *fileName) {
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
    if (fscanf(file, "%d", pins) < 1) {
        printf("Reading file '%s' error\n", fileName);
        fclose(file);
        return 1;
    }
    if (*workers > MAX_WORKERS) {
        printf("Number of workers is too big. Max number = %d\n", MAX_WORKERS);
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

void random_generation(int *workers, int *pins) {
    srand(time(NULL));
    *workers = rand() % MAX_WORKERS + 1;
    *pins = rand() % MAX_PINS + 1;
}

// output

void console_output(int cnt, const char *result) {
    printf(result, cnt);
}

void file_output(double res, const char *result, const char *filename) {
    FILE *file;
    if ((file = fopen(filename, "w")) != NULL) {
        fprintf(file, result, res);
        fclose(file);
    }
}

// sequential process (in -> 1 -> 2 -> 3 -> out)

// 1
void* curvature_check(void *arg) {
    Pin *curPin = (Pin *) arg;
    if (curPin->isCurve) {
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
    int option;
    struct timespec start, end;
    int64_t elapsed_ns;
    const char *fileInput, *fileOutput;
    int workers = 0;
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
            workers = atoi(argv[4]);
            pins = atoi(argv[5]);
        } else if (option == 2) {
            console_input(&workers, &pins);
        } else if (option == 3) {
            if (file_input(&workers, &pins, fileInput)) {
                return 1;
            }
        } else {
            random_generation(&workers, &pins);
        }
    } else {
        printf("No arguments\n");
        return 0;
    }

    printf("Input number of workers: %d, number of pins: %d\n", workers, pins);
    
    clock_gettime(CLOCK_MONOTONIC, &start);

    //
    
    int cnt = 0; // number of processed pins (pins)

    pthread_t thread_curvature_check, thread_sharpening, thread_quality_control;
    pthread_t threads[] = {thread_curvature_check, thread_sharpening, thread_quality_control};

    //pthread_create(&thread_curvature_check, NULL, curvature_check, NULL);

    pthread_mutex_init(&mutex, NULL);

    for (int i = 0; i < NUM_OF_THREADS; i++) {
        //pthread_mutex_lock(&mutex);
        //printf("Nail %d is in curvature check\n", i);
        //pthread_mutex_unlock(&mutex);
        pthread_create(&threads[i], NULL, curvature_check, NULL);
        pthread_create(&threads[i], NULL, sharpening, NULL);
       // pthread_create(&threads[i], NULL, quality_control, NULL);
    }
    
    //

    clock_gettime(CLOCK_MONOTONIC, &end);
    elapsed_ns = timespec_difference(end, start);
    printf("Elapsed: %lld ns\n", elapsed_ns);
    
    console_output(cnt, "The final number of processed pins: %d \n");
    file_output(cnt, "The final number of processed pins: %d \n", fileOutput);

    return 0;
}
