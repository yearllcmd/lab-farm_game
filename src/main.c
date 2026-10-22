// Весёлый фермер, лаба 2, вариант 5

// подключаем библиотеки (printf, scanf, exit)
#include <stdio.h>
#include <stdlib.h>

// windows.h только для Windows (русские буквы в консоли)
#ifdef _WIN32
#include <windows.h>
#endif

// константы времени
#define START_DAY 1
#define START_HOUR 8
#define HOURS_IN_DAY 24
#define MIN_WORK_HOURS 1
#define MAX_WORK_HOURS 1000

// пункты меню (номера 0-2)
enum Menu {
    MENU_EXIT,
    MENU_CLOCK,
    MENU_WORK
};

// функция read_number - ввод числа с защитой от дурака
int read_number(int min, int max)
{
    int number;
    char next = 0;

    // повторяем, пока ввод неправильный
    while (scanf("%d%c", &number, &next) != 2 || next != '\n' || number < min || number > max) {
        // ввод закончился - выходим
        if (feof(stdin)) {
            exit(0);
        }
        // чистим остаток строки
        if (next != '\n') {
            int c;
            while ((c = getchar()) != '\n' && c != EOF) {
            }
        }
        next = 0;
        printf("Ошибка! Введите число от %d до %d: ", min, max);
    }
    return number;
}

// главная функция, отсюда стартует программа
int main(void)
{
    // русские буквы в консоли
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif

    // игровое время: день 1, 08:00
    int current_day = START_DAY;
    int current_hour = START_HOUR;

    // переменные для ввода
    int choice, hours;

    // главный цикл игры, крутится пока не выбран выход
    do {
        // меню
        printf("\n[0] Выход\n"
               "[1] Посмотреть на часы\n"
               "[2] Промотать время (Поработать)\n"
               "Выберите пункт: ");
        choice = read_number(MENU_EXIT, MENU_WORK);

        // выбор пункта меню
        switch (choice) {
        // [0] выход
        case MENU_EXIT:
            printf("Пока!\n");
            break;

        // [1] посмотреть на часы
        case MENU_CLOCK:
            printf("Текущее время: День %d, %02d:00\n", current_day, current_hour);
            break;

        // [2] промотать время, переход на следующие дни
        case MENU_WORK:
            printf("Сколько часов работать? ");
            hours = read_number(MIN_WORK_HOURS, MAX_WORK_HOURS);
            current_hour += hours;
            current_day += current_hour / HOURS_IN_DAY;
            current_hour %= HOURS_IN_DAY;
            printf("Текущее время: День %d, %02d:00\n", current_day, current_hour);
            break;

        // на всякий случай
        default:
            printf("Нет такого пункта\n");
            break;
        }
    } while (choice != MENU_EXIT);

    return 0;
}
