// Весёлый фермер, лаба 2, вариант 5

// подключаем библиотеки (printf, scanf, exit)
#include <stdio.h>
#include <stdlib.h>

// windows.h только для Windows (русские буквы в консоли)
#ifdef _WIN32
#include <windows.h>
#endif

// константы рюкзака и предметов
#define INVENTORY_SIZE 10
#define EMPTY 0
#define MIN_ITEM_ID 1
#define MAX_ITEM_ID 9

// константы времени
#define START_DAY 1
#define START_HOUR 8
#define HOURS_IN_DAY 24
#define MIN_WORK_HOURS 1
#define MAX_WORK_HOURS 1000

// пункты меню (номера 0-6)
enum Menu {
    MENU_EXIT,
    MENU_CLOCK,
    MENU_WORK,
    MENU_SHOW,
    MENU_PUT,
    MENU_DROP,
    MENU_CLEAN
};

// названия предметов (индекс = id предмета)
const char *item_names[MAX_ITEM_ID + 1] = {
    "Пусто", "Дерево", "Камень", "Семена", "Морковь",
    "Лейка", "Удобрение", "Сорняк", "Сломанная ветка", "Ржавый гвоздь"
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

// функция print_inventory - показать рюкзак
void print_inventory(int inventory[])
{
    for (int i = 0; i < INVENTORY_SIZE; i++) {
        printf("Слот %d: [%d] (%s)\n", i, inventory[i], item_names[inventory[i]]);
    }
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

    // рюкзак (7, 8, 9 - мусор)
    int inventory[INVENTORY_SIZE] = {1, 7, 3, 7, 2, 9, 7, 0, 8, 9};

    // переменные для ввода
    int choice, hours, slot, id, count;

    // главный цикл игры, крутится пока не выбран выход
    do {
        // меню
        printf("\n[0] Выход\n"
               "[1] Посмотреть на часы\n"
               "[2] Промотать время (Поработать)\n"
               "[3] Посмотреть инвентарь\n"
               "[4] Положить предмет в слот\n"
               "[5] Выбросить предмет\n"
               "[6] Очистка от мусора\n"
               "Выберите пункт: ");
        choice = read_number(MENU_EXIT, MENU_CLEAN);

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

        // [3] посмотреть инвентарь
        case MENU_SHOW:
            print_inventory(inventory);
            break;

        // [4] положить предмет в слот
        case MENU_PUT:
            printf("Номер слота (0-%d): ", INVENTORY_SIZE - 1);
            slot = read_number(0, INVENTORY_SIZE - 1);
            printf("ID предмета (%d-%d): ", MIN_ITEM_ID, MAX_ITEM_ID);
            id = read_number(MIN_ITEM_ID, MAX_ITEM_ID);
            inventory[slot] = id;
            printf("В слот %d положен предмет: %s\n", slot, item_names[id]);
            break;

        // [5] выбросить предмет
        case MENU_DROP:
            printf("Номер слота (0-%d): ", INVENTORY_SIZE - 1);
            slot = read_number(0, INVENTORY_SIZE - 1);
            inventory[slot] = EMPTY;
            printf("Слот %d очищен\n", slot);
            break;

        // [6] вариант 5 - очистка от мусора
        case MENU_CLEAN:
            printf("ID мусора (%d-%d): ", MIN_ITEM_ID, MAX_ITEM_ID);
            id = read_number(MIN_ITEM_ID, MAX_ITEM_ID);
            count = 0;
            for (int i = 0; i < INVENTORY_SIZE; i++) {
                if (inventory[i] == id) {
                    inventory[i] = EMPTY;
                    count++;
                }
            }
            printf("Очищено слотов: %d\n", count);
            break;

        // на всякий случай
        default:
            printf("Нет такого пункта\n");
            break;
        }
    } while (choice != MENU_EXIT);

    return 0;
}
