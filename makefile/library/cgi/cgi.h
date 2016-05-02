#ifndef __CGI_H__
#define __CGI_H__

#define QUERY_STRING   getenv("QUERY_STRING")
#define REQUEST_METHOD getenv("REQUEST_METHOD")
#define CONTENT_TYPE   getenv("CONTENT_TYPE")
#define CONTENT_LENGTH atoi(getenv("CONTENT_LENGTH"))
#define CONTENT_TEXT   printf("Content-type: text/html\r\n\r\n")
#define DOWNLOAD_TEXT  printf("Content-type: application/octet-stream\r\n\r\n")
#define HTML_START     printf("<HTML>")
#define HTML_END       printf("</HTML>")
#define HEAD_START     printf("<HEAD>")
#define HEAD_END       printf("</HEAD>")
#define BODY_START     printf("<BODY>");
#define BODY_END       printf("</BODY>");
#define TITLE(title)   printf("<TITLE>%s</TITLE>", title)
#define H2(h2)         printf("<H2>%s</H2>\n", h2)
#define ALERT(...)     \
    do { \
        printf("<script>alert(\'"); \
        fprintf(stdout, ##__VA_ARGS__); \
        printf("\')</script>"); \
    } while(0)
#define HTML_GOTO(url) printf("<script>location.href=\"%s\";</script>", url)
#define HTTP_REFERER   getenv("HTTP_REFERER")

typedef enum content_type_t content_type_t;
enum content_type_t {
    CONTENT_TYPE_TEXT = 0,
    CONTENT_TYPE_MULTI_PART
};

typedef enum request_method_t request_method_t;
enum request_method_t {
    REQUEST_METHOD_GET    = 0,
    REQUEST_METHOD_POST ,
    REQUEST_METHOD_UNKOWN = -1,
};

typedef enum key_type_t key_type_t;
enum key_type_t {
    KEY_IS_UNKOWN = -1,
    KEY_IS_FILE   = 0,
    KEY_IS_VAR,
};

typedef struct cgi_form_info_t cgi_form_info_t;
struct cgi_form_info_t {
    char *server_software;
    char *server_name;
    char *server_protocol;
    char *server_port;
    char *gateway_interface;
    char *request_method;
    char *path_info;
    char *path_translated;
    char *script_name;
    char *query_string;
    char *remote_host;
    char *remote_addr;
    char *remote_user;
    char *remote_ident;
    char *auth_type;
    char *content_type;
    char *content_length;
    char *multipart_content_type;
} cgi_form_info;

typedef struct cgi_form_entry_t cgi_form_entry_t;
struct cgi_form_entry_t {
    /**
     * cgi form sign code
     */
    char *content_disposition;
    char *sign_code;
    int   sign_code_len;

    /**
     * multipart
     */
    char *form_name;
    char *file_name;
    int  file_size;

    char *content_type;
    char *data;
    int  data_len;
    request_method_t req_method_type;

    /**
     * html common parameter
     */
    char *todo;
    char *this_file;
    char *this_file_name;
    char *next_file;
    char *next_path;
};

typedef struct cgi_func_tab_t cgi_func_tab_t;
struct cgi_func_tab_t {
    char *name;
    key_type_t type;
    int (*get_func_cb) (char *input, char *err_msg, cgi_form_entry_t *entry);
    int (*set_func_cb) (char *output, char *err_msg, cgi_form_entry_t *entry);
    int (*err_func_cb) (char *output, char *err_msg, cgi_form_entry_t *entry);
    int readable;
    int writeable;
};

typedef struct cgi_t cgi_t;
struct cgi_t {
    /**
     * @brief  find value by name
     */
    char* (*find_val) (cgi_t *this, char *name);

    /**
     * @brief parser data 
     */
    int (*parse_input) (cgi_t *this, cgi_func_tab_t *data);

    /**
     * @brief parser data 
     */
    int (*handle_request) (cgi_t *this, cgi_func_tab_t *func_tab);

    /**
     * @brief print error information
     */
    void (*error_print) (cgi_t *this, char *msg); 

    /**
     * @brief alert information 
     */
    void (*alert) (cgi_t *this, const char *fmt, ...);

    /**
     * @brief destroy cgi instance 
     */
    void (*destroy) (cgi_t *this);
};

/**
 * @brief cgi_get_env 
 * @param env    envirnment
 * @param result return result
 */
void cgi_get_env(char *env, char **result);
void html_alert(char *errmsg);

/**
 * cgi header output
 */
void cgi_header_location(char *redirect_url);
void cgi_header_status(int status, char *status_msg);
void cgi_header_content_type(char *mime_type);
void cgi_header_content_length(int len);

/**
 * @brief create cgi instance 
 */
cgi_t *cgi_create();

#endif /* __CGI_H__ */
