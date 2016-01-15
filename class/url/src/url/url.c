#define _GNU_SOURCE
#include <url.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <curl/curl.h>
#include <utils/utils.h>
#include <data/linked_list.h>

typedef struct private_url_t private_url_t;
struct private_url_t {
    /**
     * @brief public interface
     */
    url_t public;

    /**
     * @brief curl
     */
    CURL *curl;

    /**
     * @brief curl code
     */
    CURLcode res;

    /**
     * @brief file handler
     */
    FILE *fp;

    /**
     * @brief form post
     */
    struct curl_httppost *formpost;
    struct curl_httppost *lastptr;

    /**
     * @brief htm file name
     */
    char *file;

    /**
     * @brief flag of saving file
     */
    int save_file;

    /**
     * @brief form data
     */
    linked_list_t *data;

    /**
     * @brief request string
     */
    char *req_str;
};
#define form_data this->data

form_data_t *form_data_create(const char *name, const char *value, data_type_t type)
{
    form_data_t *data = NULL;

    INIT(data, 
        .name  = name ? strdup(name) : NULL,
        .value = value ? strdup(value) : NULL,
        .type  = type,
    );

    return data;
}

METHOD(url_t, init_, int, private_url_t *this)
{
    curl_global_init(CURL_GLOBAL_ALL);
    this->curl = curl_easy_init();
    if (!this->curl) return -1;

    curl_easy_setopt(this->curl, CURLOPT_VERBOSE, 0);
    curl_easy_setopt(this->curl, CURLOPT_HEADER, 0);
    curl_easy_setopt(this->curl, CURLOPT_CONNECTTIMEOUT, 5); /*timeout of connect */
    curl_easy_setopt(this->curl, CURLOPT_TIMEOUT, 10); /* timeout of data recv */
    curl_easy_setopt(this->curl, CURLOPT_FOLLOWLOCATION, 1);
    return 0;
}

METHOD(url_t, ssl_init_, void, private_url_t *this, const char *cainfo, const char *ssl_cert, const char *ssl_key)
{
    if (!cainfo || !ssl_cert || !ssl_key) return;
    curl_easy_setopt(this->curl, CURLOPT_CAINFO, cainfo);
    curl_easy_setopt(this->curl, CURLOPT_SSLCERT, ssl_cert);
    curl_easy_setopt(this->curl, CURLOPT_SSLKEY, ssl_key);
    curl_easy_setopt(this->curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(this->curl, CURLOPT_SSL_VERIFYHOST, 0L);
}

static int write_data (void *buffer, int size, int number, void *stream)
{
    fwrite(buffer, size, number, (FILE *)stream);
    fflush((FILE *)stream);
    return size * number;
}

METHOD(url_t, set_url_, int, private_url_t *this, const char *url, const char *save_file)
{
    curl_easy_setopt(this->curl, CURLOPT_URL, url);
    if (!save_file) return 0;

    this->fp = fopen(save_file, "wb");
    if (!this->fp) return -1;

    if (this->file) {
        if (!this->save_file) unlink(this->file);
        free(this->file);
        this->file = NULL;
    }
    this->file = strdup(save_file);
    curl_easy_setopt(this->curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(this->curl, CURLOPT_WRITEDATA, this->fp);

    return 0;
}

METHOD(url_t, form_add_, void, private_url_t *this, ...)
{
    data_type_t type;
    int curl_form_type = 0;
    char *name = NULL;
    char *value = NULL;
    va_list arg;

    va_start(arg, this);
    while (1) {
        type  = va_arg(arg, data_type_t);
        name  = va_arg(arg, char *);
        value = va_arg(arg, char *);

        switch (type) {
            case FORM_TEXT:
                curl_form_type = CURLFORM_COPYCONTENTS;
                break;
            case FORM_FILE:
                curl_form_type = CURLFORM_FILE;
                break;
            default:
                curl_form_type = -1;
                break;
        }

        if (!name || !value || curl_form_type < 0) break;
        curl_formadd(&this->formpost, &this->lastptr,
            CURLFORM_COPYNAME, name,
            curl_form_type, value,
            CURLFORM_END
        );
    }
    va_end(arg);
}

METHOD(url_t, get_, int, private_url_t *this)
{
    this->res = curl_easy_perform(this->curl);
    if(this->res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(this->res));
        return -1;
    }

    return 0;
}

METHOD(url_t, post_, int, private_url_t *this, const char *data)
{
    if (!data) return -1;

    curl_easy_setopt(this->curl, CURLOPT_POSTFIELDS, data);
    curl_easy_setopt(this->curl, CURLOPT_POST, 1);
    this->res = curl_easy_perform(this->curl);
    if(this->res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(this->res));
        return -1;
    }

    return 0;
}

METHOD(url_t, form_post_, int, private_url_t *this)
{
    curl_easy_setopt(this->curl, CURLOPT_HTTPPOST, this->formpost);
    this->res = curl_easy_perform(this->curl);
    if(this->res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(this->res));
        return -1;
    }

    return 0;
}

int get_html_value(char *buf, const char *key, char **value_start, char **value_end)
{
    char *pos = NULL;
    if (!key || !buf) return -1;

    /**
     * find position of key
     */
    pos = strcasestr(buf, key);
    if (!pos) return -1;

    /**
     * find = pos
     */
    pos = strchr(pos, '=');
    if (!pos) return -1;

    /**
     * goto value start pos
     */
    while (*pos == '=') pos++;
    while (*pos == ' ') pos++;
    if (*pos == '\'' || *pos == '\"') pos++;
    *value_start = pos;

    /**
     * goto value end pos
     */
    while (*pos != '\'' && *pos != '\"' && *pos != ';') pos++;
    *value_end = pos;

    /**
     * if value is blank
     */
    if (*value_end - *value_start == 0) {
        *value_start = NULL;
        *value_end = NULL;
    }

    return 0;
}

METHOD(url_t, parse_form_data_, int, private_url_t *this)
{
    int ret               = -1;
    FILE *fp              = NULL;
    char buf[1024]        = {0};
    char *pos             = NULL;
    char *name_start_pos  = NULL;
    char *name_end_pos    = NULL;
    char *value_start_pos = NULL;
    char *value_end_pos   = NULL;
    char *type_start_pos  = NULL;
    char *type_end_pos    = NULL;
    form_data_t *data     = NULL;
    data_type_t type      = DATA_TYPE_INPUT;

    if (!this->file) return -1;
    fp = fopen(this->file, "rb");
    if (!fp) return -1;

    fseek(fp, 0, SEEK_END);
    if (ftell(fp) < 0) goto over;
    fseek(fp, 0, SEEK_SET);

    /**
     * find form start
     */
    while (fgets(buf, sizeof(buf), fp) != NULL) {
        pos = strcasestr(buf, "<form");
        if (pos) break;
    }
    if (!pos) goto over;

    /**
     * parse htm
     */
    while (fgets(buf, sizeof(buf), fp) != NULL) {
        pos = strcasestr(buf, "</form>");
        if (pos) break;
        pos = strchr(buf, '<');
        if (!pos) continue;

        name_start_pos = NULL;
        name_end_pos = NULL;
        value_start_pos = NULL;
        value_end_pos = NULL;
        if ((pos = strcasestr(buf, "input"))) {
            if (get_html_value(buf, "name", &name_start_pos, &name_end_pos) < 0)
                continue;
            get_html_value(buf, "type", &type_start_pos, &type_end_pos);
            get_html_value(buf, "value", &value_start_pos, &value_end_pos);

            if (name_end_pos) *name_end_pos = '\0';
            if (value_end_pos) *value_end_pos = '\0';
            if (type_end_pos) *type_end_pos = '\0';
            if (type_start_pos) {
                if (!strcasecmp(type_start_pos, "text")) {
                    type = DATA_TYPE_INPUT_TEXT;
                } else if (!strcasecmp(type_start_pos, "hidden")) {
                    type = DATA_TYPE_INPUT_HIDDEN;
                } else if (!strcasecmp(type_start_pos, "radio")) {
                    type = DATA_TYPE_INPUT_RADIO;
                } else if (!strcasecmp(type_start_pos, "password")) {
                    type = DATA_TYPE_INPUT_PASSWORD;
                } else if (!strcasecmp(type_start_pos, "checkbox")) {
                    type = DATA_TYPE_INPUT_CHECKBOX;
                } else if (!strcasecmp(type_start_pos, "button")) {
                    type = DATA_TYPE_INPUT_BUTTON;
                } else if (!strcasecmp(type_start_pos, "submit")) {
                    type = DATA_TYPE_INPUT_SUBMIT;
                } else if (!strcasecmp(type_start_pos, "file")) {
                    type = DATA_TYPE_INPUT_FILE;
                } else {
                    type = DATA_TYPE_INPUT;
                }
            }

            data = form_data_create(name_start_pos, value_start_pos, type);
            form_data->insert_last(form_data, data);

        } else if ((pos = strcasestr(buf, "select"))) {
            if (get_html_value(buf, "name", &name_start_pos, &name_end_pos) < 0)
                continue;
            *name_end_pos = '\0';
            data = form_data_create(name_start_pos, NULL, DATA_TYPE_SELECT);

            while (fgets(buf, sizeof(buf), fp) != NULL) {
                pos = strcasestr(buf, "option");
                if (pos) {
                    get_html_value(buf, "value", &value_start_pos, &value_end_pos);
                    break;
                } else {
                    pos = strcasestr(buf, "</select>");
                    if (pos) break;
                }
            }

            if (value_end_pos) *value_end_pos = '\0';
            if (value_start_pos) data->value = strdup(value_start_pos);
            form_data->insert_last(form_data, data);
        }
    }

    ret = 0;
over:
    if (fp) fclose(fp);
    return ret;
}

METHOD(url_t, list_data_, void, private_url_t *this)
{
    form_data_t *data = NULL;
    int cnt = 0;

    if (!form_data) return;
    cnt = form_data->get_count(form_data);
    while (cnt-- > 0) {
        if (form_data->get_next(form_data, (void **)&data) == NOT_FOUND)
            break;
        printf("name: %s, value: %s\n", data->name, data->value);
    }
}

static int cmp(form_data_t *d1, char *key)
{
    return strcmp(d1->name, key);
}

METHOD(url_t, get_value_, char *, private_url_t *this, char *key)
{
    form_data_t *data = NULL;
    if (form_data->find_first(form_data, (void **)&data, key, (void *)cmp) == NOT_FOUND)
        return NULL;
    return data->value;
}

METHOD(url_t, set_value_, int, private_url_t *this, char *key, char *value)
{
    form_data_t *data = NULL;
    if (form_data->find_first(form_data, (void **)&data, key, (void *)cmp) == NOT_FOUND)
        return -1;

    if (data->value) free(data->value);
    data->value = NULL;
    if (value) data->value = strdup(value);
    return 0;
}

#define DFT_POST_REQUEST_SIZE (2048)
METHOD(url_t, gen_post_request_, char *, private_url_t *this)
{
    form_data_t *data = NULL;
    int data_cnt = 0;
    int len = 0;

    data_cnt = form_data->get_count(form_data);
    if (data_cnt < 1) return NULL;
    if (!this->req_str) this->req_str = (char *)malloc(DFT_POST_REQUEST_SIZE);
    if (!this->req_str) return NULL;

    while (data_cnt-- > 0) {
        if (form_data->get_next(form_data, (void **)&data) == NOT_FOUND)
            break;
        
        if (!data->value || strlen(data->value) < 1) continue;
        //if (data->type == DATA_TYPE_INPUT_RADIO || data->type == DATA_TYPE_INPUT_CHECKBOX || data->type == DATA_TYPE_INPUT_BUTTON) continue;
        if (data->type == DATA_TYPE_INPUT_BUTTON) continue;
        if (strstr(this->req_str, data->name)) continue;
        len += snprintf(this->req_str + len, DFT_POST_REQUEST_SIZE - len, "%s=%s&", data->name, data->value);
    }
    this->req_str[len - 1] = '\0';

    return this->req_str;
}

METHOD(url_t, gen_multi_request_, int, private_url_t *this)
{
    form_data_t *data = NULL;
    int data_cnt = 0;

    data_cnt = form_data->get_count(form_data);
    if (data_cnt < 1) return -1;

    while (data_cnt-- > 0) {
        if (form_data->get_next(form_data, (void **)&data) == NOT_FOUND)
            break;
        
        if (!data->value || strlen(data->value) < 1) continue;
        if (data->type == DATA_TYPE_INPUT_RADIO || data->type == DATA_TYPE_INPUT_CHECKBOX) continue;

        _form_add_(this, data->type == DATA_TYPE_INPUT_FILE ? FORM_FILE : FORM_TEXT, data->name, data->value);
    }

    return 0;

}

METHOD(url_t, save_file_, void, private_url_t *this)
{
    this->save_file = 1;
}

METHOD(url_t, clear_file_, void, private_url_t *this)
{
    this->save_file = 0;
}

METHOD(url_t, destroy_, void, private_url_t *this)
{
    curl_easy_cleanup(this->curl);
    curl_global_cleanup();

    if (form_data) form_data->destroy(form_data);
    if (this->file) {
        if (!this->save_file) unlink(this->file);
        free(this->file);
    }
    if (this->req_str) free(this->req_str);
    free(this);
}

url_t *url_create()
{
    private_url_t *this;
    INIT(this, 
        .public = {
            .ssl_init  = _ssl_init_,

            .get       = _get_,
            .post      = _post_,
            .form_add  = _form_add_,
            .form_post = _form_post_,

            .set_url   = _set_url_,
            .set_value = _set_value_,
            .get_value = _get_value_,
            .list_data = _list_data_,

            .parse_form_data   = _parse_form_data_,
            .gen_post_request  = _gen_post_request_,
            .gen_multi_request = _gen_multi_request_,
            .save_file         = _save_file_,
            .clear_file        = _clear_file_,
            .destroy           = _destroy_,
        },
        .curl    = NULL,
        .fp      = NULL,
        .file    = NULL,
        .req_str = NULL,
        .data    = linked_list_create(),
        .save_file = 0,
    );

    /**
     * curl init
     */
    if (_init_(this) < 0) {
        fprintf(stderr, "curl init failed\n");
        free(this);
        return NULL;
    }

    return &this->public;
}
