#include <cint.h>
#include <utils/utils.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

typedef struct private_cint_t private_cint_t;
struct private_cint_t {
    /**
     * @brief public interface
     */
    cint_t public;

    /**
     * @brief int array data
     */
    int *data;

    /**
     * @brief data buffer size
     */
    int size;

    /**
     * @brief buffer used
     */
    int len;

    /**
     * @brief position of array
     */
    int *pos;

    /**
     * @brief current position
     */
    int *cur;
};

METHOD(cint_t, add_, int, private_cint_t *this, int n)
{
    if (this->len >= this->size) return -1;
    *this->pos++ = n;
    this->len++;
    return 0;
}

METHOD(cint_t, get_at_, int, private_cint_t *this, int index)
{
    if (index < 0 || index >= this->len) return -1;
    return *(this->data + index);
}

METHOD(cint_t, get_first_, int, private_cint_t *this)
{
    if (this->len <= 0) return -1;
    return *this->data;
}

METHOD(cint_t, get_last_, int, private_cint_t *this)
{
    if (this->len <= 0) return -1;
    return *(this->pos - 1);
}

METHOD(cint_t, insert_, int, private_cint_t *this, int index, int n)
{
    if (index < 0 || this->len >= this->size) return -1;
    if (index >= this->len) return _add_(this, n);
    memcpy(this->data + index + 1, this->data + index, sizeof(int) * (this->len - index));
    *(this->data + index) = n;
    this->pos++;
    this->len++;
    
    return 0;
}

METHOD(cint_t, remove_at_, int, private_cint_t *this, int index)
{
    if (index < 0 || index >= this->len) return -1;
    memcpy(this->data + index, this->data + index + 1, sizeof(int) * (this->len - index));
    this->pos--;
    this->len--;
    if (this->cur > this->pos) this->cur = this->pos;

    return 0;
}

METHOD(cint_t, remove_last_, int, private_cint_t *this)
{
    if (this->len <= 0) return -1;
    *this->pos = -1;
    this->pos--;
    this->len--;
    if (this->cur > this->pos) this->cur = this->pos;

    return 0;
}

METHOD(cint_t, remove_all_, void, private_cint_t *this)
{
    memset(this->data, -1, sizeof(int) * this->size);
    this->len = 0;
    this->pos = this->data;
    this->cur = this->data;
}

METHOD(cint_t, get_size_, int, private_cint_t *this)
{
    return this->size;
}

METHOD(cint_t, reset_size_, int, private_cint_t *this, int new_size)
{
    int *pcint    = NULL;
    int copy_size = 0;

    if (new_size < 0) return -1;
    if (!new_size) {
        free(this->data);
        this->data = NULL;
        this->pos  = NULL;
        this->cur  = NULL;
        this->len  = 0;
    } else {
        pcint = (int *)malloc(sizeof(int) * new_size);
        if (!pcint) return -1;
    
        copy_size = this->len < new_size ? this->len : new_size;
        memcpy(pcint, this->data, sizeof(int) * copy_size);
        if (this->data) free(this->data);
        this->data = pcint;
        this->pos  = this->data + copy_size;
        this->cur  = this->data;
        this->len  = copy_size;
        this->size = new_size;
    }

    return 0;
}

METHOD(cint_t, destroy_, void, private_cint_t *this)
{
    if (this->data) free(this->data);
    this->data = NULL;
    free(this);
}

METHOD(cint_t, reset_enumerate_, void, private_cint_t *this)
{
    this->cur = this->data;
}

METHOD(cint_t, enumerate_, int, private_cint_t *this, int *n)
{
    if (!this->cur || this->len < 1 || this->cur >= this->pos) return 0;
    *n = *this->cur++;
    return 1;
}

METHOD(cint_t, get_length_, int, private_cint_t *this)
{
    return this->len;
}

METHOD(cint_t, print_, void, private_cint_t *this)
{
    int *pint = this->data;
    int len = this->len;

    while (len-- > 0) {
        printf("%d ", *pint++);
    }
    printf("\n");
}

METHOD(cint_t, clone_, cint_t *, private_cint_t *this)
{
    private_cint_t *new = NULL;

    INIT(new, 
        .public = {
            .add         = _add_,
            .get_at      = _get_at_,
            .get_first   = _get_first_,
            .get_last    = _get_last_,
            .insert      = _insert_,
            .remove_at   = _remove_at_,
            .remove_last = _remove_last_,
            .remove_all  = _remove_all_,
            .reset_size  = _reset_size_,
            .clone       = _clone_,
            .destroy     = _destroy_,

            .enumerate   = _enumerate_,
            .get_size    = _get_size_,
            .get_length  = _get_length_,
            .print       = _print_,
            .reset_enumerate = _reset_enumerate_,
        },
        .size = this->size,
        .len  = this->len,
        .data = (int *)malloc(this->size * sizeof(int)),
        .pos  = new->data + this->len,
        .cur  = new->data + (this->cur - this->data),
    );
    memcpy(new->data, this->data, sizeof(int) * this->len);

    return &new->public;
}

cint_t *cint_create(int size)
{
    private_cint_t *this = NULL;

    if (size <= 0) return NULL;

    INIT(this, 
        .public = {
            .add         = _add_,
            .get_at      = _get_at_,
            .get_first   = _get_first_,
            .get_last    = _get_last_,
            .insert      = _insert_,
            .remove_at   = _remove_at_,
            .remove_last = _remove_last_,
            .remove_all  = _remove_all_,
            .reset_size  = _reset_size_,
            .clone       = _clone_,
            .destroy     = _destroy_,

            .enumerate   = _enumerate_,
            .get_size    = _get_size_,
            .get_length  = _get_length_,
            .print       = _print_,
            .reset_enumerate = _reset_enumerate_,
        },
        .size = size,
        .len  = 0,
        .data = (int *)malloc(size * sizeof(int)),
    );

    if (!this->data) {
        free(this);
        return NULL;
    }
    this->pos = this->data;
    this->cur = this->data;

    return &this->public;
}
