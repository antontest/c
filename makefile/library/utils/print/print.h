#ifndef __PRINT_H__
#define __PRINT_H__

#define DFT_MENU_WIDTH             50
#define DFT_MENU_MULTI_SELECTED    0
#define DFT_MENU_START_INDEX       0U
#define DFT_MAX_MENU_SELECTED_ITEM 20
#define DFT_MENU_HEADER            "Menu"
#define DFT_MENU_SEPARATOR         '='

typedef struct menu_t menu_t;
struct menu_t  {
    /**
     * @brief init menu 
     *
     * @param header      [in] header tips
     * @param start_index [in] start index for menu 
     * @param is_support_multi_selected [in] is_support_multi_selected
     */
    void (*init_menu) (menu_t *this, char *header, unsigned int start_index, unsigned int is_support_multi_selected);

    /**
     * @brief show menu
     *
     * @param ... [in] menu
     */
   void (*show_menu) (menu_t *this, ...);
    
    /**
     * @brief get choices from stdin
     *
     * @param choices[] [out]     choices which selected
     * @param size      [in/out]  buffer size of choices; count of choices selected
     *
     * @return 0, if succ; other, if failed.
     */
    int (*get_choice) (menu_t *this, int choices[], int *size);

    /**
     * @brief destroy instance and free memory
     *
     * @param 
     */
    void (*destroy) (menu_t *this);
};

/**
 * @brief create menu instance
 */
menu_t *menu_create();

typedef struct table_t table_t;
struct table_t {
    /**
     * @brief init table
     *
     * @param header [in] header of table
     * @param ... [in] colum infor
     */
    int (*init_table) (table_t *this, char *header, ...);    

    /**
     * @brief show one line
     *
     * @param ... [in] line info
     */
    void (*show_row) (table_t *this, ...);

    /**
     * @brief destroy instance and free memory
     */
    void (*destroy) (table_t *this);
};

/**
 * @brief create table instance
 */
table_t *table_create();

#endif /* __PRINT_H__ */
