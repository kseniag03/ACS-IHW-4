#  ИДЗ №4 #
## Markdown report <br> ##

### 1. Ганина Ксения Андреевна (тг для вопросов: @kgnn47) <br> ###
### 2. БПИ212 <br> ###
### 3. Вариант-25 <br> ###

![image](https://user-images.githubusercontent.com/114473740/206900139-ecd4bc8e-c9e7-43e7-b7ea-c45c612c7d83.png) <br>
________________________

### 4. Используемая модель вычислений. Источники информации, в которых описана данная модель. <br> ###

Передача производится по одной булавке.

Производители и потребители -- это парадигма взаимодействующих неравноправных потоков: одни потоки генерируют данные, другие с ними работают. <br>
Зачастую подобные потоки обращаются к одному конвейеру, через который проходит информация. <br>

Задача о кольцевом буфере <br>
Потоки производители и потребители разделяют кольцевой буфер (массив структур, представляющих булавку из MAX_COUNT элементов. У булавки два поля: для проверки кривизны и уровня качества). Производители передают сообщение потребителям, помещая его в конец очереди буфера. Потребители сообщение извлекают из начала очереди буфера. Создается многопоточное приложение с потоками "писатели" и "читатели". В данной задаче один писатель (генератор булавок) и три читателя (которые последовательно обрабатывают каждую булавку). Запись должна производиться в прочитанные ячейки. Для защиты потока писателей и потока читателей используем мьютекс. Также учитываем условные переменные (на случай, если буфер с булавками переполнится). <br>

Источник: [Задача о кольцевом буфере](https://edu.hse.ru/pluginfile.php/1867463/mod_resource/content/3/08-Multitreading.pdf) <br>
________________________

### 5. Описание работы с данными программы. <br> ###

Формат ввода данных <br>
argc -- число аргументов в функции (если > 1, значит, передали аргументы) <br>
argv -- массив с аргументами, где: <br>
argv[0] -- имя исполняемого файла <br>
argv[1] -- формат ввода (1 -- command_line_input, 2 -- console_input, 3 -- file_input, else -- random generation) <br>
argv[2] -- имя входного файла (если не задан, то по умолчанию input.txt) <br>
argv[3] -- имя выходного файла (если не задан, то по умолчанию output.txt) <br>
agrv[4] -- pins (число булавок для генерации) <br>

Ответ на задание выводится в консоль и в выходной файл <br>

Получая на вход булавку 1-й рабочий проверяет св-во кривизны -> передаёт в очередь для проверки 2-м рабочим или не смещает индекс буфера (если не кривая). Если кривая -- просто смещается индекс буфера и проверяется следующая булавка. <br>
2-й рабочий на псевдослучайное число повышает уровень качества булавки и передаёт 3-му. <br>
3-й рабочий смотрит, достаточно ли хорошее у булавки качество (сравнение с константой): если хорошее, то увеличивается счётчик обработанных булавок. В любом случае индекс для буфера смещается. <br>
________________________

### 6. Исходные тексты программы на языке C. <br> ###

В двух вариациях. В рамах задачи оказалось удобнее выполнять взаимодействие объектов через последовательный запуск и очередь (решение на C++). <br>
При работе с мьютексами и семафорами возникала следующая проблема (возможно, связано с тем, что защита предполагалась для одного читателя, а не нескольких): поток, генерирующий данные, заканчивался быстрее, чем три потока с потребителями, поэтому часть булавок не была обработана (типичная ситуация: рабочие ушли на перерыв, всё понятно...). Поэтому пришлось разделить задачи потребителей и производителя на последовательные. Решение с потоками также прикрепляется (решение на C). <br>

main.cpp -- основная функция
```cpp
#include <iostream>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <queue>

const int MAX_PINS = 100;

const char *fileInputName = "input.txt";
const char *fileOutputName = "output.txt";

int readI = 0;          // index for reading from buffer
int writeI = 0;         // index for writing to buffer
int count = 0;          // number of occupied cells in buffer

typedef struct {
    int isCurve;        // if 1 -> won't take
    int quantityLevel; // after sharpening increases
} Pin;

Pin pinBuffer[100000];

int processedPins = 0;  // number of processed pins

std::queue<Pin> queue1; // queue for pins to sharping
std::queue<Pin> queue2; // queue for pins to quality control

void curv() {
    Pin pin = pinBuffer[readI];
    if (pin.isCurve) {
        printf("Pin with value = %d is curve\n", pin.quantityLevel);
        readI = (readI + 1) % MAX_PINS; // move to next pin if current is curved
        count++; // we finished processing this curved pin
    } else { // else return pin to buffer to let other workers process it
        printf("Pin with value = %d is straight\n", pin.quantityLevel);
        queue1.push(pin);
    }
}

void sharp() {
    srand(time(NULL));
    if (!queue1.empty()) {
        Pin pin = queue1.front(); // get first pin from queue1 (recently added)
        queue1.pop();
        printf("Sharpening: ");
        int improve = rand() % 100 + 1;
        pin.quantityLevel += improve;
        printf("Pin with quantity level = %d is sharpened by %d\n", pin.quantityLevel - improve, improve);
        queue2.push(pin);
    }
}

void quality() {
    if (!queue2.empty()) {
        Pin pin = queue2.front(); // get first pin from queue2 (recently added)
        queue2.pop();
        printf("Quality control: ");
        if (pin.quantityLevel > 50) {
            printf("Pin with value = %d is good\n", pin.quantityLevel);
            ++processedPins;
        } else {
            printf("Pin with value = %d is bad\n", pin.quantityLevel);
        }
        readI = (readI + 1) % MAX_PINS;
        count++; // we finished processing this pin
    } else {
        printf("Curved pin with quantity level = %d is not controlled\n", pinBuffer[readI].quantityLevel);
    }
}

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

// the func of producers (writers) to generate pins and write them to the buffer (input parameter -- number of pins)
void generateRandomPins(int pinNum) {
    srand(time(NULL));
    for (int i = 0; i < pinNum; i++) {
        int isCurve = rand() % 2;
        int quantityLevel = rand() % 100 + 10;
        Pin nextProduced = { isCurve, quantityLevel }; // create new random pin
        pinBuffer[writeI] = nextProduced; // write pin to buffer
        printf("Generated pin with quantity level = %d to cell [%d]\n", nextProduced.quantityLevel, writeI);
        writeI = (writeI + 1) % MAX_PINS;
    }
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
    // Fill buffer with pins
    generateRandomPins(pins);
    int j = 1;
    while(1) {
        if (count == pins) break; // if all pins are processed, then stop
        if (j % 3 == 1) {
            curv();
        } else if (j % 3 == 2) {
            sharp();
        } else {
            quality();
        }
        ++j;
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    elapsed_ns = timespec_difference(end, start);
    printf("\nElapsed: %lld ns\n", elapsed_ns);
    console_output(processedPins, "The final number of processed pins: %d \n");
    file_output(processedPins, "The final number of processed pins: %d \n", fileOutput);
    return 0;
}

```

main.c -- основная функция
```c
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

```
<br>
________________________

### 7. Информация, подтверждающая выполнение задания в соответствие требованиям на предполагаемую оценку. <br> ###
<br>

Критерий на 4. <br>
1. Приведено условие задачи (см п.3.)
2. Описана используемая модель параллельных вычислений (см. п.4.)
3. Описаны входные данные программы (см. п.5.)
4. Реализовано консольное приложение, решающее поставленную задачу с использованием одного варианта синхропримитивов (решение с мьютексами и семафорами + реализация через очередь)
5. Ввод данных в приложение реализован с консоли (консольный ввод доступен при option = 2)

Критерий на 5. <br>
1. В программу добавлены комментарии, поясняющие выполняемые действия и описание используемых переменных (добавлены комментарии в ключевых моментах кода (C++) и при работе с потоками (C))
2. Сценарий описания одновременного поведения сущностей: <br>
Генерируются данные одним потоком производителя. Их читают одновременно несколько потребителей. <br>
Обработка идёт таким образом: 1->2->3 || 1->2->3 || 1->2->3... || чтение данных <br>
Может прерваться на этапе 1, до 2-го и 3-го не дойти (если прошёл на 2, то пройдёт и на 3).

Критерий на 6. <br>
1. Описан алгоритм реализации программы (см. п.4, п.6, п.7.критерий на 5.)
2. Реализован ввод данных из командной строки (ввод из строки доступен при option = 1)

Критерий на 7. <br>
1. Реализован ввод и вывод данных через файлы (файловый ввод доступен при option = 3, вывод есть)
2. Приведены входные и выходные файлы с различными результатами выполнения программы ([tests](https://github.com/kseniag03/ACS-IHW-4/tree/main/test))
3. Результаты выводятся на экран и записываются в файл (конец программы п.6.)
4. В ввод данных из командной строки добавлена возможность указать имена входного и выходного файлов (присутствует, с учётом значений по умолчанию)

Критерий на 8. <br>
1. Добавлена генерация случайных данных в допустимых диапазонах (генерация начального числа булавок + генерация самих булавок, т.е. полей структуры, так же псевдослучайна)
2. Приведены входные и выходные файлы с различными результатами выполнения программы ([tests](https://github.com/kseniag03/ACS-IHW-4/tree/main/test))
3. В ввод данных из командной строки добавлена возможность выбрать опцию с генерацией (доступна при option = all \ {1, 2, 3})

Критерий на 9 (Вариант-1). <br>
При использовании разных мьютексов для трёх потоков-читателей не выводилось ничего -- программа застревала в бесконечном цикле. <br>
При замене мьютексов на одни лишь семафоры могли перехватываться данные другим потоком и перезаписываться, ломая систему (в ситуации с глобальными переменными и контейнерами особенно актуально) <br>
При использовании циклов внутри функций void* и семафоров не менялись значения качества булавок, т.к. поток №3 наступал раньше, чем №2 или генерация конкретно данного эл-та (т.е. при контроле качества у всех булавок оставалось значение по умолчанию -- 0) <br>
Можно также посмотреть на время работы программы последовательной и параллельной (и понять, что первая по времени даже на маленьких данных сильно выигрывает...) <br>
// для примера: при pins = 3, 25907700 ns при решение с queue, 30049794700 ns -- через buffer <br>


