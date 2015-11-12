#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <socket.h>
#include <utils/get_args.h>
#include <ftp.h>
#include <utils/utils.h>

int main(int argc, char **argv)
{
    int help_flag   = 0;
    int port        = 0;
    char *server    = NULL;
    char *user      = NULL;
    char *passwd    = NULL;
    char *list_path = 0;
    ftp_t *ftp      = NULL;
    struct options opts[] = {
        {"-h", "--help",   0, RET_INT, ADDR_ADDR(help_flag)},
        {"-s", "--server", 1, RET_STR, ADDR_ADDR(server)},
        {"-p", "--port",   1, RET_INT, ADDR_ADDR(port)},
        {"-u", "--user",   1, RET_STR, ADDR_ADDR(user)},
        {"-w", "--passwd", 1, RET_STR, ADDR_ADDR(passwd)},
        {"-l", "--list",   1, RET_STR, ADDR_ADDR(list_path)},
        {NULL, NULL,       0, 0,       NULL},
    };
    struct usage help_usage[] = {
        {"-h, --help",   "program usage"},
        {"-s, --server", "ftp server IP"},
        {"-p, --port",   "ftp server port listening on"},
        {"-u, --user",   "user login"},
        {"-w, --passwd", "password login"},
        {"-l, --list",   "list current directory content in ftp server. Must append path"},
        {NULL, NULL}
    };

    get_args(argc, argv, opts);
    if (help_flag) {
        print_usage(help_usage);
        exit(0);
    }

    if (!server || !user || !passwd || port < 1) return -1;
    ftp = create_ftp();
    if (!ftp) return -1;

    if (!list_path) return -1;
    if (ftp->login(ftp, server, port, user, passwd) != 0) goto logout;
    ftp->list(ftp, list_path);

logout:
    ftp->destroy(ftp);
    return 0;
}
