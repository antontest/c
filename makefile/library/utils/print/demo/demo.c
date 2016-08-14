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
    table->destroy(table);

    return 0;
}
