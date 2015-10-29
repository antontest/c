#include <stdarg.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include "debug.h"

ENUM(debug_names, DBG_DMN, DBG_LIB,
        "DMN",
        "MGR",
        "IKE",
        "CHD",
        "JOB",
        "CFG",
        "KNL",
        "NET",
        "ASN",
        "ENC",
        "TNC",
        "IMC",
        "IMV",
        "PTS",
        "TLS",
        "APP",
        "ESP",
        "LIB",
    );

ENUM(debug_lower_names, DBG_DMN, DBG_LIB,
        "dmn",
        "mgr",
        "ike",
        "chd",
        "job",
        "cfg",
        "knl",
        "net",
        "asn",
        "enc",
        "tnc",
        "imc",
        "imv",
        "pts",
        "tls",
        "app",
        "esp",
        "lib",
    );

/**
 * level logged by the default logger
 */
static level_t default_level = 1;

/**
 * stream logged to by the default logger
 */
static FILE *default_stream = NULL;

static char debug_char[1024] = {0};

static char *get_time_string()
{
    struct tm *ptr = NULL;
    time_t lt;

    lt = time(NULL);
    ptr = localtime(&lt);
    strftime(debug_char, sizeof(debug_char), "%b %d %Y %T ", ptr);

    return debug_char;
}

/**
 * default dbg function which printf all to stderr
 */
void dbg_default(debug_t group, level_t level, char *fmt, ...)
{
    if (!default_stream)
    {
        default_stream = stderr;
    }

    if (level <= default_level)
    {
        va_list args;
        char *proc_name = NULL;

        fprintf(default_stream, "%s", get_time_string());
        if (readlink("/proc/self/exe", debug_char, sizeof(debug_char))){}
        proc_name = strrchr(debug_char, '/');
        if (proc_name != NULL) {
            fprintf(default_stream, "%s ", proc_name + 1);
        }

        if (group >= 0) 
            fprintf(default_stream, "[%s", enum_to_name(debug_names, group));
        if (level) 
            fprintf(default_stream, " %s", enum_to_name(debug_lower_names, level));
        fprintf(default_stream, "] ");

        va_start(args, fmt);
        vfprintf(default_stream, fmt, args);
        fprintf(default_stream, "\n");
        va_end(args);
    }
}

/**
 * set the level logged by the default stderr logger
 */
void dbg_default_set_level(level_t level)
{
    default_level = level;
}

/**
 * set the stream logged by dbg_default() to
 */
void dbg_default_set_stream(FILE *stream)
{
    default_stream = stream;
}

/**
 * The registered debug hook.
 */
void (*dbg) (debug_t group, level_t level, char *fmt, ...) = dbg_default;
