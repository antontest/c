#define _GNU_SOURCE
#include <cgi.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h> 
#include <unistd.h>
#include <fcntl.h>
#include <utils/utils.h>
#include <linked_list.h>
#include <ctype.h>

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

    /**
     * bundary for multipart form
     */
    char boundary[512];
    int boundary_len; 
    /*first boundary len: 27 '-' + numers + \r\n*/
    /*last boundary len: \r\n + 29 '-' + numers + 2 '-' + \r\n*/
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
        .name     = name,
        .value    = value,
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

typedef enum cgi_state_t cgi_state_t;
enum cgi_state_t {
    s_init = 0,
    s_first_boundary_start,
    s_first_boundary_end,
    s_head_filed_start,
    s_name_field_start,
    s_name_field,
    s_name,
    s_name_end,
    s_filename_start,
    s_filename,
    s_filename_end,
    s_value_field_start,
    s_value_field,
    s_value_field_end,
    s_content_type_start,
    s_content_type,
    s_content_type_end,
    s_applicate_data_start,
    s_applicate_data,
    s_applicate_data_end,
    s_data_boundary_start,
    s_data_boundary,
    s_data_boundary_end,
    s_last_boundary,
    s_uninit
};

int multipart_parse_execute(private_cgi_t *this)
{
    char *pos          = NULL;
    char *name         = NULL;
    char *value        = NULL;
    char *filename     = NULL;
    char *content_type = NULL;
    int filesize       = -1;
    cgi_data_t *data   = NULL;
    cgi_state_t state  = s_init;

    pos = cgi_form_data;
    while (*pos != '\0') {
        switch (state) {
            case s_init:
                if (*pos == '-') {
                    state = s_first_boundary_start;
                } else {
                    break;
                }
            case s_first_boundary_start:
                if (*pos == '\r') {
                    state = s_first_boundary_end;
                }
                break;
            case s_first_boundary_end:
                if (*pos != '\n') {
                    return pos - cgi_form_data;
                }
                state = s_name_field_start;
                break;
            case s_head_filed_start:
                if (*pos == ';') {
                    state = s_name_field_start;
                }
                break;
            case s_name_field_start:
                if (*pos == '=') {
                    state = s_name_field;
                }
                break;
            case s_name_field:
                if (*pos != '\"') {
                    return pos - cgi_form_data;
                }
                name = ++pos;
                state = s_name;
            case s_name:
                if (*pos == '\"') {
                    state = s_name_end;
                    *pos = '\0';
                }
                break;
            case s_name_end:
                if (*pos == '\r') {
                    state = s_value_field_start;
                    *pos = '\0';
                } else if (*pos == ';') {
                    cgi_form_name = strdup(name);
                    pos++;
                    while (*pos == ' ') {
                        pos++;
                    }
                    name = pos;
                    state = s_filename_start;
                }
                break;
            case s_filename_start:
                if (*pos == '=') {
                    *pos = '\0';
                    state = s_filename;
                }
                break;
            case s_filename:
                if (*pos != '\"') {
                    return pos - cgi_form_data;
                }
                if (*pos == ' ') {
                    pos++;
                }
                filename = pos + 1;
                state = s_filename_end;
                break;
            case s_filename_end:
                if (*pos == '\"') {
                    *pos++ = '\0';
                    cgi_file_name = strdup(filename);
                    state = s_content_type_start;
                    filename = NULL;
                }
                break;
            case s_content_type_start:
                if (*pos == ':') {
                    if (*pos == ' ') {
                        pos++;
                    }
                    content_type = pos;
                    state = s_content_type;
                }
                break;
            case s_content_type:
                if (*pos == '\r') {
                    *pos = '\0';
                    cgi_form_info.multipart_content_type = strdup(content_type);
                    state = s_content_type_end;
                    //ALERT("content_type: %s", content_type);
                }else {
                    break;
                }
            case s_content_type_end:
                if (!cgi_form_info.multipart_content_type || 
                    (cgi_form_info.multipart_content_type && 
                    strstr(cgi_form_info.multipart_content_type, "text"))) {
                    state = s_value_field_start;
                } else {
                    state = s_applicate_data_start;
                }
                break;
            case s_applicate_data_start:
                pos += 2;
                if (*pos != '\n') {
                    return pos - cgi_form_data;
                }
                state = s_applicate_data;
                break;
            case s_applicate_data:
                value = pos;
                cgi_file_size = cgi_form_data + cgi_form_data_len - pos - this->boundary_len - 4;
                ALERT("filesize: %d, filename: %s", cgi_file_size, cgi_file_name);
                ALERT("path_info: %s", cgi_form_info.path_info);
                state = s_applicate_data_end;
                *(pos + filesize + 1) = '\0';
                break;
            case s_applicate_data_end:
                data = cgi_data_create(name, value);
                cgi_data_list->insert_last(cgi_data_list, data);
                state = s_uninit;
                break;
            case s_value_field_start:
                pos += 2;
                if (*pos != '\n') {
                    return pos - cgi_form_data;
                }
                state = s_value_field;
                break;
            case s_value_field:
                value = pos;
                state = s_value_field_end;
                break;
            case s_value_field_end:
                if (*pos == '\r') {
                    state = s_data_boundary_start;
                }
                break;
            case s_data_boundary_start:
                if (*pos == '\n') {
                    state = s_data_boundary;
                }
                break;
            case s_data_boundary:
                if (*pos == '-') {
                    *(pos - 2) = '\0';
                    
                    data = cgi_data_create(name, value);
                    cgi_data_list->insert_last(cgi_data_list, data);
                    if (strcmp(name, "filename")) {
                        //ALERT("name: %s, value: %s", name, value);
                    } else {
                        cgi_file_size = pos - 2 -value;
                        ALERT("filesize: %d", cgi_file_size);
                    }
                    name = NULL;
                    value = NULL;

                    state = s_data_boundary_end;
                } else {
                    state = s_value_field_end;
                }
                break;
            case s_data_boundary_end:
                if (*pos == '\r') {
                    state = s_head_filed_start;
                }
                break;
            case s_uninit:
            default:
                break;

        }
        pos++;
    }

    return 0;
}

void html_alert(char *msg)
{
    CONTENT_TEXT;
    HTML_START;
    HEAD_START;
    TITLE("alert");
    HEAD_END;
    BODY_START;
    ALERT("%s", msg);
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

void cgi_header_content_length(int len)
{
    fprintf(stdout, "Content-Length: %d\r\n", len);
}

/**
 * @brief read_cgi_form_info 
 */
static int get_cgi_env_info(private_cgi_t *this)
{
    char *boundary = NULL;
    char *multipart = NULL;

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
    
    multipart = strstr(cgi_form_info.content_type, "multipart/form-data");
    if (multipart) {
        multipart += strlen("multipart/form-data");
        boundary = strstr(multipart, "boundary=");
        if (boundary) {
            boundary += strlen("boundary=");
            snprintf(this->boundary, sizeof(this->boundary), "%s", boundary);
            this->boundary_len = strlen(boundary) + 2 + 2;
        }
    }

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

METHOD(cgi_t, find_val_, char *, private_cgi_t *this, char *name)
{
    return find_val(this, name);
}

METHOD(cgi_t, get_req_method_, request_method_t, private_cgi_t *this)
{
    if (!strcasecmp(cgi_form_info.request_method, "POST")) cgi_req_method = REQUEST_METHOD_POST;
    else cgi_req_method = REQUEST_METHOD_GET;

    return cgi_req_method;
}

METHOD(cgi_t, get_data_, char *, private_cgi_t *this)
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

            if (fread(cgi_form_data, sizeof(char), content_len, stdin) == content_len) cgi_form_data[content_len] = '\0';
            else fprintf(stderr, "data read error\n");
            break;
        default:
            return "\n";
            break;
    }

    replace_plus_with_blank(cgi_form_data);
    return cgi_form_data;
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

static int parse_comm_data(private_cgi_t *this) 
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
        if (!(*pcomm_entry->value)) {
            if (!strcmp(pcomm_entry->name, "next_path")) {
                char *p = strstr(value, "//");
                if (!p) p = value;
                else {
                    p = strchr(p + 2, '/');
                    if (p) p++;
                }
                *pcomm_entry->value = strdup(p);
            }
            else
                *pcomm_entry->value = strdup(value);
        }
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
    this->form_data.this_file_name[name_end_pos - cgi_this_file] = '\0';

    return 0;
}

static int handle_parse_data(private_cgi_t *this, cgi_func_tab_t *func_tab)
{
    char *value = NULL;
    cgi_func_tab_t *pfunc_tab  = func_tab;

    if (!func_tab) return 0;

    /**
     * if upload binary files
     */
    if (strstr(cgi_form_info.content_type, "multipart") &&  
        (cgi_form_info.multipart_content_type && 
        !strstr(cgi_form_info.multipart_content_type, "text"))) {
        cgi_func_tab_t *pfunc = func_tab;

        ALERT("text");
        char *file = find_val(this, "filename");
        if (file) {
            while (pfunc && pfunc->name) {
                if (!strcmp(pfunc->name, "filename")) {
                    if (pfunc->get_func_cb) {
                        pfunc->get_func_cb(file, cgi_errmsg_buf, &cgi_form_entry);
                        break;
                    }
                }
                pfunc++;
            }
        }
        
        return 0;
    }

    /**
     * upload other type files not binary files
     */
    if (parse_comm_data(this) < 0 && cgi_req_method == REQUEST_METHOD_POST) {
        return -1;
    }

    /**
     * find this file action function callback
     */
    for (pfunc_tab = func_tab; pfunc_tab->name != NULL; pfunc_tab++) {
        if (pfunc_tab->type != KEY_IS_FILE) continue;
        if (strcmp(this->form_data.this_file_name, pfunc_tab->name)) continue;
        if (pfunc_tab->get_func_cb) {
            pfunc_tab->get_func_cb(cgi_this_file, cgi_errmsg_buf, &cgi_form_entry);
        }
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
            ALERT("%s", cgi_errmsg_buf);
            if (pfunc_tab->err_func_cb) pfunc_tab->err_func_cb(cgi_input_buf, cgi_errmsg_buf, &cgi_form_entry); 
            break;
        }
    }

    return 0;
}

/*
static int parse_plain_text(private_cgi_t *this)
{
    char *pstart = NULL, *pend = NULL;
    cgi_data_t *cgi_data = NULL;
    pstart = pend = cgi_form_data;

    ALERT("cgi_form_data: %s", cgi_form_data);
    while (*pend != '\0') {
        while (*pend != '\0' && *pend != '=') pend++;
        if (*pend == '\0') break;
        *pend++ = '\0';

        cgi_data = cgi_data_create(pstart, pend);
        cgi_data_list->insert_last(cgi_data_list, cgi_data);

        ALERT("name: %s, value: %s", pstart, pend);
        while (*pend != '\0' && *pend != '&') pend++;
        if (*pend == '\0') break;
        *pend++ = '\0';
        while (*pend != '\0' && *pend == '&') pend++;
        pstart = pend;
    }

    return 0;
}
*/

typedef enum plain_text_state_t plain_text_state_t;
enum plain_text_state_t  {
    s_plain_name_start = 0x100,
    s_plain_name,
    s_plain_name_end,
    s_plain_value_start,
    s_plain_value, 
    s_plain_value_end
};

static int parse_plain_text(private_cgi_t *this)
{
    char *name = NULL, *value = NULL;
    char *pos = cgi_form_data;
    cgi_data_t *data = NULL;
    plain_text_state_t state = s_plain_name_start;

    /**
     * check data form brower whether is null
     */
    if (!pos || *pos == '\0') {
        ALERT("There is no data.");
        return -1;
    }

    /**
     * parse plain text
     */
    while (*pos != '\0') {
        switch (state) {
            case s_plain_name_start:
                name = pos;
                state = s_plain_name;
            case s_plain_name:
                if (*pos == '=') {
                    *pos = '\0';
                    state = s_plain_name_end;
                } else {
                    break;
                }
            case s_plain_name_end:
                state = s_plain_value_start;
                break;
            case s_plain_value_start:
                value = pos;
                state = s_plain_value;
                break;
            case s_plain_value:
                if (*pos == '&') {
                    *pos = '\0';
                    state = s_plain_value_end;
                } else {
                    break;
                }
            case s_plain_value_end:
                data = cgi_data_create(name, value);
                cgi_data_list->insert_last(cgi_data_list, data);

                //ALERT("name: %s, value: %s", name, value);
                name = NULL;
                value = NULL;
                state = s_plain_name_start;
                break;
            default:
                break;
        }
        pos++;
    }

    /**
     * last parameter
     */
    if (name != NULL && value != NULL) {
        data = cgi_data_create(name, value);
        cgi_data_list->insert_last(cgi_data_list, data);
        //ALERT("name: %s, value: %s", name, value);
    }

    return 0;
}

static int parse_from_multipart_input(private_cgi_t *this)
{
    char *pos = cgi_form_data;
    char *name = NULL, *value = NULL;
    char *filename = NULL, *file_start_pos = NULL;
    cgi_data_t *data;

    pos += this->boundary_len;
    while (*pos != '\0') {
        /**
         * parse name
         */
        pos = strstr(pos, "name");
        if (pos) {
            name = pos = pos + strlen("name=\"");
        } else {
            break;
        }
        while (*pos != '\"') {
            pos++;
        }
        *pos++ = '\0';

        /**
         * parse value
         */
        filename = strstr(pos, "filename");
        if (filename) {
            /**
             * save form name
             */
            cgi_form_name = strdup(name);

            /**
             * save filename
             */
            name = filename;
            pos = filename += strlen("filename=\"");
            *(name + strlen("filename")) = '\0';
            while (*pos != '\"') {
                pos++;
            }
            *pos = '\0';
            pos += 2;

            /**
             * insert name and value into list
             */
            /*
            data = cgi_data_create(name, name);
            cgi_data_list->insert_last(cgi_data_list, data);
            */
            if (!cgi_file_name) {
                cgi_file_name = strdup(filename);
            }

            pos = strstr(pos, "Content-Type");
            if (!pos) {
                break;
            }
        }

        pos = strstr(pos, "\r\n\r\n");
        if (pos) {
            value = pos + 4;
            pos = value;
        } else {
            break;
        }
        
        if (filename) {

            file_start_pos = pos;
            //while ((pos = strstr(pos, "\r"))) {
            while (*pos != '\0') {
                if (*pos == '\r') {
                if (!strncmp(pos + 4, this->boundary, this->boundary_len - 4)) {
                    cgi_file_size = pos - file_start_pos;
                    break;
                }
                }
                pos += 1;
            }
            filename = NULL;
        } else {
            pos = strstr(pos, "\r\n");   
        }

        /**
         * check value
         */
        if (pos) {
            *pos = '\0';
            pos += 2;
        } else {
            break;
        }

        /**
         * inser name and value into list
         */
        /*
        if (strcmp(name, "filename")) {
            ALERT("name: %s, value: %s", name, value);
        }
        */
        data = cgi_data_create(name, value);
        cgi_data_list->insert_last(cgi_data_list, data);

        /**
         * next boundary
         */
        pos += this->boundary_len;
    }
    //ALERT("filename: %s, filesize: %d", cgi_file_name, cgi_file_size);

    return 0;
}

METHOD(cgi_t, parse_input_, int, private_cgi_t *this, cgi_func_tab_t *func_tab)
{
    if (!cgi_form_data || cgi_form_data_len < 1) return 0;

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
                parse_plain_text(this);
            } 

            /**
             * multipart type
             */
            else if (!strncasecmp(cgi_form_info.content_type, "multipart/form-data", strlen("multipart/form-data"))) {
                //parse_from_multipart_input(this);
                multipart_parse_execute(this);
            } else {
                return -1;
            }
            break;

        /**
         * request method: get
         */
        case REQUEST_METHOD_GET:
            parse_plain_text(this);
            break;
        default:
            return -1;
            break;
    }

    handle_parse_data(this, func_tab);

    return 0;
}

METHOD(cgi_t, handle_request_, int, private_cgi_t *this, cgi_func_tab_t *func_tab)
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
        HTML_GOTO("not_found.html");
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
            if(strcmp(name, pfunc_tab->name)) continue;
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

METHOD(cgi_t, alert_, void, private_cgi_t *this, const char *fmt, ...)
{
    char msg[256] = {0};
    va_list arg;
    
    va_start(arg, fmt);
    vsnprintf(msg, sizeof(msg), fmt, arg);
    va_end(arg);
    ALERT("%s", msg);
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
    if (cgi_form_info.multipart_content_type) {
        free(cgi_form_info.multipart_content_type);
    }
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
    char *p = NULL;

    INIT(this,
        .public = {
            .parse_input    = _parse_input_,
            .handle_request = _handle_request_,
            .find_val       = _find_val_,
            .error_print    = _error_print_,
            .alert          = _alert_,
            .destroy        = _destroy_,
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
        .boundary  = {0},
    );

    if (!cgi_data_list || !cgi_input_buf || !cgi_output_buf || !cgi_errmsg_buf) {
        _destroy_(this);
        return NULL;
    }

    get_cgi_env_info(this);
    _get_data_(this);
    return &this->public;
}
