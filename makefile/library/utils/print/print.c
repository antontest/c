#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <print.h>
#include <utils/utils.h>

typedef enum input_type_t input_type_t;
enum input_type_t {
    INPUT_TYPE_CHAR = 0, 
    INPUT_TYPE_STR,
    INPUT_TYPE_INT,
    INPUT_TYPE_LONG,
    INPUT_TYPE_FLOAT,
};

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

    while (*p != '\0' && *p != '\n') {
        if ((*p >= '0' && *p <= '9') || *p == ' ') {
            p++;
        } else {
            return 1;
        }
    }

    return 0;
}

static int check_choice(private_menu_t *this, int choices[], int choice)
{
    int i = 0;

    if (!choices) {
        return 0;
    }

    for (i = 0; i < this->choice_count; i++) {
        if (choices[i] == choice) {
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
    if (!fgets(buf, sizeof(buf), stdin) || check_alpha(buf)) {
        *size = 0;
        return 1;
    }
    result = strtok(buf, " ");

    while (result) {
        input_choice = atoi(result);
        if (input_choice >= this->start_index && input_choice < this->start_index + this->choice_count) {
            if (check_choice(this, choices, input_choice)) {
                goto sep;
            }
            if (choices && input_index < *size) {
                choices[input_index++] = input_choice;
            }
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
    *size = input_index;

    return 0;
}

METHOD(menu_t, menu_destroy_, void, private_menu_t *this)
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
            .destroy    = _menu_destroy_,
        },
        .header                    = NULL,
        .separator                 = DFT_MENU_SEPARATOR,
        .start_index               = DFT_MENU_START_INDEX,
        .is_support_multi_selected = DFT_MENU_MULTI_SELECTED,
        .choice_count              = 0,
    );

    return &this->public;
}


typedef struct private_table_t private_table_t;
struct private_table_t {
    /**
     * @brief public interface
     */
    table_t public;

    /**
     * @brief count of table colum 
     */
    int col_cnt;

    /**
     * @brief width of colums
     */
    int *col_width;

    /**
     * @brief format of row output
     */
    char *fmt;

    /**
     * @brief fmt length
     */
    int fmt_len;
};

static int number_len(int n)
{
    int len = 0;
    while (n) {
        len++;
        n /= 10;
    }

    return len;
}

METHOD(table_t, init_table_, int, private_table_t *this, char *header, ...)
{
    va_list list        = NULL;
    char    *col        = NULL, *type = NULL;
    int     width       = 0;
    int     total_width = 0;
    int     col_index   = 0;
    int     len         = 0;
    char    last_ch     = '0';
    char    *pfmt       = NULL;
    char    *ptype      = NULL;


    if (!header) {
        return 1;
    }

    va_start(list, header);
    while (1) {
        col = va_arg(list, char *);
        if (!col) {
            break;
        }

        type = va_arg(list, char *);
        if (!type) {
            break;
        }

        width = va_arg(list, int);
        this->fmt_len += strlen(type) + number_len(width) + 2;

        total_width += width + 1;
        this->col_cnt++;
    }
    va_end(list);
    total_width--;

    /**
     * malloc memory for col and fmt
     */
    this->col_width = (int *)malloc(this->col_cnt * sizeof(int));
    if (!this->col_width) {
        return 1;
    }
    this->fmt = (char *)malloc(this->fmt_len);
    if (!this->fmt) {
        return 1;
    }
    memset(this->col_width, 0, this->col_cnt * sizeof(int));
    memset(this->fmt,       0, this->col_cnt);
    pfmt = this->fmt;

    /**
     * print table header
     */
    len = strlen(header);
    if (total_width < len) {
        total_width = len;
    }
    printf("\033[1m%*s\n", len + (total_width - len) / 2, header);

    /**
     * print separator
     */
    print_separator('=', total_width);

    len = 0;
    va_start(list, header);
    while (1) {
        col = va_arg(list, char *);
        if (!col) {
            break;
        }

        type = va_arg(list, char *);
        if (!type) {
            break;
        }
        ptype = type;

        width = va_arg(list, int);

        last_ch = *(type + strlen(type) - 1);
        if (last_ch == 'f') {
            len = sprintf(pfmt, "%s ", type);
            pfmt += len;
        } else {
            if (*ptype != '%') {
                return 1;
            }

            *pfmt++ = *ptype++;
            len = sprintf(pfmt, "%d", width);
            pfmt += len;
            while ((*pfmt++ = *ptype++) != '\0') {
            }
            pfmt--;
            *pfmt++ = ' ';
        }
        printf("%*s ", width, col);
    }
    va_end(list);
    printf("\033[0m\n");

    return 0;
}

METHOD(table_t, show_row_, void, private_table_t *this, ...)
{
    va_list list    = NULL;

    va_start(list, this);
    vprintf(this->fmt, list);
    va_end(list);
    printf("\n");
}

METHOD(table_t, table_destroy_, void, private_table_t *this)
{
    if (this->col_width) {
        free(this->col_width);
    }
    if (this->fmt) {
        free(this->fmt);
    }
    free(this);
}

table_t *table_create()
{
    private_table_t *this;

    INIT(this,
        .public = {
            .init_table = _init_table_,
            .show_row   = _show_row_,
            .destroy    = _table_destroy_,
        },
        .col_cnt   = 0,
        .col_width = NULL,
        .fmt       = NULL,
    );

    return &this->public;
}
