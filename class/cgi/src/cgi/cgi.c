#include <cgi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h> 
#include <unistd.h>
#include <utils/utils.h>
#include <../linked_list/linked_list.h>

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
};
#define cgi_form_data     this->form_data.data
#define cgi_form_data_len this->form_data.data_len
#define cgi_req_method    this->form_data.req_method_type
#define form_data_list    this->data_list

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
            break;
        case REQUEST_METHOD_POST:
            content_len = CONTENT_LENGTH;
            cgi_form_data = (char *)malloc(content_len + 1);
            if (!cgi_form_data || content_len < 1) return "\n"; 
            if (fread(cgi_form_data, sizeof(char), content_len, stdin) == content_len) cgi_form_data[content_len] = '\0';
            else fprintf(stderr, "data read error\n");
            break;
        default:
            return "\n";
            break;
    }

    return cgi_form_data;
}

METHOD(cgi_t, parser_data_, void, private_cgi_t *this, cgi_func_tab_t *data)
{
    char *pos = NULL;
    cgi_func_tab_t *p = data;

    if (!cgi_form_data || cgi_form_data_len < 1 || !data) return;
    for (p = data; p->name != NULL; p++) {
        if (!(pos = strstr(cgi_form_data, p->name))) continue;
        pos += strlen(p->name) + 1;
        while (*pos != '\0' && *pos != '&') {
            *(p->value++) = *pos++;
        }
        *p->value++ = '\0';
    }
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
    if (!cgi_form_data) free(cgi_form_data);
    if (!form_data_list) form_data_list->destroy(form_data_list);

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
            .parser_data    = _parser_data_,
            .error_print    = _error_print_,
            .alert          = _alert_,
            .destroy        = _destroy_,
        },
        .form_data = {
            .attr         = NULL,
            .file_name    = NULL,
            .content_type = NULL,
            .data         = NULL,
            .data_len     = 0,
            .req_method_type = REQUEST_METHOD_UNKOWN, 
        },
        .data_list = linked_list_create(),
    );

    return &this->public;
}
