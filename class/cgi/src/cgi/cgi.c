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
#define DFT_CGI_OUTPUT_BUF_SIZE (4096)
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
#define cgi_this_file     this->form_data.this_file
#define cgi_next_file     this->form_data.next_file
#define cgi_next_path     this->form_data.next_path
#define cgi_form_entry    this->form_data
#define cgi_file_todo     this->form_data.todo
#define cgi_form_type     this->form_data.content_type
#define cgi_form_name     this->form_data.form_name 
#define cgi_file_name     this->form_data.file_name
#define cgi_file_size     this->form_data.file_size
#define cgi_sign_code     this->form_data.sign_code
#define cgi_sign_code_len this->form_data.sign_code_len

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

/**
 * @brief read_cgi_info 
 */
static int read_cgi_info()
{
    cgi_get_env("SERVER_SOFTWARE", &cgi_info.server_software);
    cgi_get_env("SERVER_NAME", &cgi_info.server_name);
    cgi_get_env("SERVER_PROTOCOL", &cgi_info.server_protocol);
    cgi_get_env("SERVER_PORT", &cgi_info.server_port);
    cgi_get_env("GATEWAY_INTERFACE", &cgi_info.gateway_interface);
    cgi_get_env("REQUEST_METHOD", &cgi_info.request_method);
    cgi_get_env("PATH_INFO", &cgi_info.path_info);
    cgi_get_env("PATH_TRANSLATED", &cgi_info.path_translated);
    cgi_get_env("SCRIPT_NAME", &cgi_info.script_name);
    cgi_get_env("QUERY_STRING", &cgi_info.query_string);
    cgi_get_env("REMOTE_HOST", &cgi_info.remote_host);
    cgi_get_env("REMOTE_ADDR", &cgi_info.remote_addr);
    cgi_get_env("REMOTE_USER", &cgi_info.remote_user);
    cgi_get_env("REMOTE_IDENT", &cgi_info.remote_ident);
    cgi_get_env("AUTH_TYPE", &cgi_info.auth_type);
    cgi_get_env("CONTENT_LENGTH", &cgi_info.conent_length);

    return 0;
}

METHOD(cgi_t, get_req_method_, request_method_t, private_cgi_t *this)
{
    if (!strcasecmp(cgi_info.request_method, "POST")) cgi_req_method = REQUEST_METHOD_POST;
    else cgi_req_method = REQUEST_METHOD_GET;

    return cgi_req_method;
}

METHOD(cgi_t, get_form_data_, char *, private_cgi_t *this)
{
    int content_len = 0;

    switch (_get_req_method_(this)) {
        case REQUEST_METHOD_GET:
            if (cgi_info.query_string) {
                cgi_form_data = strdup(cgi_info.query_string);
                cgi_form_data_len = content_len = strlen(cgi_form_data);
            }
            break;
        case REQUEST_METHOD_POST:
            cgi_form_data_len = content_len = !cgi_info.conent_length ? 0 : atoi(cgi_info.conent_length);
            if (!content_len) break;
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

/**
 * @brief parser_data_by_name 
 * @param data   form data
 * @param name   parameter name 
 * @param output parameter value
 */
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

static int upload_file(private_cgi_t *this)
{
    char *pstart = NULL, *pend = NULL;
    int len = 0;
    FILE *fp = NULL;
    char filepath[128] = "/home/anton/ftp/";

    /**
     * get upload file sign code
     */
    pend = pstart = cgi_form_data;
    while (strncmp(pend, "\r\n", 2) && *pend != '\0') pend++;
    cgi_sign_code_len = pend - pstart;
    cgi_sign_code = (char *)malloc(cgi_sign_code_len + 1);
    if (!cgi_sign_code) return -1;
    memcpy(cgi_sign_code, pstart, cgi_sign_code_len);
    cgi_sign_code[cgi_sign_code_len] = '\0';

    /**
     * get form name
     */
    pend = pstart = strcasestr(cgi_form_data, "name");
    if (!pend) return -1;
    pend = pstart + strlen("name=");
    while (*pend == '"' && *pend != '\0') pend++;
    pstart = pend;
    while (*pend != ';' && *pend != '\r' && 
            *pend != '\n' && *pend != '\0' && 
            *pend != '"') pend++;
    len = pend - pstart;
    cgi_form_name = (char *)malloc(len + 1);
    memcpy(cgi_form_name, pstart, len);
    cgi_form_name[len] = '\0';

    /**
     * get upload file name
     */
    pend = pstart = strcasestr(cgi_form_data, "filename");
    if (!pend) return -1;
    pend = pstart + strlen("filename=");
    while (*pend == '"' && *pend != '\0') pend++;
    pstart = pend;
    while (*pend != ';' && *pend !='\r' && 
           *pend != '\n' && *pend != '\0' && 
           *pend != '"') pend++;
    len = pend - pstart;
    cgi_file_name = (char *)malloc(len + 1);
    memcpy(cgi_file_name, pstart, len);
    cgi_file_name[len] = '\0';

    /**
     * goto file start position
     */
    while (strncmp(pend, "\r\n\r\n", 4)) pend++;
    pend += 4;

    /**
     * open file
     */
    strncat(filepath, cgi_file_name, sizeof(filepath) - strlen(filepath) - 1);
    fp = fopen(filepath, "w");
    if (!fp) return -1;

    /**
     * upload file
     */
    len = cgi_form_data + cgi_form_data_len - pend;
    while (len-- > 0) {
        if (*pend != '\r') fputc(*pend, fp);
        else {
            if ((len >= cgi_sign_code_len + 2) && 
                    strncmp(pend + 2, cgi_sign_code, cgi_sign_code_len)) {
                fputc(*pend, fp);
            } else {
                break;
            }

        }
        pend++;
    }
    fclose(fp);
    return 0;
}

METHOD(cgi_t, parse_form_input_, int, private_cgi_t *this, cgi_func_tab_t *func_tab)
{
    char *content_type = CONTENT_TYPE;
    char post_file_name[28] = {0};
    char *name_end_pos = NULL;
    cgi_func_tab_t *p  = func_tab;
    comm_entry_t *pcomm_entry = NULL;
    comm_entry_t comm_entry[] = {
        {"todo",      &cgi_file_todo},
        {"this_file", &cgi_this_file},
        {"next_file", &cgi_next_file},
        {"next_path", &cgi_next_path},
        {NULL}
    };

    if (!cgi_form_data || cgi_form_data_len < 1) return 0;

    /**
     * get cgi content type
     */
    if (content_type != NULL && 
        !strncasecmp(content_type, "multipart/form-data", sizeof("multipart/form-data") - 1)) {
        if (upload_file(this) < 0) {
            ALERT("upload file %s failed!", cgi_file_name);
        } else {
            ALERT("upload file %s succ!", cgi_file_name);
        }
        return 0;
    }

    /**
     * get cgi common parameters from form data
     */
    for (pcomm_entry = comm_entry; pcomm_entry->name != NULL; pcomm_entry++) {
        if (!parser_data_by_name(cgi_form_data, pcomm_entry->name, &cgi_input_buf)) continue;
        if (!(*pcomm_entry->value)) *pcomm_entry->value = strdup(cgi_input_buf);
    }

    /**
     * check this file and next file
     */
    if (!cgi_this_file && cgi_next_file) cgi_this_file = strdup(cgi_next_file);
    if (cgi_this_file && !cgi_next_file) cgi_next_file = strdup(cgi_this_file);
    if (!cgi_this_file && !cgi_next_file) return -1;

    /**
     * get this file name
     */
    if (!func_tab) return -1;
    name_end_pos = strstr(cgi_this_file, ".htm");
    if (!name_end_pos) return -1;
    memcpy(post_file_name, cgi_this_file, name_end_pos - cgi_this_file);

    /**
     * find this file action function callback
     */
    for (p = func_tab; p->name != NULL; p++) {
        if (p->type != KEY_IS_FILE) continue;
        if (strcmp(post_file_name, p->name)) continue;
        break;
    }

    /**
     * parser data
     */
    for (; p->name != NULL && p->type != KEY_IS_FILE; p++) {
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

    return 0;
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

METHOD(cgi_t, write_to_html_, int, private_cgi_t *this, cgi_func_tab_t *func_tab)
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
        return -1;

    if (!(fp = fopen(cgi_next_file, "rb+"))) {
        HTML_GOTO("not_found.htm");
        return -1;
    }

    while (fgets(buf, sizeof(buf), fp) != NULL) {
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
    return 0;
}

METHOD(cgi_t, parser_and_action_, int, private_cgi_t *this, cgi_func_tab_t *func_tab)
{
    _parser_and_action_(this, func_tab);
    _write_to_html_(this, func_tab);
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

static void free_cgi_form_entry(cgi_form_entry_t *entry) 
{
    if (entry->todo)      free(entry->todo);
    if (entry->this_file) free(entry->this_file);
    if (entry->next_file) free(entry->next_file);
    if (entry->next_path) free(entry->next_path);

    if (entry->sign_code) free(entry->sign_code);
    if (entry->form_name) free(entry->form_name);
    if (entry->file_name) free(entry->file_name);
}

METHOD(cgi_t, destroy_, void, private_cgi_t *this)
{
    if (cgi_form_data)  free(cgi_form_data);
    if (cgi_output_buf) free(cgi_output_buf);
    if (cgi_errmsg_buf) free(cgi_errmsg_buf);
    if (form_data_list) form_data_list->destroy(form_data_list);
    free_cgi_form_entry(&this->form_data);

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
            .parse_form_input  = _parse_form_input_,
            .write_to_html     = _write_to_html_,
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
            .todo            = NULL,
            .next_file       = NULL,
            .this_file       = NULL,
            .next_path       = NULL,

            .sign_code       = NULL,
            .form_name       = NULL,
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

    read_cgi_info();
    _get_form_data_(this);
    return &this->public;
}
