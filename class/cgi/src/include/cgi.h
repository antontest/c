#ifndef __CGI_H__
#define __CGI_H__

#define QUERY_STRING   getenv("QUERY_STRING")
#define REQUEST_METHOD getenv("REQUEST_METHOD")
#define CONTENT_LENGTH atoi(getenv("CONTENT_LENGTH"))
#define CONTENT_TEXT   printf("Content-type: text/html\r\n\r\n")
#define HTML_START     printf("<HTML>")
#define HTML_END       printf("</HTML>")
#define HEAD_START     printf("<HEAD>")
#define HEAD_END       printf("</HEAD>")
#define BODY_START     printf("<BODY>");
#define BODY_END       printf("</BODY>");
#define TITLE(title)   printf("<TITLE>%s</TITLE>", title)
#define H2(h2)         printf("<H2>%s</H2>\n", h2)
#define ALERT(msg)     printf("<script>alert(\"%s\")</script>", msg)
#define HTML_GOTO(url)       printf("<script>location.href=\"%s\";</script>", url)

typedef enum request_method_t request_method_t;
enum request_method_t {
    REQUEST_METHOD_UNKOWN = -1,
    REQUEST_METHOD_GET    = 1 << 1,
    REQUEST_METHOD_POST   = 1 << 2,
};

typedef struct cgi_func_tab_t cgi_func_tab_t;
struct cgi_func_tab_t {
    const char *name;
    char *value;
    int (*get_func_cb) (char *input, char *err_msg);
    int (*set_func_cb) (char *output, char *err_msg);
};

typedef struct cgi_form_entry_t cgi_form_entry_t;
struct cgi_form_entry_t {
    char *attr;
    char *file_name;
    char *content_type;
    char *data;
    int  data_len;
    request_method_t req_method_type;
};

typedef struct cgi_t cgi_t;
struct cgi_t {
    /**
     * @brief request method 
     */
    request_method_t (*get_req_method) (cgi_t *this);

    /**
     * @brief get data from brower  
     * @return data of form
     */
    char* (*get_data) (cgi_t *this);

    /**
     * @brief parser data 
     */
    void (*parser_data) (cgi_t *this, cgi_func_tab_t *data);

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

/**
 * @brief create cgi instance 
 */
cgi_t *cgi_create();

#endif /* __CGI_H__ */
