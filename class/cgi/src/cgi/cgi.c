#include <cgi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <utils/utils.h>

typedef struct private_cgi_t private_cgi_t;
struct private_cgi_t {
    /**
     * @brief public interface  
     */
    cgi_t public; 

    /**
     * @brief request_method
     */
    request_method_t req_type;

    /**
     * @brief cgi data
     */
    char *data;
};
#define req_method this->req_type
#define cgi_data this->data

METHOD(cgi_t, get_req_method_, request_method_t, private_cgi_t *this)
{
    char *method = REQUEST_METHOD;
    if (!strcasecmp(method, "POST")) req_method = REQUEST_METHOD_POST;
    else req_method = REQUEST_METHOD_GET;

    return req_method;
}

METHOD(cgi_t, get_data_, char *, private_cgi_t *this)
{
    int content_len = 0;
    switch (_get_req_method_(this)) {
        case REQUEST_METHOD_GET:
            cgi_data = QUERY_STRING;
            break;
        case REQUEST_METHOD_POST:
            content_len = CONTENT_LENGTH;
            cgi_data = (char *)malloc(content_len + 2);
            if (!cgi_data || content_len < 1) return "\n"; 
            if (fread(cgi_data, sizeof(char), content_len, stdin) == content_len) cgi_data[content_len] = '\0';
            else fprintf(stderr, "data read error\n");
            break;
    }

    return cgi_data;
}

METHOD(cgi_t, parser_data_, void, private_cgi_t *this, data_parser_t *data)
{
    char *pos = NULL;
    data_parser_t *p = data;

    if (!cgi_data || strlen(cgi_data) < 1 || !data) return;
    for (p = data; p->name != NULL; p++) {
        if (!(pos = strstr(cgi_data, p->name))) continue;
        pos += strlen(p->name) + 1;
        while (*pos != '\0' && *pos != '&') {
            *(p->value++) = *pos++;
        }
        *p->value++ = '\0';
    }
}

METHOD(cgi_t, destroy_, void, private_cgi_t *this)
{
    if (!cgi_data) free(cgi_data);
    free(this);
    this = NULL;
}

cgi_t *cgi_create()
{
    private_cgi_t *this;

    INIT(this,
        .public = {
            .get_req_method = _get_req_method_,
            .get_data       = _get_data_,
            .parser_data    = _parser_data_,
            .destroy        = _destroy_,
        },
        .data = NULL,
    );

    return &this->public;
}
