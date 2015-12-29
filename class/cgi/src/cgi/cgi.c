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

char *content_type_s[] = {
    "application/x-www-form-urlencoded",
    "multipart/form-data",
};

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

typedef struct cgi_data_t cgi_data_t;
struct cgi_data_t {
    char *name;
    char *value;
};

cgi_data_t *cgi_data_create(char *name, char *value)
{
    cgi_data_t *this; 

    INIT(this, 
        .name  = name,
        .value = value,
    );

    return this;
}

#define cgi_form_data     this->form_data.data
#define cgi_form_data_len this->form_data.data_len
#define cgi_req_method    this->form_data.req_method_type
#define cgi_data_list     this->data_list
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

void cgi_header_location(char *redirect_url)
{
    fprintf(stdout, "Location: %s\r\n\r\n", redirect_url);
}

void cgi_header_status(int status, char *status_msg)
{
    fprintf(stdout, "Status: %d %s\r\n\r\n", status, status_msg);
}

void cgi_header_content_type(char *mime_type)
{
    fprintf(stdout, "Content-type: %s\r\n\r\n", mime_type);
}

/**
 * @brief read_cgi_form_info 
 */
static int read_cgi_form_info()
{
    cgi_get_env("SERVER_SOFTWARE", &cgi_form_info.server_software);
    cgi_get_env("SERVER_NAME", &cgi_form_info.server_name);
    cgi_get_env("SERVER_PROTOCOL", &cgi_form_info.server_protocol);
    cgi_get_env("SERVER_PORT", &cgi_form_info.server_port);
    cgi_get_env("GATEWAY_INTERFACE", &cgi_form_info.gateway_interface);
    cgi_get_env("REQUEST_METHOD", &cgi_form_info.request_method);
    cgi_get_env("PATH_INFO", &cgi_form_info.path_info);
    cgi_get_env("PATH_TRANSLATED", &cgi_form_info.path_translated);
    cgi_get_env("SCRIPT_NAME", &cgi_form_info.script_name);
    cgi_get_env("QUERY_STRING", &cgi_form_info.query_string);
    cgi_get_env("REMOTE_HOST", &cgi_form_info.remote_host);
    cgi_get_env("REMOTE_ADDR", &cgi_form_info.remote_addr);
    cgi_get_env("REMOTE_USER", &cgi_form_info.remote_user);
    cgi_get_env("REMOTE_IDENT", &cgi_form_info.remote_ident);
    cgi_get_env("AUTH_TYPE", &cgi_form_info.auth_type);
    cgi_get_env("CONTENT_TYPE", &cgi_form_info.content_type);
    cgi_get_env("CONTENT_LENGTH", &cgi_form_info.content_length);

    return 0;
}

static void replace_plus_with_blank(char *s)
{
    if (!s) return;

    while (*s != '\0') {
        if (*s == '+') *s = ' ';
        s++;
    }
}

METHOD(cgi_t, get_req_method_, request_method_t, private_cgi_t *this)
{
    if (!strcasecmp(cgi_form_info.request_method, "POST")) cgi_req_method = REQUEST_METHOD_POST;
    else cgi_req_method = REQUEST_METHOD_GET;

    return cgi_req_method;
}

METHOD(cgi_t, get_form_data_, char *, private_cgi_t *this)
{
    int content_len = 0;

    switch (_get_req_method_(this)) {
        case REQUEST_METHOD_GET:
            if (cgi_form_info.query_string) {
                cgi_form_data = strdup(cgi_form_info.query_string);
                cgi_form_data_len = content_len = strlen(cgi_form_data);
            }
            break;
        case REQUEST_METHOD_POST:
            cgi_form_data_len = content_len = !cgi_form_info.content_length ? 0 : atoi(cgi_form_info.content_length);
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

    replace_plus_with_blank(cgi_form_data);
    return cgi_form_data;
}

static char *find_val(private_cgi_t *this, char *name)
{
    int data_list_len = 0;
    cgi_data_t *cgi_data = NULL;

    cgi_data_list->reset_current(cgi_data_list);
    data_list_len = cgi_data_list->get_count(cgi_data_list);
    while (data_list_len-- > 0) {
        if (cgi_data_list->get_next(cgi_data_list, (void **)&cgi_data) == NOT_FOUND) break;
        if (strcmp(cgi_data->name, name)) continue;
        return cgi_data->value;
    }
    return NULL;
}

/**
 * @brief parser_data_by_name 
 * @param data   form data
 * @param name   parameter name 
 * @param output parameter value
 */
char *parser_data_by_name(char *data, char *name, char **output)
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

int parse_post_multipart_input_old(private_cgi_t *this)
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

static int parse_form_comm_input(private_cgi_t *this) 
{
    char *name_end_pos = NULL;
    char *value = NULL;
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
     * get cgi common parameters from form data
     */
    for (pcomm_entry = comm_entry; pcomm_entry->name != NULL; pcomm_entry++) {
        //if (!parser_data_by_name(cgi_form_data, pcomm_entry->name, &cgi_input_buf)) continue;
        value = find_val(this, pcomm_entry->name);
        if (!value) continue;
        if (!(*pcomm_entry->value)) *pcomm_entry->value = strdup(value);
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
    name_end_pos = strstr(cgi_this_file, ".htm");
    if (!name_end_pos) return -1;
    this->form_data.this_file_name = (char *)malloc(name_end_pos - cgi_this_file + 1);
    if (!this->form_data.this_file_name) return -1;
    memcpy(this->form_data.this_file_name, cgi_this_file, name_end_pos - cgi_this_file);

    return 0;
}

static int parse_form_input(private_cgi_t *this, cgi_func_tab_t *func_tab)
{
    char *value = NULL;
    cgi_func_tab_t *pfunc_tab  = func_tab;
    if (!func_tab) return 0;
    if (parse_form_comm_input(this) < 0 && cgi_req_method == REQUEST_METHOD_POST) return -1;

    /**
     * find this file action function callback
     */
    for (pfunc_tab = func_tab; pfunc_tab->name != NULL; pfunc_tab++) {
        if (pfunc_tab->type != KEY_IS_FILE) continue;
        if (strcmp(this->form_data.this_file_name, pfunc_tab->name)) continue;
        break;
    }

    if (!pfunc_tab->name) return -1;
    if (pfunc_tab->set_func_cb) {
    }
    pfunc_tab++;

    /**
     * parser data
     */
    for (; pfunc_tab->name != NULL && pfunc_tab->type != KEY_IS_FILE; pfunc_tab++) {
        if (!pfunc_tab->set_func_cb) continue;
        bzero(cgi_input_buf, DFT_CGI_INPUT_BUF_SIZE);
        bzero(cgi_errmsg_buf, DFT_CGI_ERRMSG_BUF_SIZE);

        value = find_val(this, pfunc_tab->name);
        if (!value) continue;

        if (pfunc_tab->set_func_cb(value, cgi_errmsg_buf, &cgi_form_entry) < 0 && 
            strlen(cgi_errmsg_buf) > 0) {
            ALERT(cgi_errmsg_buf);
            if (pfunc_tab->err_func_cb) pfunc_tab->err_func_cb(cgi_input_buf, cgi_errmsg_buf, &cgi_form_entry); 
            break;
        }
    }

    return 0;
}

static int parse_from_plain_text_input(private_cgi_t *this)
{
    char *pstart = NULL, *pend = NULL;
    cgi_data_t *cgi_data = NULL;
    pstart = pend = cgi_form_data;

    while (*pend != '\0') {
        while (*pend != '\0' && *pend != '=') pend++;
        if (*pend == '\0') break;
        *pend++ = '\0';

        cgi_data = cgi_data_create(pstart, pend);
        cgi_data_list->insert_last(cgi_data_list, cgi_data);

        while (*pend != '\0' && *pend != '&') pend++;
        if (*pend == '\0') break;
        *pend++ = '\0';
        while (*pend != '\0' && *pend == '&') pend++;
        pstart = pend;
    }

    return 0;
}

static int parse_from_multipart_input(private_cgi_t *this)
{
    char *pstart = NULL, *pend = NULL;
    char *value_start = NULL, *value_end = NULL;
    char end_char = 0;
    int file_flag = 0;
    int cgi_data_left_len = cgi_form_data_len;
    cgi_data_t *cgi_data = NULL;

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
    
    while (*pend != '\0') {
        /**
         * cgi formname
         */
        pstart = strstr(pend, "name");
        if (!pstart) break;
        while (*pstart != '\r' && *pstart != '=') pstart++;
        pstart++;
        while (*pstart != '\r' && *pstart == '\"') pstart++;
        pend = pstart + 1;

        /**
         * cgi formname value
         */
        while (*pend != '\"' && *pend != '\r') pend++;
        *pend = '\0';

        /**
         * check whether has filename
         */
        while (*pend != ';' && *pend != '\r') pend++;
        end_char = *pend;
        *pend = '\0';

        /**
         * parse upload file content
         */
        if (end_char == ';') {
            /**
             * cgi form name
             */
            *pend++ = '\0';
            if (!cgi_form_name) cgi_form_name = strdup(pstart);

            /**
             * upload filename
             */
            value_start = pstart = strstr(pend, "filename");
            if (!pstart) break;
            while (*pstart != '\r' && *pstart != '=') pstart++;
            *pstart = '\0';
            pstart++;
            while (*pstart != '\r' && *pstart == '\"') pstart++;
            pend = pstart + 1;

            /**
             * upload filename value
             */
            while (*pend != '\"') pend++;
            value_end = pend;
            *pend = '\0';
            while (*pend != '\r') pend++;

            /**
             * save filename value
             */
            cgi_file_name = strdup(pstart);
            pstart = value_start;

            /**
             * goto file content start position
             */
            while (strncmp(pend, "\r\n", 2)) pend++;
            pend += 2;

            file_flag = 1;
        }

        /**
         * goto file start position
         */
        while (strncmp(pend, "\r\n", 2)) pend++;
        pend += 2;
        while (!strncmp(pend, "\r\n", 2)) pend += 2;

        /**
         * save name and value
         */
        cgi_data = cgi_data_create(pstart, pend);
        cgi_data_list->insert_last(cgi_data_list, cgi_data);

        /**
         * form data value
         */
        value_start = pend;
        cgi_data_left_len = cgi_form_data + cgi_form_data_len - pend; 
        while (cgi_data_left_len-- > 0) {
            if (*pend == '\r') {
                if (!strncmp(pend + 2, cgi_sign_code, cgi_sign_code_len)) {
                    break;
                } else {
                    cgi_data_left_len--;
                }
            }
            pend++;
        }
        *pend = '\0';
        value_end = pend;
        pstart = pend = pend + 2 + cgi_sign_code_len;

        if (cgi_file_size < 1 && file_flag) {
            cgi_file_size = value_end - value_start;
        }
        file_flag = 0;
    }

    return 0;
}

METHOD(cgi_t, parse_form_input_, int, private_cgi_t *this, cgi_func_tab_t *func_tab)
{
    if (!cgi_form_data || cgi_form_data_len < 1) return 0;

    char buf[30240] = {0};
    snprintf(buf, sizeof(buf), "echo \"%s\" > /home/anton/cgi.txt", cgi_form_data);
    if (system(buf)) {}

    /**
     * parse form input data by request method(post, get)
     * 1. post 
     *   1) normal text input;
     *   2) multipart input;
     * 2. get 
     *   1) normal text input
     */
    switch (cgi_req_method) {
        /**
         * request method: post
         */
        case REQUEST_METHOD_POST:
            /**
             * post form data parse by content type
             */
            if (!cgi_form_info.content_type) return -1;

            /**
             * normal plain text type
             */
            if (!strncasecmp(cgi_form_info.content_type, "application/x-www-form-urlencoded", sizeof("application/x-www-form-urlencoded") - 1)) {
                parse_from_plain_text_input(this);
            } 

            /**
             * multipart type
             */
            else if (!strncasecmp(cgi_form_info.content_type, "multipart/form-data", sizeof("multipart/form-data") - 1)) {
                parse_from_multipart_input(this);
            } else {
                return -1;
            }
            break;

        /**
         * request method: get
         */
        case REQUEST_METHOD_GET:
            parse_from_plain_text_input(this);
            break;
        default:
            return -1;
            break;
    }
    if (parse_form_input(this, func_tab) < 0) {
        cgi_header_status(500, "Error reading form data");
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
    if (cgi_data_list) cgi_data_list->destroy(cgi_data_list);
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
            .todo            = NULL,
            .next_file       = NULL,
            .this_file       = NULL,
            .next_path       = NULL,

            .sign_code       = NULL,
            .form_name       = NULL,
            .file_name       = NULL,
            .file_size       = -1,
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

    if (!cgi_data_list || !cgi_input_buf || !cgi_output_buf || !cgi_errmsg_buf) {
        _destroy_(this);
        return NULL;
    }
    CONTENT_TEXT;

    read_cgi_form_info();
    _get_form_data_(this);
    return &this->public;
}
