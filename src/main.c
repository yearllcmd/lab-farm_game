// Весёлый фермер, лаба 3, вариант 3

// подключаем библиотеки
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// windows.h только для Windows (русские буквы в консоли)
#ifdef _WIN32
#include <windows.h>
#include <wchar.h>
#endif

// константы рюкзака и предметов
#define INVENTORY_SIZE 10
#define EMPTY 0
#define MAX_ITEMS 10
#define MAX_NAME_LEN 32
#define MIN_ITEM_ID 1
#define MAX_ITEM_ID (MAX_ITEMS - 1)

// файлы
#define ITEMS_FILE "items.txt"
#define DIARY_FILE "diary.txt"

// константы времени
#define START_DAY 1
#define START_HOUR 8
#define HOURS_IN_DAY 24
#define MIN_WORK_HOURS 1
#define MAX_WORK_HOURS 1000

// размер строки для ввода
#define LINE_SIZE 256

// длина имени фермера (без '\0')
#define MAX_FARMER_NAME 32

// пункты меню
enum Menu {
    MENU_EXIT,
    MENU_CLOCK,
    MENU_WORK,
    MENU_SHOW,
    MENU_PUT,
    MENU_DROP,
    MENU_CLEAN,
    MENU_SEARCH,
    MENU_DIARY,
    MENU_COUNT // сколько всего пунктов
};

// предмет не найден в каталоге
#define NOT_FOUND -1

// стандартные названия, если items.txt не найден (индекс = id предмета)
const char *DEFAULT_ITEM_NAMES[MAX_ITEMS] = {
    "Пусто", "Дерево", "Камень", "Семена_пшеницы", "Морковь",
    "Лейка", "Удобрение", "Сорняк", "Сломанная_ветка", "Ржавый_гвоздь"
};

// ===== ввод =====

// читает строку с клавиатуры, возвращает 0 если строка слишком длинная
int read_line(char line[], int size)
{
#ifdef _WIN32
    // в консоли Windows читаем через ReadConsoleW, иначе русские буквы ломаются
    HANDLE console = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode;
    if (GetConsoleMode(console, &mode)) {
        wchar_t wide[LINE_SIZE];
        DWORD count;
        if (!ReadConsoleW(console, wide, LINE_SIZE - 1, &count, NULL)) {
            exit(0);
        }
        wide[count] = L'\0';
        // нет Enter - строка не влезла, дочитываем остаток
        if (wcschr(wide, L'\n') == NULL) {
            do {
                if (!ReadConsoleW(console, wide, LINE_SIZE - 1, &count, NULL)) {
                    exit(0);
                }
                wide[count] = L'\0';
            } while (wcschr(wide, L'\n') == NULL);
            return 0;
        }
        wide[wcscspn(wide, L"\r\n")] = L'\0';
        return WideCharToMultiByte(CP_UTF8, 0, wide, -1, line, size, NULL, NULL) != 0;
    }
#endif
    if (fgets(line, size, stdin) == NULL) {
        exit(0);
    }
    // нет Enter - строка не влезла, дочитываем остаток
    if (strchr(line, '\n') == NULL && !feof(stdin)) {
        int c;
        while ((c = getchar()) != '\n' && c != EOF) {
        }
        return 0;
    }
    line[strcspn(line, "\r\n")] = '\0';
    return 1;
}

// ввод числа с защитой от дурака
int read_number(int min, int max)
{
    char line[LINE_SIZE];
    int number;
    char extra;

    // повторяем, пока не введут одно число из диапазона
    while (!read_line(line, LINE_SIZE) || sscanf(line, "%d %c", &number, &extra) != 1
           || number < min || number > max) {
        printf("Ошибка! Введите число от %d до %d: ", min, max);
    }
    return number;
}

// имя фермера: не пустое и не длиннее MAX_FARMER_NAME
void ask_farmer_name(char name[])
{
    printf("Как зовут фермера? ");
    while (!read_line(name, MAX_FARMER_NAME + 1) || strlen(name) == 0) {
        printf("Ошибка! Имя от 1 до %d символов: ", MAX_FARMER_NAME);
    }
}

int ask_menu_choice(void)
{
    printf("Выберите пункт: ");
    return read_number(MENU_EXIT, MENU_COUNT - 1);
}

// название предмета, не пустое
void ask_item_name(char name[])
{
    printf("Название предмета: ");
    while (!read_line(name, MAX_NAME_LEN) || strlen(name) == 0) {
        printf("Ошибка! Название от 1 до %d символов: ", MAX_NAME_LEN - 1);
    }
}

int ask_work_hours(void)
{
    printf("Сколько часов работать? ");
    return read_number(MIN_WORK_HOURS, MAX_WORK_HOURS);
}

int ask_slot(void)
{
    printf("Номер слота (0-%d): ", INVENTORY_SIZE - 1);
    return read_number(0, INVENTORY_SIZE - 1);
}

int ask_item_id(void)
{
    printf("ID предмета (%d-%d): ", MIN_ITEM_ID, MAX_ITEM_ID);
    return read_number(MIN_ITEM_ID, MAX_ITEM_ID);
}

// ===== файлы =====

// читает строку из файла без '\n', возвращает 0 когда файл кончился
int read_file_line(FILE *file, char line[], int size)
{
    if (fgets(line, size, file) == NULL) {
        return 0;
    }
    // строка длиннее буфера - пропускаем остаток
    if (strchr(line, '\n') == NULL && !feof(file)) {
        int c;
        while ((c = fgetc(file)) != '\n' && c != EOF) {
        }
    }
    line[strcspn(line, "\r\n")] = '\0';
    return 1;
}

// заполняет каталог стандартными названиями
void set_default_items(char names[][MAX_NAME_LEN])
{
    for (int id = 0; id < MAX_ITEMS; id++) {
        strcpy(names[id], DEFAULT_ITEM_NAMES[id]);
    }
}

// читает названия из файла (строки вида "1 Дерево")
// возвращает сколько названий загружено, или -1 если файла нет
int load_items(char names[][MAX_NAME_LEN], const char filename[])
{
    set_default_items(names); // чего нет в файле - останется по умолчанию

    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        return -1;
    }

    char line[LINE_SIZE];
    int loaded = 0;
    while (read_file_line(file, line, LINE_SIZE)) {
        int id, pos;
        // id, потом пробелы, pos - где начинается название
        if (sscanf(line, "%d %n", &id, &pos) == 1 && id >= 0 && id < MAX_ITEMS
            && strlen(line + pos) > 0 && strlen(line + pos) < MAX_NAME_LEN) {
            strcpy(names[id], line + pos);
            loaded++;
        }
    }
    fclose(file);
    return loaded;
}

// ===== обработка =====

// день после работы (каждые 24 часа - новый день)
int day_after_work(int day, int hour, int hours)
{
    return day + (hour + hours) / HOURS_IN_DAY;
}

// час после работы (остаток от суток)
int hour_after_work(int hour, int hours)
{
    return (hour + hours) % HOURS_IN_DAY;
}

// убирает все предметы с этим id, возвращает сколько слотов очищено
int clean_trash(int inventory[], int id)
{
    int count = 0; // счётчик удалённых слотов
    for (int i = 0; i < INVENTORY_SIZE; i++) {
        if (inventory[i] == id) { // если находит нужный айди то...
            inventory[i] = EMPTY; // удаляет этот предмет после чего...
            count++;              // увеличивает счётчик на 1
        }
    }
    return count;
}

// ищет id по названию, NOT_FOUND если такого нет
int find_item_id(char names[][MAX_NAME_LEN], const char name[])
{
    for (int id = 0; id < MAX_ITEMS; id++) {
        if (strcmp(names[id], name) == 0) { // 0 - строки одинаковые
            return id;
        }
    }
    return NOT_FOUND;
}

// записывает в slots номера слотов с этим id, возвращает сколько нашлось
int find_item_slots(int inventory[], int id, int slots[])
{
    int found = 0;
    for (int i = 0; i < INVENTORY_SIZE; i++) {
        if (inventory[i] == id) {
            slots[found] = i;
            found++;
        }
    }
    return found;
}

// ===== вывод =====

void print_menu(const char name[])
{
    printf("\n===== Фермер %s =====\n", name);
    printf("[0] Выход\n"
           "[1] Посмотреть на часы\n"
           "[2] Промотать время (Поработать)\n"
           "[3] Посмотреть инвентарь\n"
           "[4] Положить предмет в слот\n"
           "[5] Выбросить предмет\n"
           "[6] Очистка от мусора\n"
           "[7] Поиск предмета в рюкзаке по названию\n"
           "[8] Записать состояние в дневник фермера\n");
}

void print_time(int day, int hour)
{
    printf("Текущее время: День %d, %02d:00\n", day, hour);
}

// название берём из каталога по id предмета
void print_inventory(int inventory[], char names[][MAX_NAME_LEN])
{
    for (int i = 0; i < INVENTORY_SIZE; i++) {
        printf("Слот %d: [%d] - %s\n", i, inventory[i], names[inventory[i]]);
    }
}

// сводка для дневника, out - экран (stdout) или файл
void print_summary(FILE *out, const char name[], int day, int hour,
                   int inventory[], char names[][MAX_NAME_LEN])
{
    fprintf(out, "===== Дневник фермера =====\n");
    fprintf(out, "Фермер: %s\n", name);
    fprintf(out, "Время: День %d, %02d:00\n", day, hour);
    fprintf(out, "Инвентарь:\n");
    for (int i = 0; i < INVENTORY_SIZE; i++) {
        fprintf(out, "  Слот %d: %s\n", i, names[inventory[i]]);
    }
    fprintf(out, "\n");
}

// ===== пункты меню =====

// [4] положить предмет в слот
void menu_put_item(int inventory[], char names[][MAX_NAME_LEN])
{
    int slot = ask_slot();
    int id = ask_item_id();
    inventory[slot] = id;
    printf("В слот %d положен предмет: %s\n", slot, names[id]);
}

// [5] выбросить предмет
void menu_drop_item(int inventory[])
{
    int slot = ask_slot();
    inventory[slot] = EMPTY;
    printf("Слот %d очищен\n", slot);
}

// [6] очистка от мусора (вариант 5 из лабы 2)
void menu_clean_trash(int inventory[])
{
    printf("Мусор - ");
    int id = ask_item_id();
    printf("Очищено слотов: %d\n", clean_trash(inventory, id));
}

// [7] поиск предмета по названию
void menu_search_item(int inventory[], char names[][MAX_NAME_LEN])
{
    char name[MAX_NAME_LEN];
    ask_item_name(name);

    int id = find_item_id(names, name);
    if (id == NOT_FOUND) {
        printf("Предмета \"%s\" нет в каталоге\n", name);
        return;
    }

    int slots[INVENTORY_SIZE];
    int found = find_item_slots(inventory, id, slots);
    if (found == 0) {
        printf("Предмета \"%s\" нет в рюкзаке\n", name);
        return;
    }

    printf("\"%s\" (ID %d) лежит в слотах:", name, id);
    for (int i = 0; i < found; i++) {
        printf(" %d", slots[i]);
    }
    printf("\n");
}

// [8] запись в дневник: на экран и в конец diary.txt
void menu_write_diary(const char name[], int day, int hour,
                      int inventory[], char names[][MAX_NAME_LEN])
{
    print_summary(stdout, name, day, hour, inventory, names);

    FILE *file = fopen(DIARY_FILE, "a"); // "a" - дописать в конец
    if (file == NULL) {
        printf("Ошибка: не удалось открыть %s\n", DIARY_FILE);
        return;
    }
    print_summary(file, name, day, hour, inventory, names);
    fclose(file);
    printf("Записано в %s\n", DIARY_FILE);
}

// главная функция, отсюда стартует программа
int main(void)
{
    // русские буквы в консоли
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif

    // имя фермера (+1 место под '\0')
    char farmer_name[MAX_FARMER_NAME + 1];
    ask_farmer_name(farmer_name);
    printf("Привет, %s! Добро пожаловать на ферму.\n", farmer_name);

    // каталог предметов из файла
    char item_names[MAX_ITEMS][MAX_NAME_LEN];
    int loaded = load_items(item_names, ITEMS_FILE);
    if (loaded < 0) {
        printf("Внимание: файл %s не найден, названия предметов по умолчанию\n", ITEMS_FILE);
    } else {
        printf("Загружено названий из %s: %d\n", ITEMS_FILE, loaded);
    }

    // игровое время: день 1, 08:00
    int current_day = START_DAY;
    int current_hour = START_HOUR;

    // рюкзак (7, 8, 9 - мусор)
    int inventory[INVENTORY_SIZE] = {1, 7, 3, 7, 2, 9, 7, 0, 8, 9};

    int choice, hours;

    // главный цикл игры, крутится пока не выбран выход
    do {
        print_menu(farmer_name);
        choice = ask_menu_choice();

        switch (choice) {
        case MENU_EXIT:
            printf("Пока!\n");
            break;
        case MENU_CLOCK:
            print_time(current_day, current_hour);
            break;
        case MENU_WORK:
            hours = ask_work_hours();
            current_day = day_after_work(current_day, current_hour, hours);
            current_hour = hour_after_work(current_hour, hours);
            print_time(current_day, current_hour);
            break;
        case MENU_SHOW:
            print_inventory(inventory, item_names);
            break;
        case MENU_PUT:
            menu_put_item(inventory, item_names);
            break;
        case MENU_DROP:
            menu_drop_item(inventory);
            break;
        case MENU_CLEAN:
            menu_clean_trash(inventory);
            break;
        case MENU_SEARCH:
            menu_search_item(inventory, item_names);
            break;
        case MENU_DIARY:
            menu_write_diary(farmer_name, current_day, current_hour, inventory, item_names);
            break;
        // на всякий случай
        default:
            printf("Нет такого пункта\n");
            break;
        }
    } while (choice != MENU_EXIT);

    return 0;
}
