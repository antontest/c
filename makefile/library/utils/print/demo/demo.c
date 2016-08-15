#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <print.h>

int main(int argc, char **argv)
{
#if 0
    menu_t *menu = NULL;
    int choice[5] = {0};
    int size = sizeof(choice);

    menu = menu_create();
    menu->init_menu(menu, "Menu", 0, 1);
    menu->show_menu(menu, "Test1", "Test2", "Test3", NULL);
    menu->get_choice(menu, (int *)choice, &size);
    printf("size: %d\n", size);

    printf("choice: ");
    while (--size >= 0) {
        printf("%d | ", choice[size]);
    }
    printf("\n");
    menu->destroy(menu);
#endif

#if 0
    table_t *table = NULL;

    table = table_create();
    table->init_table(table, "Digest Test",
        "Name", "%s", 10,
        "age", "%d", 4,
        "info", "%s", 7,
        NULL);

    table->show_row(table, 
        "anton", 23, "6.05",
        NULL);
    table->show_row(table, 
        "antonio", 25, "3336.05",
        NULL);

    table->show_column(table, "lee");
    table->show_column(table, 23);
    table->show_column(table, "5.43");
    table->show_column(table, "lee");
    table->show_column(table, 23);
    table->show_column(table, "5.43");
    table->destroy(table);
#endif

#if 1
    printf("\033[31m\033[1maaa\033[33maa\033[0m\n");
    cprintf("[r]red[y]yellow[g]green[n][b]%s[n][c]cyan[p]pink[n]\n", "blue");

#if 0
    printf("%-10s", "SM1_EBC");
    fflush(stdout);
    sleep(4);
    printf("%-8s\n", "OK");
    fflush(stdout);
#endif

#endif

    return 0;
}
