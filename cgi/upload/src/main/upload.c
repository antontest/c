/************************************************************************** 
  2007-1-5 11:42 establish by lzh.A cgi program. 
  get a file from user's explorer. 
 ***************************************************************************/  
#include <stdio.h>  
#include <stdlib.h>  
#include <string.h>  
#include <cgi.h>

#define DEAL_BUF_LEN 1024  
#define SIGN_CODE_LEN 100  
#define FILE_NAME_LEN 64  
#define FILE_SAVE_DIR "/home/anton/web/"  

int main(void)  
{  
    cgi_t *cgi = cgi_create();
    cgi->get_form_data(cgi);
    cgi->read_action(cgi, NULL);
    cgi->destroy(cgi);
    return 0;
}  
