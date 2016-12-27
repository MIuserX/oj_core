#ifndef _XUTOJ_MONITOR_INC
#define _XUTOJ_MONITOR_INC

typedef struct monitor_arg {
    pid_t   pidApp;
    int  &  ACflg;
    char *  outfile; // 检测程序输出是否超限
    char *  errfile; // 检测程序是否出错
    int     solutionid;
    int     lang;        // 使用的语言
    int  &  time_lmt;    // oj规则逻辑，时间限制
    int  &  mem_lmt;     // oj规则逻辑，内存限制
    int  &  usedtime;    
} MonitorArg;

//int xutoj_monitor(MonitorArg & marg);

int xutoj_monitor(pid_t pidApp,
                  int & ACflg,
                  char * outfile,
                  char * errfile,
                  int solution_id,
                  int lang,
                  int & topmemory, 
                  int time_lmt, 
                  int mem_lmt, 
                  int & usedtime);

#endif
