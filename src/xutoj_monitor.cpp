#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#include <stdarg.h>

#include <dirent.h>

#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>

/* <sys/user.h>
 *     struct user_regs_struct
 *
 **/
#include <sys/user.h>

/* <sys/syscall.h>
 * 
 *
 **/
#include <sys/syscall.h>

/* <sys/ptrace.h>
 *     ptrace - 用于跟踪程序
 *
 **/
#include <sys/ptrace.h>

/* <signal.h>
 *     kill - 向进程发送信号
 *
 **/
#include <signal.h>

/* <sys/wait.h>
 *     wait4       - 
 *     WIFEXITED   - 
 *     WTERMSIG    - 
 *     WEXITSTATUS - 
 *
 **/
#include <sys/wait.h>

/* <unistd.h>
 *     alarm - 
 *
 **/
#include <unistd.h>

#include "../inc/oj_errcodes.h"
#include "../inc/oj_tools.h"
#include "../inc/oj_status.h"
#include "jdebugger.h"

using namespace std;

#ifdef __i386
    #define REG_SYSCALL orig_eax
    #define REG_RET eax
    #define REG_ARG0 ebx
    #define REG_ARG1 ecx
#else
    #define REG_SYSCALL orig_rax
    #define REG_RET rax
    #define REG_ARG0 rdi
    #define REG_ARG1 rsi
#endif

typedef struct procinfo {
    int sig;                // 用于记录 ptrace 跟踪的子进程的停止信号
    int status;             // 用于记录 ptrace 跟踪的子进程的状态 
    int exitcode;           // 用于记录 ptrace 跟踪的子进程的返回码
    struct rusage ruse;     // 用于记录 ptrace 跟踪的子进程的资源使用情况
    struct user_regs_struct reg;
} ProcInfo;

#define DEBUG (1)
#define STD_MB 1048576

#define JDEBUGGER (1)

/* xutoj_monitor - 
 *
 * Description:
 *
 * Params:
 *     pidApp    [in]  - 待监控进程（子进程）的 pid
 *     ACflg     [out] - solution 程序对应于 oj 的通过状态
 *
 * Return:
 *
 *
 **/

/* inpath  - 测试用例文件路径
 * outpath - 测试用例答案路径
 * errpath - 错误文件路径
 * 
 **/

/* 1. 对于内存的监控
 *     每次进程的 status 改变时，都检查一个 VmPeak 值，
 * 是否超过 mem_limit。
 *     所以需要传入 mem_limit 参数。
 **/

/* 2. 对于时间的监控
 *     每次进程的 status 改变时，都将消耗的时间加到 usedtime 上，
 * 是否超过 time_lmt。
 *     所以需要传入 time_lmt 参数。
 **/

/* 3. 
 *
 *
 *
 **/

int xutoj_monitor(MonitorInArg marg,
                  pid_t pidApp,
                  int & ACflg,
                  char * userfile,
                  char * errfile,
                  int solution_id,
                  int lang,
                  int & topmemory,
                  int time_lmt,
                  int mem_lmt,
                  int & usedtime)
{
    printf("Enter into xutoj_monitor()\n");

    // parent
    int retval = 0;
    int tempmemory;
    int sub_level = 0;
    int sig; 
    int status;             // 用于记录 ptrace 跟踪的子进程的状态 
    int exitcode;           // 用于记录 ptrace 跟踪的子进程的返回码
    struct user_regs_struct reg;
    struct rusage ruse;
    int sub = 0;
    pid_t subwatcher = 0;
    
    while (1) {
        
        printf("Before wait4\n");

//======================
// 1. 检测内存是否超限 
//======================
        //--> 1. 获取子进程进程状态改变时的资源使用情况
        retval = wait4(pidApp, &status, 0, &ruse);
        if (0 != retval) {
            return OJERR_UNKNOWN;
        }

        printf("After wait4\n");

        //--> 2. 获取进程虚拟内存使用峰值(tempmemory 单位为 byte)
        tempmemory = get_proc_status(pidApp, "VmPeak:") << 10;

        //--> 3. 调整 tempmemory
        if (tempmemory > topmemory) {
            topmemory = tempmemory;
        }

        //--> 4. 如果内存使用大于内存限制，则
        if (topmemory > mem_lmt * STD_MB) {
            
            if (ACflg == OJ_AC) {
                ACflg = OJ_ML;
            }
        
            printf("out of memory, will kill\n");

            kill(pidApp, SIGKILL);

            break;
        }

        //--> 5. 如果 status 代表 solution 程序已退出，则退出该循环
        if ( WIFEXITED(status) ) {
            
            printf("WIFEXITED with status = %d\n", status);
            
            break;
        }

//==========================
// 2. 检测出错文件是否超限
//==========================
        //--> 6. 检测 error.out 文件的大小，其实就是检测程序有没有出错
        if (get_file_size(errfile) > 0) {
            
            ACflg = OJ_RE;
            
            printf("error size > 0 \n");

            kill(pidApp, SIGKILL);
            
            break;
        }

//======================
// 3. 检测输出是否超限
//======================

        //--> 7. 检测 user.out 文件的大小，其实就是检测输出数据是不是超过限制
        //if (!isspj && get_file_size(userfile) > get_file_size(outfile) * 2+1024) {
        if (get_file_size(userfile) > 1024 * 2) {
            
            ACflg = OJ_OL;
            
            printf("userfile too large\n");

            kill(pidApp, SIGKILL);
            
            break;
        }

        exitcode = WEXITSTATUS(status);
        /* exitcode == 5 waiting for next CPU allocation
         * ruby using system to run,exit 17 ok
         **/
        if (exitcode == 0x05 || exitcode == 0)
            //go on and on
            ;
        else {

            if (ACflg == OJ_AC) {
                switch (exitcode) {
                    case SIGCHLD:
                    case SIGALRM:
                        alarm(0);
                    
                    case SIGKILL:
                    case SIGXCPU:
                        ACflg = OJ_TL;
                        break;
                    
                    case SIGXFSZ:
                        ACflg = OJ_OL;
                        break;
                    
                    default:
                        ACflg = OJ_RE;
                }
                printf("sig=%d\n", exitcode);
                //print_runtimeerror(strsignal(exitcode));
            }

            kill(pidApp, SIGKILL);

            break;
        }

//========================
// 4. 检测程序收到的信号
//========================

        // 检测 ptrace 监控的进程是否因为收到信号而结束
        if (WIFSIGNALED(status)) {
            /*  WIFSIGNALED: if the process is terminated by signal
             *
             *  psignal(int sig, char *s)，like perror(char *s)，print out s, with error msg from system of sig
             * sig = 5 means Trace/breakpoint trap
             * sig = 11 means Segmentation fault
             * sig = 25 means File size limit exceeded
             */

            /* WTERMSIG(status) 宏用于获取导致进程终止的信号值
             */

            //--> 1. 获取导致进程终止的信号值
            sig = WTERMSIG(status);

            //--> 2. 处理该信号
            if (ACflg == OJ_AC) {
                switch (sig) {
                    case SIGCHLD:
                    case SIGALRM:
                        alarm(0);
                    
                    case SIGKILL:
                    case SIGXCPU:
                        ACflg = OJ_TL;
                        break;
                    
                    case SIGXFSZ:
                        ACflg = OJ_OL;
                        break;

                    default:
                        ACflg = OJ_RE;
                }

                //print_runtimeerror(strsignal(sig));
            }

            break;
        }

//========================
// 5. 处理程序的系统调用
//========================

#if 1
        // check the system calls
        ptrace(PTRACE_GETREGS, pidApp, NULL, &reg);
#if 0
        if (reg.REG_SYSCALL > 0 && call_counter[reg.REG_SYSCALL] <= 0) {
        // 如果系统调用。。。
            
            ACflg = OJ_RE;

            char error[BUFFER_SIZE];
            sprintf(error,"[ERROR] A Not allowed system call: runid:%d callid:%lld\n", solution_id, reg.REG_SYSCALL);
            //write_log(error);
            //print_runtimeerror(error);
            
            kill(pidApp, SIGKILL);    
        }
        else 
#endif
        {
            //if (sub == 1 && call_counter[reg.REG_SYSCALL] > 0) {
            //    call_counter[reg.REG_SYSCALL]--;
            //}

            // 处理创建子进程的情况
            if (reg.REG_SYSCALL == SYS_fork || reg.REG_SYSCALL == SYS_clone || reg.REG_SYSCALL == SYS_vfork) {
                
                if (sub_level > 3 && sub == 1) {
                    printf("sub are not allowed to fork!\n");
                    kill(pidApp, SIGKILL);
                }
                else {
                    
                    ptrace(PTRACE_SINGLESTEP, pidApp, NULL, NULL);

                    ptrace(PTRACE_GETREGS, pidApp, NULL, &reg);
                    
                    pid_t subpid = reg.REG_RET;
                    
                    if (subpid > 0 && subpid != subwatcher) {

                        subwatcher = fork();
                        if (subwatcher == 0) {
                            //total_sub++;
                            sub_level++;
                            pidApp = subpid;
                            int success = ptrace(PTRACE_ATTACH, pidApp, NULL, NULL);
                            if(success==0) {
                                wait(NULL);
                                printf("attatched sub %d->%d\n", getpid(), pidApp);

                                // ptrace(PTRACE_SYSCALL, traced_process,NULL, NULL);
                            }
                            else {
                                //printf("not attatched sub %d\n",traced_process);

                                exit (0);
                            }
                        }
                    }
                }
                
                reg.REG_SYSCALL = 0;

            }// 处理子进程情况结束
        }
        
        sub = 1 - sub;

        ptrace(PTRACE_SYSCALL, pidApp, NULL, NULL);
#endif

        //---- 用于计算进程使用的 CPU 时间
        usedtime += (ruse.ru_utime.tv_sec * 1000 + ruse.ru_utime.tv_usec / 1000);
        usedtime += (ruse.ru_stime.tv_sec * 1000 + ruse.ru_stime.tv_usec / 1000);
        
        //---- 
        if(sub_level) {
            exit(0);
        }
        
        //clean_session(pidApp);
    }
}

#if 0
// disabled on 2016.10.18

int oj_monitor()
{
    char path[] = "/root/Projects/c++/ojcore/src";

    DIR    *pdir = NULL;
    dirent *pdirent = NULL;
    
    pdir = opendir(path);
    if (NULL == pdir) {
        return -1;
    }

    while (1) {
        pdirent = readdir(pdir);
        
        if (NULL == pdirent) {
            break;
        }

        cout << pdirent->d_name << endl;
    }

    return 0;
}

int main()
{
    oj_monitor();
    
    return 0;
}
#endif
