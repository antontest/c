#ifndef __XLS_H__
#define __XLS_H__

int xls_file(char *outbuf, char *errbuf, cgi_form_entry_t *entry);
int xls_data(char *outbuf, char *errbuf, cgi_form_entry_t *entry);
int xls_change_data(char *outbuf, char *errbuf, cgi_form_entry_t *entry);

#endif /* __XLS_H__ */
