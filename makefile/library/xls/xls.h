#ifndef __XLS_H__
#define __XLS_H__

typedef struct xls_t xls_t;
struct xls_t {
    /**
     * @brief open xls file, if not exist, then create it
     */
    int (*open) (xls_t *this, const char *file);

    /**
     * @brief open work sheet by index
     */
    int (*open_sheet) (xls_t *this, int sheet_index);

    /**
     * @brief  read data from xls
     */
    char *(*read) (xls_t *this, int row, int col);

    /**
     * @brief  enumerate xls data
     */
    char *(*enumerate) (xls_t *this);
    void (*reset_enumerate) (xls_t *this);

    /**
     * @brief destroy instance and free memory
     */
    void (*destroy) (xls_t *this);
};

/**
 * @brief create xls instance
 */
xls_t* xls_create();

#endif /* __XLS_H__ */
