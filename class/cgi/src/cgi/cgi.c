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

METHOD(cgi_t, get_data_, char *, private_cgi_t *this)
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

METHOD(cgi_t, read_back_, void, private_cgi_t *this, cgi_func_tab_t *data)
{
    cgi_func_tab_t *p = data;
    comm_entry_t comm_entry[] = {
        {"next_file", &cgi_next_file},
        {"this_file", &cgi_this_file},
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

    if (!data) return;
    for (p = data; p->name != NULL; p++) {
        if (!p->set_func_cb) continue;
        bzero(cgi_input_buf, DFT_CGI_INPUT_BUF_SIZE);
        bzero(cgi_errmsg_buf, DFT_CGI_ERRMSG_BUF_SIZE);

        parser_data_by_name(cgi_form_data, p->name, &cgi_input_buf);
        if (p->set_func_cb(cgi_input_buf, cgi_errmsg_buf) < 0 && 
            strlen(cgi_errmsg_buf) > 0) {
            break;
        }
    }
}

METHOD(cgi_t, get_next_file_, char *, private_cgi_t *this)
{
    if (cgi_form_data_len < 1)
        _get_data_(this);
    return cgi_next_file;
}

METHOD(cgi_t, write_back_, void, private_cgi_t *this, cgi_func_tab_t *data)
{
    int  ret   = 0;
    FILE *fp   = NULL;
    char *name = NULL;
    char *pos  = NULL;
    char *value_start_pos = NULL;
    char *value_end_pos   = NULL;
    char value_end_char   = 0;
    char buf[1024] = {0};
    var_type_t type = VAR_IS_UNKOWN;
    cgi_func_tab_t *pdata = data;

    if (!cgi_next_file || strlen(cgi_next_file) < 1) 
        return;

    if (!(fp = fopen(cgi_next_file, "rb+"))) {
        HTML_GOTO("not_found.htm");
        return;
    }

    while (fgets(buf, DFT_CGI_OUTPUT_BUF_SIZE, fp) != NULL) {
        pos = strcasestr(buf, "value");
        if (!pos) {
            printf("%s", buf);
            continue;
        }

        pos += sizeof("value");
        while (*pos == ' ') pos++;
        while (*pos == '=') pos++;
        while (*pos == ' ') pos++;
        while (*pos == '\"') pos++;
        if (*pos != '@') {
            printf("%s", buf);
            continue;
        }

        name = value_start_pos = pos;
        while (*pos != '\"' && *pos != '\0' && *pos != '>') pos++;
        value_end_char = *pos;
        value_end_pos = pos;
        *pos='\0';
        for (pdata = data; pdata != NULL && pdata->name != NULL; pdata++) {
            if(!strcasestr(name, pdata->name)) continue;
            if (!pdata->get_func_cb) continue;
            ret = pdata->get_func_cb(cgi_output_buf, cgi_errmsg_buf);
            type = pdata->type;
            break;
        }

        if (type != VAR_IS_FILE) {
            *value_start_pos = '\0';
            printf("%s", buf);
        }

        if (ret < 0) this->public.alert(&this->public, cgi_errmsg_buf); 
        else printf("%s", cgi_output_buf);

        if (type != VAR_IS_FILE) {
            *value_end_pos = value_end_char;
            printf("%s", value_end_pos);
        }

        bzero(cgi_output_buf, DFT_CGI_OUTPUT_BUF_SIZE);
    }
    fclose(fp);
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
    if (!cgi_next_file)  free(cgi_next_file);
    if (!cgi_this_file)  free(cgi_this_file);

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
            .get_req_method = _get_req_method_,
            .get_data       = _get_data_,
            .read_back      = _read_back_,
            .write_back     = _write_back_,
            .get_next_file  = _get_next_file_,
            .error_print    = _error_print_,
            .alert          = _alert_,
            .destroy        = _destroy_,
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
