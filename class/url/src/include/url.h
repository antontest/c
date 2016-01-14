#ifndef __URL_H__
#define __URL_H__

typedef int (*write_data_t) (void* buffer, int size, int number, void *stream);

typedef enum form_type_t form_type_t;
enum form_type_t {
    FORM_TEXT = 0,
    FORM_FILE
};

typedef enum data_type_t data_type_t;
enum data_type_t {
    DATA_INPUT = 0,
    DATA_SELECT
};

typedef struct url_t url_t;
struct url_t {
    /**
     * @brief init ssl cert and key
     */
    void (*ssl_init) (url_t *this, const char *cainfo, const char *ssl_cert, const char *ssl_key);

    /**
     * @brief set url
     */
    int (*set_url) (url_t *this, const char *url, const char *save_file);

    /**
     * @brief add form data
     */
    void (*form_add) (url_t *this, form_type_t type, const char *name, const char *value);

    /**
     * @brief send get request 
     */
    int (*get) (url_t *this);

    /**
     * @brief send post request 
     */
    int (*post) (url_t *this, const char *data);

    /**
     * @brief post form data
     */
    int (*form_post) (url_t *this);

    /**
     * @brief parse_form_data
     */
    int (*parse_form_data) (url_t *this);

    /**
     * @brief  get value by key
     */
    char *(*get_value) (url_t *this, char *key);

    /**
     * 
     * @brief  get value by key
     */
    int (*set_value) (url_t *this, char *key, char *value);

    /**
     * @brief list data
     */
    void (*list_data) (url_t *this);

    /**
     * @brief destroy instance and free memory
     */
    void (*destroy) (url_t *this);
};

/**
 * @brief create url instance
 */
url_t *url_create();
#endif /* __URL_H__ */
