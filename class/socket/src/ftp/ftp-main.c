#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <socket.h>
#include <utils/get_args.h>
#include <ftp.h>
#include <utils/utils.h>

#define DFT_FTP_SERVER_PORT (21)
int main(int argc, char **argv)
{
    int help_flag    = 0;
    int port         = 0;
    char *server     = NULL;
    char *user       = NULL;
    char *passwd     = NULL;
    char *list_path  = 0;
    char *path       = NULL;
    char *down_file  = NULL;
    char *up_path[2] = {NULL};
    ftp_t *ftp       = NULL;
    struct options opts[] = {
        {"-h", "--help"    , 0 , RET_INT , ADDR_ADDR(help_flag)} ,
        {"-s", "--server"  , 1 , RET_STR , ADDR_ADDR(server)}    ,
        {"-p", "--port"    , 1 , RET_INT , ADDR_ADDR(port)}      ,
        {"-u", "--user"    , 1 , RET_STR , ADDR_ADDR(user)}      ,
        {"-w", "--passwd"  , 1 , RET_STR , ADDR_ADDR(passwd)}    ,
        {"-l", "--list"    , 1 , RET_STR , ADDR_ADDR(list_path)} ,
        {"-z", "--size"    , 1 , RET_STR , ADDR_ADDR(path)} ,
        {"-d", "--download", 1 , RET_STR , ADDR_ADDR(down_file)} ,
        {"-o", "--upload"  , 2 , RET_STR , ADDR_ADDR(up_path)} ,
        {NULL, NULL        , 0 , 0       , NULL}                 ,
    };
    struct usage help_usage[] = {
        {"-h, --help"     , "program usage"}                                           ,
        {"-s, --server"   , "ftp server IP"}                                           ,
        {"-p, --port"     , "ftp server port listening on"}                            ,
        {"-u, --user"     , "user login"}                                              ,
        {"-w, --passwd"   , "password login"}                                          ,
        {"-l, --list"     , "list directory content in ftp server. Must append path."} ,
        {"-z, --size"     , "show file size on ftp server."},
        {"-d, --download" , "download one file"},
        {"-o, --upload"   , "upload one file"},
        {NULL             , NULL}
    };

    get_args(argc, argv, opts);
    if (help_flag) {
        print_usage(help_usage);
        exit(0);
    }

    if (!server) {
        printf("Please input ftp server IP address\n");
        return -1;
    }
    if (!user || !passwd) {
        printf("Please input ftp user or passwd!\n");
        return -1;
    }

    if (port < 1) port = DFT_FTP_SERVER_PORT;
    ftp = create_ftp();
    if (!ftp) return -1;

    if (ftp->login(ftp, server, port, user, passwd) != 0) goto logout;
    if (list_path) ftp->list(ftp, list_path);
    if (path) printf("%s size: %d\n", path, ftp->size(ftp, path));
    if (down_file) ftp->download(ftp, down_file);
    if (up_path[0] && up_path[1]) ftp->upload(ftp, up_path[0], up_path[1]);
    ftp->rmdir(ftp, ".", "test");

logout:
    ftp->destroy(ftp);
    return 0;
}
