#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>

#include "../inc/oj_errcodes.h"

using namespace std;

#define DEBUG (1)

void watch_solution(pid_t pidApp,
                    char * infile,
                    int & ACflg,
                    int isspj,
                    char * userfile,
                    char * outfile,
                    int solution_id,
                    int lang,
                    int & topmemory, 
                    int mem_lmt, 
                    int & usedtime, 
                    int time_lmt, 
                    int & p_id,
                    int & PEflg, 
                    char * work_dir)
{
    // parent
    int tempmemory;
    int sub_level = 0;
    
    if (DEBUG) {
        printf("pid=%d judging %s\n", pidApp, infile);
    }
    
    int status;             // 用于记录 ptrace 跟踪的子进程的状态 
    int sig; 
    int exitcode;
    struct user_regs_struct reg;
    struct rusage ruse;
    int sub = 0;
    pid_t subwatcher = 0;
    
    while (1) {
        //--> 1. 获取子进程进程状态改变时的资源使用情况
        wait4(-1, &status, 0, &ruse);

        //--> 2. 
        //jvm gc ask VM before need,so used kernel page fault times and page size
        if (lang == 3) {
            //tempmemory = get_page_fault_mem(ruse, pidApp);
        }
        else {  //other language use VmPeak
            tempmemory = get_proc_status(pidApp, "VmPeak:") << 10;
        }

        //--> 3. 
        if (tempmemory > topmemory) {
            topmemory = tempmemory;
        }

        //--> 4. 
        if (topmemory > mem_lmt * STD_MB) {
            if (DEBUG) {
                printf("out of memory %d\n", topmemory);
            }
            
            if (ACflg == OJ_AC) {
                ACflg = OJ_ML;
            }
            
            ptrace(PTRACE_KILL, pidApp, NULL, NULL);
            break;
        }

        //--> 5. 如果 status 代表 solution 程序已退出，则退出该循环
        if ( WIFEXITED(status) ) {
            break;
        }

        //--> 6. 检测 error.out 文件的大小，其实就是检测程序有没有出错
        // lang < 4 表示语言为 {0->c; 1->c++; 2->; 3->;}
        // lang = 9 
        // 
        // !oi_mode 
        //if ((lang < 4 || lang == 9) && get_file_size("error.out") && !oi_mode) {
        if ((lang < 4 || lang == 9) && get_file_size("error.out")) {
            ACflg = OJ_RE;
            //addreinfo(solution_id);
            ptrace(PTRACE_KILL, pidApp, NULL, NULL);
            break;
        }

        //--> 7. 检测 user.out 文件的大小，其实就是检测输出数据是不是超过限制
        // !isspj 
        // get_file_size(userfile)
        // get_file_size(outfile)
        //if (!isspj && get_file_size(userfile) > get_file_size(outfile) * 2+1024) {
        if (get_file_size(userfile) > get_file_size(outfile) * 2 + 1024)
            ACflg = OJ_OL;
            ptrace(PTRACE_KILL, pidApp, NULL, NULL);
            break;
        }

        exitcode = WEXITSTATUS(status);
        /* exitcode == 5 waiting for next CPU allocation
         * ruby using system to run,exit 17 ok
         **/
        if ((lang >= 3 && exitcode == 17) || exitcode == 0x05 || exitcode == 0)
            //go on and on
            ;
        else {

            if (DEBUG) {
                printf("status>>8=%d\n", exitcode);
            }
            //psignal(exitcode, NULL);

            if (ACflg == OJ_AC)
            {
                switch (exitcode)
                {
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
                print_runtimeerror(strsignal(exitcode));
            }
            ptrace(PTRACE_KILL, pidApp, NULL, NULL);

            break;
        }
        if (WIFSIGNALED(status)) {
            /*  WIFSIGNALED: if the process is terminated by signal
             *
             *  psignal(int sig, char *s)，like perror(char *s)，print out s, with error msg from system of sig
                   * sig = 5 means Trace/breakpoint trap
                   * sig = 11 means Segmentation fault
                   * sig = 25 means File size limit exceeded
                   */
            sig = WTERMSIG(status);

            if (DEBUG) {
                printf("WTERMSIG=%d\n", sig);
                psignal(sig, NULL);
            }
            if (ACflg == OJ_AC)
            {
                switch (sig)
                {
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
                print_runtimeerror(strsignal(sig));
            }
            break;
        }
        /*     comment from http://www.felix021.com/blog/read.php?1662

          WIFSTOPPED: return true if the process is paused or stopped while ptrace is watching on it
          WSTOPSIG: get the signal if it was stopped by signal
         */

        // check the system calls
        ptrace(PTRACE_GETREGS, pidApp, NULL, &reg);

        if (false && reg.REG_SYSCALL > 0 && call_counter[reg.REG_SYSCALL] == 0)   //do not limit JVM syscall for using different JVM
        {
            ACflg = OJ_RE;

            char error[BUFFER_SIZE];
            sprintf(error,"[ERROR] A Not allowed system call: runid:%d callid:%lld\n",
                    solution_id, reg.REG_SYSCALL);
            write_log(error);
            print_runtimeerror(error);
            ptrace(PTRACE_KILL, pidApp, NULL, NULL);
         //   wait4(pidApp,NULL,0,NULL);
        }
        else {
            if (sub == 1 && call_counter[reg.REG_SYSCALL] > 0)
                call_counter[reg.REG_SYSCALL]--;


            if(reg.REG_SYSCALL==SYS_fork||reg.REG_SYSCALL==SYS_clone||reg.REG_SYSCALL==SYS_vfork)//
            {
                if(sub_level>3&&sub==1) {
                    printf("sub are not allowed to fork!\n");
                    ptrace(PTRACE_KILL, pidApp, NULL, NULL);

                }
                else {
                    //printf("syscall:%ld\t",regs.REG_SYSCALL);
                    ptrace(PTRACE_SINGLESTEP, pidApp, NULL, NULL);

                    ptrace(PTRACE_GETREGS, pidApp,
                           NULL, &reg);
                    //printf("pid=%lu\n",regs.eax);
                    pid_t subpid=reg.REG_RET;
                    if(subpid > 0 && subpid != subwatcher) {
                        //ptrace(PTRACE_ATTACH, subpid,               NULL, NULL);
                        //wait(NULL);

                        subwatcher=fork();
                        if(subwatcher==0) {
                            //total_sub++;
                            sub_level++;
                            pidApp=subpid;
                            int success = ptrace(PTRACE_ATTACH, pidApp, NULL, NULL);
                            if(success==0) {
                                wait(NULL);
                                printf("attatched sub %d->%d\n",getpid(),pidApp);

                                // ptrace(PTRACE_SYSCALL, traced_process,NULL, NULL);
                            }
                            else {
                                //printf("not attatched sub %d\n",traced_process);

                                exit (0);
                            }
                        }



                    }
                }
                reg.REG_SYSCALL=0;

            }
        }
        sub = 1 - sub;


        ptrace(PTRACE_SYSCALL, pidApp, NULL, NULL);
    }

    usedtime += (ruse.ru_utime.tv_sec * 1000 + ruse.ru_utime.tv_usec / 1000);
    usedtime += (ruse.ru_stime.tv_sec * 1000 + ruse.ru_stime.tv_usec / 1000);
    if(sub_level) exit(0);
    //clean_session(pidApp);
}

#if 0
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
