#ifndef __URL_H__
#define __URL_H__

typedef int (*write_data_t) (void* buffer, int size, int number, void *stream);

typedef enum form_type_t form_type_t;
enum form_type_t {
    FORM_TEXT = 0,
    FORM_FILE,
};

typedef enum data_type_t data_type_t;
enum data_type_t {
    DATA_TYPE_SELECT = 0,
    DATA_TYPE_INPUT,
    DATA_TYPE_INPUT_CHECKBOX,
    DATA_TYPE_INPUT_HIDDEN,
    DATA_TYPE_INPUT_PASSWORD,
    DATA_TYPE_INPUT_RADIO,
    DATA_TYPE_INPUT_TEXT,
    DATA_TYPE_INPUT_FILE,
    DATA_TYPE_INPUT_BUTTON,
    DATA_TYPE_INPUT_SUBMIT,
};

typedef struct form_data_t form_data_t;
struct form_data_t {
    char *name;
    char *value;
    data_type_t type;
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
     * @usage form_add(curl, FORM_TEXT, "name", "lte", FORM_FILE, "file", "index.htm")
     */
    void (*form_add) (url_t *this, ...);

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
     * @brief  generate request string
     */
    char *(*gen_post_request) (url_t *this);

    /**
     * @brief generate multipart request
     */
    int (*gen_multi_request) (url_t *this);

    /**
     * @brief save file
     */
    void (*save_file) (url_t *this);

    /**
     * @brief clear file
     */
    void (*clear_file) (url_t *this);

    /**
     * @brief destroy instance and free memory
     */
    void (*destroy) (url_t *this);
};

/**
 * @brief create url instance
 */
url_t *url_create();

/**
 * @brief crete html form data
 *
 * @param name   data name 
 * @param value  data value 
 * @param type   data type
 */
form_data_t *form_data_create(const char *name, const char *value, data_type_t type);

/**
 * @brief get value form html line
 *
 * @param buf          data buf
 * @param key          find key 
 * @param value_start  return value start pos 
 * @param value_end    return value end pos
 */
int get_html_value(char *buf, const char *key, char **value_start, char **value_end);
#endif /* __URL_H__ */
