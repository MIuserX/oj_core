#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../inc/oj_errcodes.h"

void ErrDesc(long errcode, char * desc)
{
    switch (errcode) {
        case OJERR_SYSCALL_NICE:
            strcpy(desc, "syscall nice() error!");
            break;

        default:
            strcpy(desc, "Unknown Error!");
    }
}

#define BUFFER_SIZE (128)

int get_proc_status(int pid, const char * mark)
{
    FILE * fp = NULL;
    char fpath[BUFFER_SIZE];
    char linebuf[BUFFER_SIZE];
    int retval = 0;
    
    sprintf(fpath, "/proc/%d/status", pid);
    
    fp = fopen(fpath, "r");
    if (NULL == fp) {
        return -1;
    }
    
    int m = strlen(mark);
    while (fgets(linebuf, BUFFER_SIZE - 1, fp)) {

        linebuf[strlen(linebuf) - 1] = 0;
        if (strncmp(linebuf, mark, m) == 0) {
            sscanf(linebuf + m + 1, "%d", &retval);
        }
    }
    
    fclose(fp);

    return retval;
}

long get_file_size(const char * filename)
{
    struct stat f_stat;

    if (stat(filename, &f_stat) == -1) {
        return -1;
    }

    return (long)f_stat.st_size;
}
