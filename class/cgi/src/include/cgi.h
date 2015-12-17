#ifndef __CGI_H__
#define __CGI_H__

#define QUERY_STRING   getenv("QUERY_STRING")
#define REQUEST_METHOD getenv("REQUEST_METHOD")
#define CONTENT_LENGTH atoi(getenv("CONTENT_LENGTH"))

typedef enum request_method_t request_method_t;
enum request_method_t {
    REQUEST_METHOD_GET  = 1 << 1,
    REQUEST_METHOD_POST = 1 << 2,
};

typedef struct data_parser_t data_parser_t;
struct data_parser_t {
    const char *name;
    char *value;
    int (*get_value) ();
    int (*set_value) ();
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
    void (*parser_data) (cgi_t *this, data_parser_t *data);

    /**
     * @brief destroy cgi instance 
     */
    void (*destroy) (cgi_t *this);
};

/**
 * @brief create cgi instance 
 */
cgi_t *cgi_create();

#endif /* __CGI_H__ */
