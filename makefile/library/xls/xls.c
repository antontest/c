#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <utils/utils.h>
#include <xls.h>
#include <libxls/xls.h>
#include <xlslib/xlslib.h>

typedef struct private_xls_t private_xls_t;
struct private_xls_t {
    /**
     * @brief public interface
     */
    xls_t public;

    /**
     * for xls reading
     */
    xlsWorkBook *rbook;
    xlsWorkSheet *rsheet;
    int cur_row;
    int cur_col;

    /**
     * for xls writing
     */
};

METHOD(xls_t, open_, int, private_xls_t *this, const char *file)
{
    if (!file) {
        return -1;
    }

    this->rbook = xls_open(file, "UTF-8");
    if (!this->rbook) {
        return -1;
    }

    return 0;
}

METHOD(xls_t, open_sheet_, int, private_xls_t *this, int sheet_index)
{
    if (sheet_index < 0 || sheet_index > this->rbook->sheets.count) {
        return -1;
    }

    this->rsheet = xls_getWorkSheet(this->rbook, sheet_index);
    if (!this->rsheet) {
        return -1;
    }
    xls_parseWorkSheet(this->rsheet);

    return 0;
}

METHOD(xls_t, read_, char *, private_xls_t *this, int row, int col)
{
    struct st_row_data *rowdata;

    if (col < 0 || row < 0 || 
        col > this->rsheet->rows.lastcol ||
        row > this->rsheet->rows.lastrow) {
        return NULL;
    }

    rowdata = &this->rsheet->rows.row[row];
    return (char *)rowdata->cells.cell[col].str;
}

METHOD(xls_t, enumerate_, char *, private_xls_t *this)
{
    struct st_row_data *row;
    char   *str;

    if (this->cur_row >= this->rsheet->rows.lastrow) {
        return NULL;
    }
    if (this->cur_col >= this->rsheet->rows.lastcol) {
        this->cur_col = 0;
        this->cur_row++;
    }

    row = &this->rsheet->rows.row[this->cur_row];
    str = (char *)row->cells.cell[this->cur_col++].str;

    if (str) {
        return str;
    } else {
        return "";
    }
}

METHOD(xls_t, reset_enumerate_, void, private_xls_t *this)
{
    this->cur_col = 0;
    this->cur_row = 0;
}

METHOD(xls_t, destroy_, void, private_xls_t *this)
{
    free(this);
}

xls_t* xls_create()
{
    private_xls_t *this;

    INIT(this,
        .public = {
            .open            = _open_,
            .open_sheet      = _open_sheet_,
            .read            = _read_,
            .destroy         = _destroy_,
            .enumerate       = _enumerate_,
            .reset_enumerate = _reset_enumerate_,
        },
    );

    return &this->public;
}
