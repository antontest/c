#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <print.h>
#include <utils/utils.h>

typedef struct private_menu_t private_menu_t;
struct private_menu_t {
    /**
     * @brief public interface
     */
    menu_t public;

    /**
     * @brief header for menu
     */
    char *header;

    /**
     * @brief separator
     */
    char separator;

    /**
     * @brief menu width
     */
    unsigned int menu_width;

    /**
     * @brief start index
     */
    unsigned int start_index;

    /**
     * @brief menu choice count
     */
    int choice_count;

    /**
     * @brief whether support muti selected
     */
    unsigned int is_support_multi_selected;

    /**
     * @brief menu choice
     */
    unsigned int choice[DFT_MAX_MENU_SELECTED_ITEM];
};

METHOD(menu_t, init_menu_, void, private_menu_t *this, char *header, unsigned int start_index, unsigned int is_support_multi_selected)
{
    if (header) {
        this->header = header;
    } else {
        this->header = DFT_MENU_HEADER;
    }
    
    this->start_index = start_index;
    if (is_support_multi_selected >= 1) {
        this->is_support_multi_selected = 1;
    }
}

static void print_separator(char separator, int width)
{
    while (width-- > 0) {
        printf("%c", separator);
    }
    printf("\n");
}

METHOD(menu_t, show_menu_, void, private_menu_t *this, ...)
{
    char    *menu      = NULL;
    int     max_len    = 0;
    int     menu_len   = 0;
    int     header_len = 0;
    int     menu_index = this->start_index;
    va_list menu_list  = NULL;
    

    va_start(menu_list, this);
    while (1) {
        menu = va_arg(menu_list, char *);
        if (!menu) {
            break;
        }
        menu_len = strlen(menu);
        if (menu_len > max_len) {
            max_len = menu_len;
        }
        this->choice_count++;
    }
    va_end(menu_list);

    menu_len += 2;
    if (max_len < DFT_MENU_WIDTH) {
        max_len = DFT_MENU_WIDTH;
    }

    /**
     * print header
     */
    header_len = strlen(this->header);
    printf("%*s\n", header_len + (max_len - header_len) / 2, this->header);

    /**
     * print separator
     */
    print_separator(this->separator, max_len);

    /**
     * print menu
     */
    va_start(menu_list, this);
    while (1) {
        menu = va_arg(menu_list, char *);
        if (!menu) {
            break;
        }
        printf("  %d. %s\n", menu_index++, menu);
    }
    va_end(menu_list);
    
    /**
     * print separator
     */
    print_separator(this->separator, max_len);
    printf("  Please input your choice (%d - %d): \n", this->start_index, menu_index - 1);

}

static int check_alpha(char *string)
{
    char *p = string;

    while (*p != '\0') {
        if (*p < '0' || *p > '8') {
            return 1;
        }
    }

    return 0;
}

static int check_choice(private_menu_t *this, int choice)
{
    int i = 0;
    for (i = 0; i < this->choice_count; i++) {
        if (this->choice[i] == choice) {
            return 1;
        }
    }

    return 0;
}

METHOD(menu_t, get_choice, int, private_menu_t *this, int choices[], int *size)
{
    int  input_index  = 0;
    int  input_choice = 0;
    char buf[128]     = {0};
    char *result      = NULL;

    /**
     * read input
     */
    ignore_result(fgets(buf, sizeof(buf), stdin));
    result = strtok(buf, " ");
    while (result) {
        if (check_alpha(result)) {
            *size = 0;
            return 1;
        }

        input_choice = atoi(result);
        if (input_choice >= this->start_index && input_choice < this->start_index + this->choice_count) {
            if (check_choice(this, input_choice)) {
                goto sep;
            }
            this->choice[input_index++] = input_choice;
            printf("input: %d\n", input_choice);
        } else {
            printf("No such choice!\n");
            return 1;
        }

        if (!this->is_support_multi_selected) {
            break;
        }

sep:
        result = strtok(NULL, " ");
    }

    /**
     * copy choices
     */
    if (choices) {
        memcpy(choices, this->choice, input_index * sizeof(int));
        *size = input_index;
    }

    return 0;
}

METHOD(menu_t, destroy_, void, private_menu_t *this)
{
    free(this);
}

/**
 * @brief menu_create 
 */
menu_t *menu_create()
{
    private_menu_t *this;

    INIT(this, 
        .public = {
            .init_menu  = _init_menu_,
            .show_menu  = _show_menu_,
            .get_choice = _get_choice,
            .destroy    = _destroy_,
        },
        .header                    = NULL,
        .separator                 = DFT_MENU_SEPARATOR,
        .start_index               = DFT_MENU_START_INDEX,
        .is_support_multi_selected = DFT_MENU_MULTI_SELECTED,
        .choice_count              = 0,
    );
    memset(this->choice, -1, sizeof(this->choice));

    return &this->public;
}
