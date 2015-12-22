#define _GNU_SOURCE
#include <cgi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h> 
#include <unistd.h>
#include <fcntl.h>
#include <utils/utils.h>
#include <../linked_list/linked_list.h>

#define DFT_CGI_INPUT_BUF_SIZE  (1024)
#define DFT_CGI_OUTPUT_BUF_SIZE (1024)
#define DFT_CGI_ERRMSG_BUF_SIZE (256)
typedef struct private_cgi_t private_cgi_t;
struct private_cgi_t {
    /**
     * @brief public interface  
     */
    cgi_t public; 

    /**
     * @brief cgi data from brower form
     */
    cgi_form_entry_t form_data;

    /**
     * @brief list of form data
     */
    linked_list_t *data_list;

    /**
     * @brief output to brower
     */
    char *input;
    
    /**
     * @brief output to brower
     */
    char *output;

    /**
     * @brief err_msg buffer 
     */
    char *err_msg;
};
#define cgi_form_data     this->form_data.data
#define cgi_form_data_len this->form_data.data_len
#define cgi_req_method    this->form_data.req_method_type
#define form_data_list    this->data_list
#define cgi_input_buf     this->input
#define cgi_output_buf    this->output
#define cgi_errmsg_buf    this->err_msg
#define cgi_next_file     this->form_data.next_file
#define cgi_this_file     this->form_data.this_file
#define cgi_next_path     this->form_data.next_path
#define cgi_form_entry    this->form_data
#define cgi_file_todo     this->form_data.todo

void html_alert(char *msg)
{
    CONTENT_TEXT;
    HTML_START;
    HEAD_START;
    TITLE("alert");
    HEAD_END;
    BODY_START;
    ALERT(msg);
    BODY_END;
    HTML_END;
}

/**
 * @brief cgi_get_env 
 * @param env    envirnment
 * @param result return result
 */
void cgi_get_env(char *env, char **resulst)
{
    if (!resulst || !env) return;
    *resulst = getenv(env);
    if (!(*resulst)) *resulst = "";
}

METHOD(cgi_t, get_req_method_, request_method_t, private_cgi_t *this)
{
    char *method = REQUEST_METHOD;
    if (!strcasecmp(method, "POST")) cgi_req_method = REQUEST_METHOD_POST;
    else cgi_req_method = REQUEST_METHOD_GET;

    return cgi_req_method;
}

METHOD(cgi_t, get_form_data_, char *, private_cgi_t *this)
{
    int content_len = 0;
    switch (_get_req_method_(this)) {
        case REQUEST_METHOD_GET:
            cgi_form_data = QUERY_STRING;
            cgi_form_data_len = content_len = cgi_form_data != NULL ? strlen(cgi_form_data) : 0;
            break;
        case REQUEST_METHOD_POST:
            content_len   = CONTENT_LENGTH;
            cgi_form_data = (char *)malloc(content_len + 1);

            if (!cgi_form_data || content_len < 1) return "\n"; 
            if (fread(cgi_form_data, sizeof(char), content_len, stdin) == content_len) cgi_form_data[content_len] = '\0';
            else fprintf(stderr, "data read error\n");

            cgi_form_data_len = content_len + 1;
            break;
        default:
            return "\n";
            break;
    }

    return cgi_form_data;
}

static char *parser_data_by_name(char *data, char *name, char **output)
{
    char *p = *output;
    char *pos = NULL;

    if (!(pos = strcasestr(data, name))) return NULL;
    pos += strlen(name) + 1;

    while (*pos != '\0' && *pos != '&') *p++ = *pos++;
    *p++ = '\0';

    return *output;
}

typedef struct comm_entry_t comm_entry_t;
struct comm_entry_t {
    char *name;
    char **value;
};

METHOD(cgi_t, read_action_, void, private_cgi_t *this, cgi_func_tab_t *func_tab)
{
    cgi_func_tab_t *p = func_tab;
    comm_entry_t comm_entry[] = {
        {"todo",      &cgi_file_todo},
        {"this_file", &cgi_this_file},
        {"next_file", &cgi_next_file},
        {"next_path", &cgi_next_path},
        {NULL}
    };
    comm_entry_t *pcomm_entry = NULL;


    if (!cgi_form_data || cgi_form_data_len < 1) return;
    for (pcomm_entry = comm_entry; pcomm_entry->name != NULL; pcomm_entry++) {
        if (!parser_data_by_name(cgi_form_data, pcomm_entry->name, &cgi_input_buf)) continue;
        if (!(*pcomm_entry->value)) *pcomm_entry->value = strdup(cgi_input_buf);
    }

    if (cgi_req_method == REQUEST_METHOD_GET) {
        return;
    }

    if (!func_tab) return;
    for (p = func_tab; p->name != NULL; p++) {
        if (!p->set_func_cb) continue;
        bzero(cgi_input_buf, DFT_CGI_INPUT_BUF_SIZE);
        bzero(cgi_errmsg_buf, DFT_CGI_ERRMSG_BUF_SIZE);

        parser_data_by_name(cgi_form_data, p->name, &cgi_input_buf);
        if (p->set_func_cb(cgi_input_buf, cgi_errmsg_buf, &cgi_form_entry) < 0 && 
            strlen(cgi_errmsg_buf) > 0) {
            ALERT(cgi_errmsg_buf);
            if (p->err_func_cb) p->err_func_cb(cgi_input_buf, cgi_errmsg_buf, &cgi_form_entry); 
            break;
        }
    }
}

METHOD(cgi_t, get_file_todo_, char *, private_cgi_t *this)
{
    if (cgi_form_data_len < 1)
        _get_form_data_(this);
    return cgi_file_todo;
}

METHOD(cgi_t, get_this_file_, char *, private_cgi_t *this)
{
    if (cgi_form_data_len < 1)
        _get_form_data_(this);
    return cgi_this_file;
}

METHOD(cgi_t, get_next_file_, char *, private_cgi_t *this)
{
    if (cgi_form_data_len < 1)
        _get_form_data_(this);
    return cgi_next_file;
}

METHOD(cgi_t, write_action_, void, private_cgi_t *this, cgi_func_tab_t *func_tab)
{
    int  ret       = 0;
    FILE *fp       = NULL;
    char *name     = NULL;
    char buf[1024] = {0};
    char *value_start_pos     = NULL;
    char *value_end_pos       = NULL;
    key_type_t type           = KEY_IS_UNKOWN;
    cgi_func_tab_t *pfunc_tab = func_tab;

    if (!cgi_next_file || strlen(cgi_next_file) < 1) 
        return;

    if (!(fp = fopen(cgi_next_file, "rb+"))) {
        HTML_GOTO("not_found.htm");
        return;
    }

    while (fgets(buf, DFT_CGI_OUTPUT_BUF_SIZE, fp) != NULL) {
        value_start_pos = strstr(buf, "@");
        if (!value_start_pos) {
            printf("%s", buf);
            continue;
        }

        value_end_pos = strstr(value_start_pos + 1, "#");
        if (!value_end_pos) {
            printf("%s", buf);
            continue;
        }

        name = value_start_pos + 1;
        *value_end_pos='\0';
        for (pfunc_tab = func_tab; pfunc_tab != NULL && pfunc_tab->name != NULL; pfunc_tab++) {
            if(!strcasestr(name, pfunc_tab->name)) continue;
            if (!pfunc_tab->get_func_cb) continue;
            ret = pfunc_tab->get_func_cb(cgi_output_buf, cgi_errmsg_buf, &cgi_form_entry);
            type = pfunc_tab->type;
            break;
        }

        if (type != KEY_IS_FILE) {
            *value_start_pos = '\0';
            printf("%s", buf);
        }

        if (ret < 0 && strlen(cgi_errmsg_buf) > 0) this->public.alert(&this->public, cgi_errmsg_buf); 
        else printf("%s", cgi_output_buf);

        if (type != KEY_IS_FILE) {
            printf("%s", value_end_pos + 1);
        }

        bzero(cgi_output_buf, DFT_CGI_OUTPUT_BUF_SIZE);
    }
    fclose(fp);
}

METHOD(cgi_t, parser_and_action_, int, private_cgi_t *this, cgi_func_tab_t *func_tab)
{
    _read_action_(this, func_tab);
    _write_action_(this, func_tab);
    return 0;
}

METHOD(cgi_t, alert_, void, private_cgi_t *this, const char *fmt, ...)
{
    char msg[256] = {0};
    va_list arg;
    
    va_start(arg, fmt);
    vsnprintf(msg, sizeof(msg), fmt, arg);
    va_end(arg);
    ALERT(msg);
}

METHOD(cgi_t, destroy_, void, private_cgi_t *this)
{
    if (!cgi_form_data)  free(cgi_form_data);
    if (!cgi_output_buf) free(cgi_output_buf);
    if (!cgi_errmsg_buf) free(cgi_errmsg_buf);
    if (!form_data_list) form_data_list->destroy(form_data_list);
    if (!cgi_this_file)  free(cgi_this_file);
    if (!cgi_next_file)  free(cgi_next_file);
    if (!cgi_next_path)  free(cgi_next_path);
    if (!cgi_file_todo)  free(cgi_file_todo);

    free(this);
    this = NULL;
}

METHOD(cgi_t, error_print_, void, private_cgi_t *this, char *msg)
{
    CONTENT_TEXT;
    HTML_START;
    HEAD_START;
    TITLE("Error");
    HEAD_END;
    BODY_START;
    H2(msg);
    BODY_END;
    HTML_END;
    
    _destroy_(this);
    exit(0);
}

cgi_t *cgi_create()
{
    private_cgi_t *this;

    INIT(this,
        .public = {
            .get_req_method    = _get_req_method_,
            .get_form_data     = _get_form_data_,
            .read_action       = _read_action_,
            .write_action      = _write_action_,
            .parser_and_action = _parser_and_action_,
            .get_file_todo     = _get_file_todo_,
            .get_this_file     = _get_this_file_,
            .get_next_file     = _get_next_file_,
            .error_print       = _error_print_,
            .alert             = _alert_,
            .destroy           = _destroy_,
        },
        .form_data = {
            .attr            = NULL,
            .next_file       = NULL,
            .this_file       = NULL,
            .file_name       = NULL,
            .content_type    = NULL,
            .data            = NULL,
            .data_len        = 0,
            .req_method_type = REQUEST_METHOD_UNKOWN,
        },
        .data_list = linked_list_create(),
        .input     = (char *)malloc(DFT_CGI_INPUT_BUF_SIZE),
        .output    = (char *)malloc(DFT_CGI_OUTPUT_BUF_SIZE),
        .err_msg   = (char *)malloc(DFT_CGI_ERRMSG_BUF_SIZE),
    );

    if (!form_data_list || !cgi_input_buf || !cgi_output_buf || !cgi_errmsg_buf) {
        _destroy_(this);
        return NULL;
    }
    CONTENT_TEXT;

    return &this->public;
}