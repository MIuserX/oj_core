#ifndef _OJ_RUNNING_INC
#define _OJ_RUNNING_INC

typedef struct _run_arg {
    int  lang;
    char work_dir;
    int  time_lmt;
    int  mem_lmt;
    int  used_time;
} RunArg;

int oj_running(RunArg & arg, int & lang, char * work_dir, int & time_lmt, int & usedtime, int & mem_lmt);

#endif
