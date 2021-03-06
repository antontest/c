#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <utils/utils.h>
#include <xls.h>
#include <libxls/xls.h>
#include <iconv.h>
#include <locale.h>
#include <libxl/libxl.h>

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
    BookHandle wbook;
    SheetHandle wsheet;

    /**
     * mode
     */
    mode_t mode;

    /**
     * col and row count
     */
    int row_cnt;
    int col_cnt;
};

static int open_for_read(private_xls_t *this, const char *file)
{
    this->rbook = xls_open(file, "UTF-8");
    if (!this->rbook) {
        return -1;
    }

    return 0;
}

static int open_for_write(private_xls_t *this, const char *file)
{
    this->wbook = xlCreateBook();
    if (!this->wbook) {
        return -1;
    }

    if (!xlBookLoad(this->wbook, file)) {
        return -1;
    }

    return 0;
}

METHOD(xls_t, open_, int, private_xls_t *this, const char *file, mode_t mode)
{
    if (!file) {
        return -1;
    }

    setlocale(LC_ALL, NULL);
    switch (mode) {
        case O_RDONLY:
            if (open_for_read(this, file) < 0) {
                return -1;
            }
            break;
        case O_WRONLY:
        case O_RDWR:
            if (open_for_read(this, file) < 0 || open_for_write(this, file) < 0) {
                return -1;
            }
            break;
        default:
            return -1;
    }
    this->mode = mode;

    return 0;
}

static int open_sheet_for_read(private_xls_t *this, int sheet_index)
{
    if (sheet_index < 0 || sheet_index > this->rbook->sheets.count) {
        return -1;
    }

    this->rsheet = xls_getWorkSheet(this->rbook, sheet_index);
    if (!this->rsheet) {
        return -1;
    }
    xls_parseWorkSheet(this->rsheet);
    this->row_cnt = this->rsheet->rows.lastrow;
    this->col_cnt = this->rsheet->rows.lastcol;
    return 0;
}

static int open_sheet_for_write(private_xls_t *this, int sheet_index)
{
    if (sheet_index < 0) {
        return -1;
    }

    this->wsheet = xlBookGetSheet(this->wbook, sheet_index);
    if (!this->wsheet) {
        return -1;
    }

    return 0;
}

METHOD(xls_t, open_sheet_, int, private_xls_t *this, int sheet_index)
{
    switch (this->mode) {
        case O_RDONLY:
            if (open_sheet_for_read(this, sheet_index) < 0) {
                return -1;
            }
            break;
        case O_WRONLY:
            if (open_sheet_for_read(this, sheet_index) < 0) {
                return -1;
            }
            xls_close(this->rbook);
            this->rbook = NULL;

            if (open_sheet_for_write(this, sheet_index) < 0) {
                return -1;
            }
            break;
        case O_RDWR:
            if (open_sheet_for_read(this, sheet_index) < 0) {
                return -1;
            }

            if (open_sheet_for_write(this, sheet_index) < 0) {
                return -1;
            }
            break;
        default:
            return -1;
            break;
    }

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

METHOD(xls_t, write_, int, private_xls_t *this, int row, int col, char *data)
{
    if (row < 0 || col < 0) {
        return -1;
    }
    xlSheetWriteStr(this->wsheet, row, col, data, 0);
    return 0;
}

METHOD(xls_t, insert_row_, int, private_xls_t *this, int firstrow, int lastrow)
{
    if (lastrow < firstrow) {
        return -1;
    }

    xlSheetInsertRow(this->wsheet, firstrow, lastrow);
    return 0;
}

METHOD(xls_t, delete_row_, int, private_xls_t *this, int firstrow, int lastrow)
{
    if (lastrow < 0 || firstrow < 0 || lastrow < firstrow) {
        return -1;
    }

    xlSheetRemoveRow(this->wsheet, firstrow, lastrow);
    return 0;
}

METHOD(xls_t, group_row_, int, private_xls_t *this, int firstrow, int lastrow)
{
    if (lastrow < firstrow) {
        return -1;
    }

    xlSheetGroupRows(this->wsheet, firstrow, lastrow, 0);
    return 0;
}

METHOD(xls_t, insert_col_, int, private_xls_t *this, int firstcol, int lastcol)
{
    if (lastcol < firstcol || lastcol < 0) {
        return -1;
    }

    xlSheetInsertCol(this->wsheet, firstcol, lastcol);
    return 0;
}

METHOD(xls_t, delete_col_, int, private_xls_t *this, int firstcol, int lastcol)
{
    if (lastcol < 0 || firstcol < 0 || lastcol < firstcol) {
        return -1;
    }

    xlSheetRemoveCol(this->wsheet, firstcol, lastcol);
    return 0;
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

METHOD(xls_t, save_, int, private_xls_t *this, const char *file)
{
    if (!file) {
        return -1;
    }
    return xlBookSave(this->wbook, file);
}

METHOD(xls_t, destroy_, void, private_xls_t *this)
{
    if (this->rbook) {
        xls_close(this->rbook);
    }
    if (this->wbook) {
        xlBookRelease(this->wbook);
    }
    free(this);
}

METHOD(xls_t, get_row_cnt_, int, private_xls_t *this)
{
    return this->row_cnt;
}

METHOD(xls_t, get_col_cnt_, int, private_xls_t *this)
{
    return this->col_cnt;
}

xls_t* xls_create()
{
    private_xls_t *this;

    INIT(this,
        .public = {
            .open            = _open_,
            .open_sheet      = _open_sheet_,
            .read            = _read_,
            .write           = _write_,
            .save            = _save_,
            .destroy         = _destroy_,

            .insert_row      = _insert_row_,
            .delete_row      = _delete_row_,
            .group_row       = _group_row_,

            .insert_col      = _insert_col_,
            .delete_col      = _delete_col_,

            .enumerate       = _enumerate_,
            .reset_enumerate = _reset_enumerate_,

            .get_row_cnt     = _get_row_cnt_,
            .get_col_cnt     = _get_col_cnt_,
        },
        .rbook  = NULL,
        .rsheet = NULL,
    );

    return &this->public;
}
