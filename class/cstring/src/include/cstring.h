#ifndef __CSTRING_H__
#define __CSTRING_H__

typedef struct cstring cstring;
struct cstring {
    /**
     * @brief   Set string   
     *
     * @param s new string
     * @return  pointer to string
     */
    const char *(*set) (cstring *this, const char *fmt, ...);

    /**
     * @brief   Add string to current string  
     *
     * @param s new string
     * @return  pointer to string
     */
    const char *(*add) (cstring *this, const char *fmt, ...);

    /**
     * @brief Get string  
     */
    const char *(*get) (cstring *this);

    /**
     * @brief Get a piece of string  
     */
    const char *(*offset) (cstring *this, unsigned int start, unsigned int count);

    /**
     * @brief length of string
     */
    int (*length) (cstring *this);

    /**
     * @brief Resize string buffer
     * @return 0, if succ; -1, if failed
     */
    int (*resize) (cstring *this, unsigned int size);

    /**
     * @brief destroy instance and free memory  
     */
    void (*destroy) (cstring *this);

    /**
     * @brief transfrom to int
     */
    int (*toint) (cstring *this);

    /**
     * @brief transfrom to lower char
     */
    const char *(*tolower) (cstring *this);

    /**
     * @brief transfrom to lower char
     */
    const char *(*toupper) (cstring *this);

    /**
     * @brief trim left blank char   
     */
    const char *(*left_trim) (cstring *this);

    /**
     * @brief  trim middle blank char   
     */
    const char *(*mid_trim) (cstring *this);

    /**
     * @brief trim right blank char   
     */
    const char *(*right_trim) (cstring *this);

    /**
     * @brief trim all blank char   
     */
    const char *(*all_trim) (cstring *this);
};

/**
 * @brief Create cstring instance 
 */
cstring *create_cstring(unsigned int size);

#endif /* __CSTRING_H__ */
