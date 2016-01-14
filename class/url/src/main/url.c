/**
 * usual head files
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <url.h>

/*********************************************************
 **************    Function Declaration    ***************
 *********************************************************/

/*********************************************************
 ******************    Main Function    ******************
 *********************************************************/
int main(int agrc, char *agrv[])
{
    int rt = 0; /* return value of function main */
    url_t *url = url_create();

    /*
    url->set_url(url, "http://172.21.34.27:8000", "index.htm");
    url->get(url);
    */
    
    /*
    url->set_url(url, "http://172.21.34.27:8000/setup.cgi", "login.htm");
    url->post(url, "username=admin&password=admin&h_rem=1&todo=save&next_file=htmlcontrol.htm&this_file=index.htm");
    */

    /*
    url->set_url(url, "http://172.21.34.27:8000/upload.htm", "upload.htm");
    url->get(url);
    */

    /*
    url->set_url(url, "http://172.21.34.27:8000/upload.cgi", NULL);
    url->form_add(url, FORM_TEXT, "next_path", "upload.htm");
    url->form_add(url, FORM_FILE, "FILE1", "index.htm");
    url->form_add(url, FORM_TEXT, "todo", "save");
    url->form_add(url, FORM_TEXT, "next_file", "upload.htm");
    url->form_add(url, FORM_TEXT, "this_file", "upload.htm");
    url->form_post(url);
    */

    url->ssl_init(url, 
        "/home/anton/certs/ejbca/superadmin/ssl.ca.crt.pem",
        "/home/anton/certs/ejbca/superadmin/ssl.cli.crt.pem",
        "/home/anton/certs/ejbca/superadmin/ssl.cli.key.pem"
    );
    url->set_url(url, "https://172.21.34.86:8443/ejbca/adminweb/ra/editendentity.jsp?username=anton", "anton.htm");
    url->get(url);
    url->parse_form_data(url);
    url->list_data(url);
    
    char *value = NULL;
    url->set_value(url, "selecttoken", "654321");
    value = url->get_value(url, "selecttoken");
    printf("get_value: %s\n", value);

    url->destroy(url);
    return rt;
}
