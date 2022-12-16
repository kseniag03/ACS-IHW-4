#include <iostream>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <queue>

const int MAX_PINS = 10;//100000;

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
int processedPins = 0; // number of processed pins (pins)

std::queue<Pin> queue1;
std::queue<Pin> queue2;

void curv() {
    Pin pin = pinBuffer[front];
    if (pin.isCurve) {
        printf("Pin with value = %d is curve\n", pin.quantityLevel);
        front = (front + 1) % MAX_PINS; // двигаемся к следующей булавке, если текущая оказалась кривой
        count++;
    } else { // иначе возвращаем булавку в буфер, чтобы другие рабочие могли ее обработать
        printf("Pin with value = %d is straight\n", pin.quantityLevel);
        queue1.push(pin);
    }
}

void sharp() {
    srand(time(NULL));
    if (!queue1.empty()) {
        Pin pin = queue1.front();
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
        Pin pin = queue2.front();
        queue2.pop();
        printf("Quality control: ");
        if (pin.quantityLevel > 50) {
            printf("Pin with value = %d is good\n", pin.quantityLevel);
            ++processedPins;
        } else {
            printf("Pin with value = %d is bad\n", pin.quantityLevel);
        }
        front = (front + 1) % MAX_PINS;
        count++;
    } else {
        printf("Curved pin with quantity level = %d is not controlled\n", pinBuffer[front].quantityLevel);
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

//стартовая функция производителей (писателей) -- генерация булавок и запись их в буфер (входной параметр -- число генерируемых булавок)
void generateRandomPins(int pinNum) {
    srand(time(NULL));
    for (int i = 0; i < pinNum; i++) {
        int isCurve = rand() % 2;
        int quantityLevel = rand() % 100 + 10;
        Pin nextProduced = { isCurve, quantityLevel };
        //запись в общий буфер
        pinBuffer[rear] = nextProduced;
        printf("Generated pin with quantity level = %d to cell [%d]\n", nextProduced.quantityLevel, rear);
        rear = (rear + 1) % MAX_PINS;
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

    // Начальная инициализация массива
    generateRandomPins(pins);
    for(int i = 0; i < pins; i++) {
        printf("Pin with value = %d\n", pinBuffer[i].quantityLevel);
    }

    int j = 1;
    while(1) {
        if (count == pins) break;
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
