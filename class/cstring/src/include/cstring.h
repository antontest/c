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
    char *(*set) (cstring *this, const char *fmt, ...);

    /**
     * @brief   Add string to current string  
     *
     * @param s new string
     * @return  pointer to string
     */
    char *(*add) (cstring *this, const char *fmt, ...);

    /**
     * @brief   Insert string to current string  
     *
     * @param index  start deleting number
     * @param fmt new string
     * @return    pointer to string
     */
    char *(*insert) (cstring *this, unsigned int index, const char *fmt, ...);

    /**
     * @brief delete string  
     *
     * @param index  start deleting number
     * @param count  delete count
     */
    char *(*delete) (cstring *this, unsigned int index, unsigned int count);

    /**
     * @brief Get string  
     */
    char *(*get) (cstring *this);

    /**
     * @brief Get a piece of string  
     *
     * @param count  left charcter count 
     */
    char *(*left) (cstring *this, unsigned int count);

    /**
     * @brief Get a piece of string  
     */
    char *(*mid) (cstring *this, unsigned int start, unsigned int count);

    /**
     * @brief Get a piece of string  
     *
     * @param count  right charcter count 
     */
    char *(*right) (cstring *this, unsigned int count);

    /**
     * @brief length of string
     */
    int (*get_length) (cstring *this);

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
     * @brief get cstring buffer size
     */
    int (*get_size) (cstring *this);

    /**
     * @brief transfrom to int
     */
    int (*toint) (cstring *this);

    /**
     * @brief transfrom to lower char
     */
    char *(*tolower) (cstring *this);

    /**
     * @brief transfrom to lower char
     */
    char *(*toupper) (cstring *this);

    /**
     * @brief trim left blank char   
     */
    char *(*left_trim) (cstring *this);

    /**
     * @brief  trim middle blank char   
     */
    char *(*mid_trim) (cstring *this);

    /**
     * @brief trim right blank char   
     */
    char *(*right_trim) (cstring *this);

    /**
     * @brief trim all blank char   
     */
    char *(*all_trim) (cstring *this);

    /**
     * @brief cmp two string
     * @return an integer less than, equal to, or
     *         greater than zero if s1 is found, respectively, to be less
     *         than, to match, or be greater than s.
     */
    int (*cmp) (cstring *this, const char *s);

    /**
     * @brief cmp two string
     * @return an integer less than, equal to, or
     *         greater than zero if s1 is found, respectively, to be less
     *         than, to match, or be greater than s.
     */
    int (*casecmp) (cstring *this, const char *s);

    /**
     * @brief string wether is empty
     */
    int (*is_empty) (cstring *this);
};

/**
 * @brief Create cstring instance 
 */
cstring *create_cstring(unsigned int size);

#endif /* __CSTRING_H__ */
