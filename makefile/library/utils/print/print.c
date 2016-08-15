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

METHOD(menu_t, init_menu_, void, private_menu_t *this, unsigned int menu_width, char *header, unsigned int start_index, unsigned int is_support_multi_selected)
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

    this->menu_width = menu_width;
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

    memset(choices, -1, sizeof(int) * *size);
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
            // printf("No such choice!\n");
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
    int len;
    
    /**
     * @brief ponter to fmt
     */
    char *cur;
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
        this->len += strlen(type) + number_len(width) + 2;

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
    this->fmt = (char *)malloc(this->len);
    if (!this->fmt) {
        return 1;
    }
    memset(this->col_width, 0, this->col_cnt * sizeof(int));
    memset(this->fmt,       0, this->col_cnt);
    pfmt = this->cur = this->fmt;

    /**
     * print table header
     */
    len = strlen(header);
    if (total_width < len) {
        total_width = len;
    }
    printf("\033[1;39m%*s\n", len + (total_width - len) / 2, header);

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

METHOD(table_t, show_column_, void, private_table_t *this, ...)
{
    va_list list = NULL;
    char *fmt_start = NULL;
    char ch         = '\0';

    if (*this->cur == '\0' || !this->cur) {
        this->cur = this->fmt;
    }

    fmt_start = this->cur;
    if (*this->cur == '%') {
        this->cur++;
    }

    while ((ch = *this->cur++) != '\0') {
        if (ch == '%') {
            break;
        }
    }

    *(--this->cur) = '\0';
    va_start(list, this);
    vprintf(fmt_start, list);
    va_end(list);

    *this->cur = ch;
    if (ch == '\0') {
        printf("\n");
    }
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
            .init_table  = _init_table_,
            .show_row    = _show_row_,
            .show_column = _show_column_,
            .destroy     = _table_destroy_,
        },
        .col_cnt   = 0,
        .col_width = NULL,
        .fmt       = NULL,
    );

    return &this->public;
}

typedef enum color_status_t color_status_t;
enum color_status_t {
    COLOR_STATUS_STAERT = 1,
    COLOR_STATUS_PASER,
    COLOR_STATUS_END
};

typedef struct color_info_t color_info_t;
struct color_info_t {
    int id;
    char key;
    char *value;
};
typedef enum color_id_t color_id_t;
enum color_id_t {
    COLOR_ID_BLACK  = 0,
    COLOR_ID_RED    ,
    COLOR_ID_GREEN  ,
    COLOR_ID_YELLOW ,
    COLOR_ID_BLUE   ,
    COLOR_ID_PINK   ,
    COLOR_ID_CYAN   ,
    COLOR_ID_WHITE  ,
    COLOR_ID_NORMAL 
};

typedef enum color_key_t color_key_t;
enum color_key_t {
    COLOR_KEY_BLACK  = 'h',
    COLOR_KEY_RED    = 'r',
    COLOR_KEY_GREEN  = 'g',
    COLOR_KEY_YELLOW = 'y',
    COLOR_KEY_BLUE   = 'b',
    COLOR_KEY_PINK   = 'p',
    COLOR_KEY_CYAN   = 'c',
    COLOR_KEY_WHITE  = 'w',
    COLOR_KEY_NORMAL = 'n'
};

static color_info_t clr_info[] = {
    {COLOR_ID_BLACK,  COLOR_KEY_BLACK,  "\033[30m"},
    {COLOR_ID_RED,    COLOR_KEY_RED,    "\033[31m"},
    {COLOR_ID_GREEN,  COLOR_KEY_GREEN,  "\033[32m"},
    {COLOR_ID_YELLOW, COLOR_KEY_YELLOW, "\033[33m"},
    {COLOR_ID_BLUE,   COLOR_KEY_BLUE,   "\033[34m"},
    {COLOR_ID_PINK,   COLOR_KEY_PINK,   "\033[35m"},
    {COLOR_ID_CYAN,   COLOR_KEY_CYAN,   "\033[36m"},
    {COLOR_ID_WHITE,  COLOR_KEY_WHITE,  "\033[37m"},
    {COLOR_ID_NORMAL, COLOR_KEY_NORMAL, "\033[0m" },
};

static char *parser_format(char *fmt)
{
    char *p       = fmt;
    char *result  = NULL;
    char *presult = NULL;
    int  status   = COLOR_STATUS_STAERT;
    int  clr_cnt  = 0;
    int  len      = 0;
    int  color_id = 0;

    while (*p != '\0') {
        len++;
        switch (status) {
            case COLOR_STATUS_STAERT:
                if (*p == '[') {
                    status = COLOR_STATUS_PASER;
                }
                break;
            case COLOR_STATUS_PASER:
                switch (*p) {
                    case COLOR_KEY_BLACK:
                    case COLOR_KEY_RED:
                    case COLOR_KEY_GREEN:
                    case COLOR_KEY_YELLOW:
                    case COLOR_KEY_BLUE:
                    case COLOR_KEY_PINK:
                    case COLOR_KEY_CYAN:
                    case COLOR_KEY_WHITE:
                    case COLOR_KEY_NORMAL:
                        status = COLOR_STATUS_END;
                        break;
                    default:
                        status = COLOR_STATUS_STAERT;
                        break;
                }
                break;
            case COLOR_STATUS_END:
                clr_cnt++;
                status = COLOR_STATUS_STAERT;
                break;
            default:
                break;
        }
        p++;
    }

    /**
     * malloc memory for format
     */
    len = clr_cnt * 8 + len - clr_cnt * 3 + 1;
    result = (char *)malloc(len);
    if (!result) {
        return NULL;
    }
    memset(result, 0, len);
    presult = result;

    /**
     * parser format
     */
    p = fmt;
    while (*p != '\0') {
        switch (status) {
            case COLOR_STATUS_STAERT:
                if (*p == '[') {
                    status = COLOR_STATUS_PASER;
                }
                *presult++ = *p;
                break;
            case COLOR_STATUS_PASER:
                switch (*p) {
                    case COLOR_KEY_BLACK:
                        color_id = COLOR_ID_BLACK;
                        break;
                    case COLOR_KEY_RED:
                        color_id = COLOR_ID_RED;
                        break;
                    case COLOR_KEY_GREEN:
                        color_id = COLOR_ID_GREEN;
                        break;
                    case COLOR_KEY_YELLOW:
                        color_id = COLOR_ID_YELLOW;
                        break;
                    case COLOR_KEY_BLUE:
                        color_id = COLOR_ID_BLUE;
                        break;
                    case COLOR_KEY_PINK:
                        color_id = COLOR_ID_PINK;
                        break;
                    case COLOR_KEY_CYAN:
                        color_id = COLOR_ID_CYAN;
                        break;
                    case COLOR_KEY_WHITE:
                        color_id = COLOR_ID_WHITE;
                        break;
                    case COLOR_KEY_NORMAL:
                        color_id = COLOR_ID_NORMAL;
                        break;
                    default:
                        *presult++ = *p;
                        break;
                }

                len = sprintf(presult - 1, "%s", clr_info[color_id].value);
                presult += len - 1;
                status = COLOR_STATUS_END;
                break;
            case COLOR_STATUS_END:
                status = COLOR_STATUS_STAERT;
                break;
            default:
                break;
        }
        p++;
    }
    
    return result;
}

/**
 * @brief color print
 *
 * @param fmt [in] printf format
 * @param ... [in]
 */
void cprintf(char *fmt, ...)
{
    va_list list  = NULL;
    char *new_fmt = NULL;
    new_fmt = parser_format(fmt);

    va_start(list, fmt);
    vprintf(new_fmt, list);
    va_end(list);

    free(new_fmt);
}
