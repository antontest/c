#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <print.h>
#include <utils/utils.h>

/**
 *
 *  左上 = ┌
 *  上   =  ┬
 *  右上 =  ┐
 *  左   =  ├
 *  中心 =  ┼
 *  右   =  ┤
 *  左下 =  └
 *  下   =  ┴
 *  右下 =  ┘
 *  垂直 =  │
 *  水平 =   ─
 *
 */

typedef enum direct_id_t direct_id_t;
enum direct_id_t {
    DIRECT_UPLEFT = 0,
    DIRECT_UP,
    DIRECT_UPRIGHT,
    DIRECT_LEFT,
    DIRECT_CENTER,
    DIRECT_RIGHT,
    DIRECT_DOWNLEFT,
    DIRECT_DOWN,
    DIRECT_DOWNRIGHT,
    DIRECT_VERTICAL,
    DIRECT_HORIZONTAL
};

typedef struct direct_info_t direct_info_t;
struct direct_info_t {
    direct_id_t id;
    char *key;
};

static direct_info_t direct_info[] = {
    {DIRECT_UPLEFT,     "┌"},
    {DIRECT_UP,         "┬"},
    {DIRECT_UPRIGHT,    "┐"},
    {DIRECT_LEFT,       "├"},
    {DIRECT_CENTER,     "┼"},
    {DIRECT_RIGHT,      "┤"},
    {DIRECT_DOWNLEFT,   "└"},
    {DIRECT_DOWN,       "┴"},
    {DIRECT_DOWNRIGHT,  "┘"},
    {DIRECT_VERTICAL,   "│"},
    {DIRECT_HORIZONTAL, "─"},
};

typedef enum input_type_t input_type_t;
enum input_type_t {
    INPUT_TYPE_CHAR = 0, 
    INPUT_TYPE_STR,
    INPUT_TYPE_INT,
    INPUT_TYPE_LONG,
    INPUT_TYPE_FLOAT,
};

typedef enum menu_state_t menu_state_t;
enum menu_state_t {
    MENU_INIT = 0,
    MENU_INIT_SEPARATOR,
    MENU_INIT_END,
    MENU_HEAD_START,
    MENU_HEAD,
    MENU_HEAD_END,
    MENU_LINE_START,
    MENU_LINE,
    MENU_LINE_END,
    MENU_CONTENT_START,
    MENU_PREFIX_BLANK,
    MENU_CONTENT_NUM, 
    MENU_CONTENT, 
    MENU_SUFFIX_BLANK,
    MENU_CONTENT_END,
    MENU_END_START,
    MENU_END_SEPARATOR,
    MENU_END
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

static int number_len(int n)
{
    int len = 0;

    if (!n) {
        return 1;
    }

    while (n) {
        len++;
        n /= 10;
    }

    return len;
}

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

void print_separator(char separator, int width)
{
    while (width-- > 0) {
        printf("%c", separator);
    }
    printf("\n");
}

METHOD(menu_t, show_menu_, void, private_menu_t *this, ...)
{
    va_list menu_list;
    char    *menu       = NULL;
    int     len         = 0;
    int     max_len     = 0;
    int     menu_len    = 0;
    int     header_len  = 0;
    int     end_flag    = 1;
    int     line_pos    = 0;
    int     menu_index  = this->start_index;
    menu_state_t status = MENU_INIT;
    

    /**
     * get max length
     */
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

    /**
     * set max length
     */
    header_len  = strlen(this->header);
    max_len    += number_len(this->choice_count);
    if (max_len < header_len) {
        max_len = header_len;
    }
    if (max_len < DFT_MENU_WIDTH) {
        max_len = DFT_MENU_WIDTH;
    }

#if 0
    /**
     * print header
     */
    print_separator(this->separator, max_len);
    header_len = strlen(this->header);
    printf("|%*s%s%*s|\n", (max_len - header_len) / 2 - 1, " ", this->header, (max_len - header_len) / 2 - 1, " ");

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
#endif

    /**
     * print menu
     */
    va_start(menu_list, this);
    while (end_flag) {
        switch (status) {
            case MENU_INIT:
                printf("%s", direct_info[DIRECT_UPLEFT].key);
                status = MENU_INIT_SEPARATOR;
            case MENU_INIT_SEPARATOR:
                if (line_pos < (max_len + 2 * DFT_MENU_BLANK_WIDTH + 1)) {
                    printf("%s", direct_info[DIRECT_HORIZONTAL].key);
                } else {
                    status = MENU_INIT_END;
                }
                break;
            case MENU_INIT_END:
                printf("%s\n", direct_info[DIRECT_UPRIGHT].key);
                status = MENU_HEAD_START;
            case MENU_HEAD_START:
                line_pos = 0;
                printf("%s", direct_info[DIRECT_VERTICAL].key);
                status = MENU_HEAD;
            case MENU_HEAD:
                line_pos = header_len + (max_len + 2 * DFT_MENU_BLANK_WIDTH - header_len) / 2 + 1;
                printf("\033[1m%*s\033[0m", line_pos, this->header);
                status = MENU_HEAD_END;
            case MENU_HEAD_END:
                if (line_pos < (max_len + 2 * DFT_MENU_BLANK_WIDTH + 1)) {
                    printf(" ");
                    break;
                } else {
                    printf("%s\n", direct_info[DIRECT_VERTICAL].key);
                    status = MENU_LINE_START;
                    line_pos = 0;
                }
            case MENU_LINE_START:
                line_pos = 0;
                printf("%s", direct_info[DIRECT_LEFT].key);
                status = MENU_LINE;
            case MENU_LINE:
                if (line_pos < (max_len + 2 * DFT_MENU_BLANK_WIDTH + 1)) {
                    printf("%s", direct_info[DIRECT_HORIZONTAL].key);
                    break;
                } else {
                    status = MENU_LINE_END;
                }
            case MENU_LINE_END:
                printf("%s\n", direct_info[DIRECT_RIGHT].key);
                status = MENU_CONTENT_START;
            case MENU_CONTENT_START:
                line_pos = 0;
                printf("%s", direct_info[DIRECT_VERTICAL].key);
                menu = va_arg(menu_list, char *);
                if (!menu) {
                    status = MENU_END_START;
                    break;
                } else {
                    status = MENU_PREFIX_BLANK;
                }
            case MENU_PREFIX_BLANK:
                if (line_pos < DFT_MENU_BLANK_WIDTH) {
                    printf(" ");
                    break;
                } else {
                    status = MENU_CONTENT_NUM;
                }
            case MENU_CONTENT_NUM:
                line_pos += number_len(menu_index) + 2;
                printf("%d. ", menu_index++);
                status = MENU_CONTENT;
            case MENU_CONTENT:
                printf("%s", menu);
                line_pos += strlen(menu);
                status = MENU_SUFFIX_BLANK;
            case MENU_SUFFIX_BLANK:
                if (line_pos < ( 2 * DFT_MENU_BLANK_WIDTH + max_len + 1)) {
                    printf(" ");
                    break;
                } else {
                    status = MENU_CONTENT_END;
                }
            case MENU_CONTENT_END:
                printf("%s\n", direct_info[DIRECT_VERTICAL].key);
                status = MENU_CONTENT_START;
                line_pos = 0;
                break;
            case MENU_END_START:
                printf("\b%s", direct_info[DIRECT_DOWNLEFT].key);
                status = MENU_END_SEPARATOR;
            case MENU_END_SEPARATOR:
                if (line_pos < (2 * DFT_MENU_BLANK_WIDTH + max_len + 2)) {
                    printf("%s", direct_info[DIRECT_HORIZONTAL].key);
                } else {
                    status = MENU_END;
                }
                break;
            case MENU_END:
                printf("%s\n", direct_info[DIRECT_DOWNRIGHT].key);
                end_flag = 0;
                break;
            default:
                end_flag = 0;
                break;
        }
        line_pos++;
    }
    va_end(menu_list);
    printf("  Please input your choice (%d - %d): ", this->start_index, menu_index - 1);
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
            if (choices && size && input_index < *size) {
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
    if (size) {
        *size = input_index;
    }

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
            .init       = _init_menu_,
            .show       = _show_menu_,
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

METHOD(table_t, init_table_, int, private_table_t *this, char *header, ...)
{
    va_list list;
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
    this->len += this->col_cnt + 1;
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
    memset(this->fmt,       0, this->len);
    pfmt = this->cur = this->fmt;

    /**
     * print table header
     */
    len = strlen(header);
    if (total_width < len) {
        total_width = len;
    }
#ifndef _WIN32
    printf("\033[1;39m%*s\n", len + (total_width - len) / 2, header);
#else 
    printf("%*s\n", len + (total_width - len) / 2, header);
#endif

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

#ifndef _WIN32
    printf("\033[0m\n");
#else 
    printf("\n");
#endif

    return 0;
}

METHOD(table_t, show_row_, void, private_table_t *this, ...)
{
    va_list list;

    va_start(list, this);
    vprintf(this->fmt, list);
    va_end(list);
    printf("\n");
}

METHOD(table_t, show_column_, void, private_table_t *this, ...)
{
    va_list list;
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
            .init        = _init_table_,
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
    /**
     * color
     */
    COLOR_ID_BLACK  = 0,
    COLOR_ID_RED    ,
    COLOR_ID_GREEN  ,
    COLOR_ID_YELLOW ,
    COLOR_ID_BLUE   ,
    COLOR_ID_PINK   ,
    COLOR_ID_CYAN   ,
    COLOR_ID_WHITE  ,

    /**
     * show
     */
    COLOR_ID_BOLD   ,
    COLOR_ID_UNDERLINE,
    COLOR_ID_SHINE,
    COLOR_ID_INVET,

    COLOR_ID_NORMAL ,
};

typedef enum color_key_t color_key_t;
enum color_key_t {
    /**
     * color
     */
    COLOR_KEY_BLACK  = 'h',
    COLOR_KEY_RED    = 'r',
    COLOR_KEY_GREEN  = 'g',
    COLOR_KEY_YELLOW = 'y',
    COLOR_KEY_BLUE   = 'b',
    COLOR_KEY_PINK   = 'p',
    COLOR_KEY_CYAN   = 'c',
    COLOR_KEY_WHITE  = 'w',

    /**
     * show
     */
    COLOR_KEY_BOLD      = 'B',
    COLOR_KEY_UNDERLINE = 'U',
    COLOR_KEY_SHINE     = 'S',
    COLOR_KEY_INVET     = 'I',

    COLOR_KEY_NORMAL = 'n',
};

static color_info_t clr_info[] = {
    {COLOR_ID_BLACK,     COLOR_KEY_BLACK,     "\033[30m"},
    {COLOR_ID_RED,       COLOR_KEY_RED,       "\033[31m"},
    {COLOR_ID_GREEN,     COLOR_KEY_GREEN,     "\033[32m"},
    {COLOR_ID_YELLOW,    COLOR_KEY_YELLOW,    "\033[33m"},
    {COLOR_ID_BLUE,      COLOR_KEY_BLUE,      "\033[34m"},
    {COLOR_ID_PINK,      COLOR_KEY_PINK,      "\033[35m"},
    {COLOR_ID_CYAN,      COLOR_KEY_CYAN,      "\033[36m"},
    {COLOR_ID_WHITE,     COLOR_KEY_WHITE,     "\033[37m"},
    {COLOR_ID_BOLD,      COLOR_KEY_BOLD,      "\033[1m"},
    {COLOR_ID_UNDERLINE, COLOR_KEY_UNDERLINE, "\033[4m"},
    {COLOR_ID_SHINE,     COLOR_KEY_SHINE,     "\033[5m"},
    {COLOR_ID_INVET,     COLOR_KEY_INVET,     "\033[7m"},
    {COLOR_ID_NORMAL,    COLOR_KEY_NORMAL,    "\033[0m" },
};

#define SWITCH_OPT(key) \
    case COLOR_KEY_##key: \
        color_id = COLOR_ID_##key; \
        break

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
                    case COLOR_KEY_BOLD:
                    case COLOR_KEY_UNDERLINE:
                    case COLOR_KEY_SHINE:
                    case COLOR_KEY_INVET:
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
                    SWITCH_OPT(BLACK);
                    SWITCH_OPT(RED);
                    SWITCH_OPT(GREEN);
                    SWITCH_OPT(YELLOW);
                    SWITCH_OPT(BLUE);
                    SWITCH_OPT(PINK);
                    SWITCH_OPT(CYAN);
                    SWITCH_OPT(WHITE);

                    SWITCH_OPT(BOLD);
                    SWITCH_OPT(UNDERLINE);
                    SWITCH_OPT(SHINE);
                    SWITCH_OPT(INVET);
                    SWITCH_OPT(NORMAL);
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
 * [h] -- black 
 * [r] -- red 
 * [g] -- green 
 * [y] -- yellow 
 * [b] -- blue 
 * [p] -- pink 
 * [c] -- cyan 
 * [w] -- white 
 * [B] -- bold 
 * [U] -- underline 
 * [I] -- invet 
 * [n] -- normal
 *
 * @usage cprintf("[r]red[b][B]blue[n]");
 *
 * @param fmt [in] printf format
 * @param ... [in]
 */
void cprintf(char *fmt, ...)
{
    va_list list;
    char *new_fmt = NULL;
    new_fmt = parser_format(fmt);

    va_start(list, fmt);
    vprintf(new_fmt, list);
    va_end(list);

    free(new_fmt);
}


typedef struct private_progress_t private_progress_t;
struct private_progress_t {
    /**
     * @brief public interface
     */
    progress_t public;

    /**
     * @brief max value of progress
     */
    int max;

    /**
     * @brief progress offset
     */
    int offset;

    /**
     * @brief title of progress
     */
    char *title;

    /**
     * @brief style of progress
     */
    char *style;
};

METHOD(progress_t, init_progress_, void, private_progress_t *this, char *title, int max)
{
    if (title) {
        this->title = title;
    } else {
        this->title = "";
    }
    this->max   = max;
}

METHOD(progress_t, progress_show_, void, private_progress_t *this, int bit)
{
    if (bit > this->max) {
        bit = this->max;
    }

    printf("%s \033[?25l\033[42m\033[1m%*s\033[0m %d%%\033[?25h", this->title, bit, " ", 100 * bit / this->max);
    if (bit >= this->max) {
        printf("\n");
    } else {
        printf("\r");
    }
    fflush(stdout);
}

METHOD(progress_t, progress_destory_, void, private_progress_t *this)
{
    free(this);
}

progress_t *progress_create()
{
    private_progress_t *this = NULL;

    INIT(this,
        .public = {
            .init    = _init_progress_,
            .show    = _progress_show_,
            .destroy = _progress_destory_,
        },
    );

    return &this->public;
}
