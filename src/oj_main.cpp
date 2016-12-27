#include <iostream>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/ptrace.h>

#include "../inc/oj_status.h"
#include "../inc/oj_compiler.h"
#include "../inc/oj_running.h"
#include "../inc/xutoj_monitor.h"
#include "../inc/oj_errcodes.h"


#ifndef JDEBUGGER
    #define JDEBUGGER (1)
#endif

#include "jdebugger.h"

using namespace std;

int main()
{
    cout << "Before OJCompiler()\n";

    OJCompiler(0);
 
    cout << "After OJCompiler()\n";

    int ACflg = OJ_AC;
    int PEflg = 0;
    int time_lmt = 10;
    int mem_lmt = 1024*10;
    int used_time = 1000;
    int lang = 0;
    char work_dir[] = "/root/Projects/c++/ojcore";
    char infile[] = "testdata.in";
    char userfile[] = "solution.out";
    char outfile[] = "answer.out";
    char errpath[] = "error.out";
    int topmemory = 1024;

    RunArg rarg;

    pid_t run_pid = fork();
    if (-1 == run_pid) {
        cout << "[ Error ] fork error" << endl;
        return 1;
    }
    else if (0 == run_pid) {
        errno = 0;
        int retval = ptrace(PTRACE_TRACEME, 0, NULL, NULL);
        if (-1 == retval && 0 != errno) {
            return OJERR_SYSCALL_PTRACE;
        }
        
        retval = oj_running(rarg, lang, work_dir, time_lmt, used_time, mem_lmt);
        cout << "[ Error ] oj_runnging error with retval=" << retval << endl;
        
        return 1;
    }
    else {
        //int retval = 0;
        int retval = xutoj_monitor(run_pid, ACflg, outfile, errpath, 2, lang, topmemory, 5, 1024, used_time);
        if (0 != retval) {
            return retval;
        }
        return 0;

        int status = -1;
        waitpid(run_pid, &status, 0);
        cout << "[ Info ] parent waitpid finished with status=" << status << endl;
    
        if (0 == WIFEXITED(status)) {
            int exitcode = WEXITSTATUS(status);
            cout << "exitcode=" << exitcode << endl;
        }

        if ( WIFSIGNALED(status) ) {
            int sig = WTERMSIG(status);
            cout << "term sig=" << sig << endl;
        }
    
        if ( WIFSTOPPED(status) ) {
            int sig = WSTOPSIG(status);
            cout << "stop sig=" << sig << endl;
        }
    
    }

    return 0;
}
