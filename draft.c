// http://samzan.net/174085?ysclid=lbovbvu6cb13766593

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h> // Библиотека POSIX Threads

#include <semaphore.h>
#include <unistd.h>

const int MAX_WORKERS = 5;//10000;
const int MAX_PINS = 10;//100000;

const int NUM_OF_THREADS = 3;

const char *fileInputName = "input.txt";
const char *fileOutputName = "output.txt";

int front = 0 ;    // индекс для чтения из буфера
int rear = 0 ;     // индекс для записи в буфер
int count = 0 ; //количество занятых ячеек буфера

typedef struct {
    int isCurve; // if 1 -> won't take
    int quantityLevel; // after sharpening increases
} Pin;

Pin pinBuffer[100000];

int shouldContinue = 0; // flag if pin is straight and we can continue
int improvement = 0; // flag for improving: if > 0 -> improve
int processedPins = 0; // number of processed pins (pins)

sem_t  empty ; //семафор, отображающий как  буфер пуст
sem_t  full ;  //семафор, отображающий как буфер полон
pthread_mutex_t mutexD ; //мьютекс для операции записи
pthread_mutex_t mutexF ; //мьютекс для операции чтения
unsigned int seed = 101; // инициализатор ГСЧ

pthread_mutex_t mutex; // мьютекс для условных переменных
// поток-писатель блокируется этой условной переменной,
// когда количество занятых ячеек становится = bufSize
pthread_cond_t not_full ;
// поток-читатель блокируется этой условной переменной,
// когда количество занятых ячеек становится равно 0
pthread_cond_t not_empty ;


// sequential process (in -> 1 -> 2 -> 3 -> out)

// 1
int curvature_check(Pin pin) {
    printf("Curvature check: ");
    if (!pin.isCurve) {
        printf("Pin is straight\n");
        return 1;
    }
    printf("Pin is curved\n");
    return 0;
}

// 2
int sharpening(Pin *pin) {
    printf("Sharpening: ");
    srand(time(NULL));
    int improve = rand() % 100 + 1;
    pin->quantityLevel += improve;
    printf("Pin is sharpened by %d\n", improve);
    return improve;
}

// 3
void quality_control(Pin pin) {
    printf("Quality control: ");
    if (pin.quantityLevel > 50) {
        printf("Pin is good\n");
        ++processedPins;
    } else {
        printf("Pin is bad\n");
    }
}

//стартовая функция потоков – производителей (писателей) -- генерация булавок и запись их в буфер (входной параметр -- число генерируемых булавок)
void *Producer(void *param) {
    int pNum = *((int*)param);
    int i = pNum;
    while (i--) {
        //создать булавку -- элемент для буфера
        srand(time(NULL));
        int isCurve = rand() % 2;
        int quantityLevel = rand() % 100 + 10;
        Pin nextProduced = { isCurve, quantityLevel };

        //поместить элемент в буфер

        pthread_mutex_lock(&mutex) ; //защита записи

        //заснуть, если количество занятых ячеек равно размеру буфера
        while (count == MAX_PINS ) {
            pthread_cond_wait(&not_full, &mutex);
        }

        //запись в общий буфер
        pinBuffer[rear] = nextProduced;
        rear = (rear + 1) % MAX_PINS;
        count++ ; //появилась занятая ячейка

        //конец критической секции
        pthread_mutex_unlock(&mutex) ;

        //разбудить читателей после добавления в буфер
        pthread_cond_broadcast(&not_empty) ;

        printf("Generated pin with quantity level = %d to cell [%d]\n", nextProduced.quantityLevel, rear);

        sleep(2);
    }
    return NULL;
}

// стартовая функция потоков – потребителей (читателей), проверка булавок на кривизну
void *Consumer_curvature_check(void *param) {
    int cNum = *((int*)param);
    Pin result;

    printf("param consumer curve %d\n", cNum);

    while (1) {
        printf("Consumer %d is waiting for pin\n", cNum);

        //извлечь элемент из буфера
        pthread_mutex_lock(&mutex); //защита чтения

        //заснуть, если количество занятых ячеек равно нулю
        while (count == 0) {
            pthread_cond_wait(&not_empty, &mutex);
        }

        //изъятие из буфера – начало критической секции
        result = pinBuffer[front];
        count-- ; //занятая ячейка стала свободной

        //обработать полученный элемент
        printf("Consumer %d: Reads pin with quantity level = %d from cell [%d]\t", cNum, result.quantityLevel, front);
        printf("pin is %s\n", ((!result.isCurve) ? "straight" : "curved"));

        shouldContinue = curvature_check(result);
        if (shouldContinue) {
            count++ ; //появилась занятая ячейка (возвращаем булавку в буфер, чтобы она не потерялась и другие рабочие могли ее обработать)
        } else {
            front = (front + 1) % MAX_PINS; //критическая секция (двигаемся к следующей булавке, если текущая оказалась кривой)
        }

        // конец критической секции
        pthread_mutex_unlock(&mutex) ;

        // разбудить потоки-писатели

        // после получения элемента из буфера
        pthread_cond_broadcast(&not_full) ;

        sleep(5);
    }
}

//стартовая функция потоков – потребителей (читателей), шлифовка булавок
void *Consumer_sharpening(void *param) {
    int cNum = *((int*)param);
    Pin result;

    printf("param consumer sharp %d\n", cNum);

    while (1) {
        printf("Consumer %d is waiting for pin...\n", cNum);

        //извлечь элемент из буфера
        pthread_mutex_lock(&mutex); //защита чтения

        //заснуть, если количество занятых ячеек = нулю
        while (count == 0) {
            pthread_cond_wait(&not_empty, &mutex);
        }

        //изъятие из буфера – начало критической секции
        result = pinBuffer[front];
        count-- ; //занятая ячейка стала свободной

        //обработать полученный элемент
        printf("Consumer %d: Reads pin with quantity level = %d from cell [%d]\n", cNum, result.quantityLevel, front);


        if (shouldContinue) {
            improvement = sharpening(&pinBuffer[front]);
            count++;
        } else {
            front = (front + 1) % MAX_PINS; //критическая секция
        }

        // конец критической секции
        pthread_mutex_unlock(&mutex) ;

        // разбудить потоки-писатели

        // после получения элемента из буфера
        pthread_cond_broadcast(&not_full) ;

        sleep(5);
    }
}

//стартовая функция потоков – потребителей (читателей), проверка качества булавок
void *Consumer_quality_control(void *param) {
    int cNum = *((int*)param);
    Pin result;

    printf("param consumer quality %d\n", cNum);

    while (1) {
        printf("Consumer %d is waiting for pin...\n", cNum);

        //извлечь элемент из буфера
        pthread_mutex_lock(&mutex); //защита чтения

        //заснуть, если количество занятых ячеек = нулю
        while (count == 0) {
            pthread_cond_wait(&not_empty, &mutex);
        }

        //изъятие из буфера – начало критической секции
        result = pinBuffer[front];

        count-- ; //занятая ячейка стала свободной

        //обработать полученный элемент
        printf("Consumer %d: Reads pin with quantity level = %d from cell [%d]\n", cNum, result.quantityLevel, front);


        if (shouldContinue && improvement > 0) {
            //result.quantityLevel += improve;
            quality_control(pinBuffer[front]);
        }

        shouldContinue = 0;
        improvement = 0;

        front = (front + 1) % MAX_PINS; //критическая секция

        // конец критической секции
        pthread_mutex_unlock(&mutex) ;

        // разбудить потоки-писатели

        // после получения элемента из буфера
        pthread_cond_broadcast(&not_full) ;

        sleep(5);
    }
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

    printf("Input number of workers: %d, number of pins: %d\n\n", workers, pins);

    clock_gettime(CLOCK_MONOTONIC, &start);

    //

    srand(seed);
    int i ;
    //инициализация мьютексов и семафоров
    pthread_mutex_init(&mutex, NULL) ;
    pthread_cond_init(&not_full, NULL) ;
    pthread_cond_init(&not_empty, NULL) ;

    //запуск потребителей
    pthread_t threadC[workers];
    int consumers[workers];
    for (i = 0 ; i < workers ; i++) {
        consumers[i] = i + 1;
        if ((i + 1) % NUM_OF_THREADS == 1) { // curve: 1 4 7 10
            pthread_create(&threadC[i],NULL,Consumer_curvature_check,(void*)(consumers + i));
        } else if ((i + 1) % NUM_OF_THREADS == 2) { // sharp: 2 5 8 11
            pthread_create(&threadC[i],NULL,Consumer_sharpening,(void*)(consumers + i));
        } else { // quality: 3 6 9 12
            pthread_create(&threadC[i],NULL,Consumer_quality_control,(void*)(consumers + i));
        }
    }

    // пусть главный поток будет потоком производителя -- генерация булавок
    Producer((void*)&pins);

    //инициализация мьютексов и семафоров
    pthread_mutex_init(&mutexD, NULL);
    pthread_mutex_init(&mutexF, NULL) ;
    //количество свободных ячеек равно bufSize
    sem_init(&empty, 0, MAX_PINS) ;
    //количество занятых ячеек равно 0
    sem_init(&full, 0, 0) ;

    clock_gettime(CLOCK_MONOTONIC, &end);
    elapsed_ns = timespec_difference(end, start);
    printf("\nElapsed: %lld ns\n", elapsed_ns);

    console_output(processedPins, "The final number of processed pins: %d \n");
    file_output(processedPins, "The final number of processed pins: %d \n", fileOutput);

    return 0;
}
