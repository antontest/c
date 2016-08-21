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
    int size = sizeof(choice) / sizeof(choice[0]);

    menu = menu_create();
    menu->init(menu, "Perform Test Menu", 1, 1);
    menu->show(menu, "Test Enc Only", "Test Dec Only", "Test Digest Only", NULL);
    menu->get_choice(menu, (int *)choice, &size);
    printf("size: %d\n", size);

    printf("choice: ");
    while (--size >= 0) {
        printf("%d | ", choice[size]);
    }
    printf("\n");
    menu->destroy(menu);
    return 0;
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

#if 0
    printf("\033[31m\033[1maaa\033[33maa\033[0m\n");
    cprintf("[r]red[y]yellow[g][U][S]green[n][b][I]%s[n][c][B]cyan[p]pink[n]\n", "blue");
    printf("╔═════════════════════════╗\n"); 

#if 0
    printf("%-10s", "SM1_EBC");
    fflush(stdout);
    sleep(4);
    printf("%-8s\n", "OK");
    fflush(stdout);
#endif

#endif

    int bit = 0;
    progress_t *progress = NULL;

    progress = progress_create();
    progress->init(progress, "test", 50);

    for (bit = 1; bit <= 50; bit += 2) {
        progress->show(progress, bit);
        sleep(1);
    }
    progress->destroy(progress);

    return 0;
}
