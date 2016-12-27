#include <iostream>

/* <stdio.h>
 *     freopen
 *
 **/
#include <stdio.h>

/* <unistd.h>
 *     execvp
 *     fork
 **/
#include <unistd.h>

//#include <sys/types.h>

/* <sys/wait.h>
 *     waitpid
 *
 **/
#include <sys/wait.h>

/* <sys/resources.h>
 *     setrlimit
 *
 **/
#include <sys/resource.h>

/* "timecost.h"
 *     class 
 *
 **/
#include "timecost.h"

#include "../inc/oj_errcodes.h"

using namespace std;
using namespace Jin_tools;

#define STD_MB (1024*1024)
#define DEBUG 1

int OJCompiler(int lang, const char * srcpath, const char * destpath)
{
    class Timecost tc;

    tc.Start();

    int retval = 0;
    int status = -1; // 子进程的退出状态码

    // 32 * 8 bytys = 256 bytes
    const char * compile_cmd[][32] = {
        { "gcc", "-std=c99", "-O2", "-Wall", "-o", "Main", "Main.c", "-lm", "-static", NULL },
        { "g++", "-O2", "-Wall", "-o", "Main", "Main.cpp", "-lm", "--static", "-DONLINE_JUDGE", NULL },
        { "fpc", "Main.pas", "-O2","-Co", "-Ct","-Ci", NULL },
        { "javac", "-J-Xms32m", "-J-Xmx256m", "Main.java",NULL },
        { "ruby", "-c", "Main.rb", NULL },
        { "chmod", "+rx", "Main.sh", NULL },
        { "python","-c","import py_compile; py_compile.compile(r'Main.py')", NULL },
        { "php", "-l","Main.php", NULL },
        { "perl","-c", "Main.pl", NULL },
        { "gmcs","-warn:0", "Main.cs", NULL },
        { "gcc","-o","Main","Main.m","-fconstant-string-class=NSConstantString","-I","/usr/include/GNUstep/","-L","/usr/lib/GNUstep/Libraries/","-lobjc","-lgnustep-base",NULL},
        { "fbc","-static","Main.bas",NULL}
    };
    
    // 0  -> c
    // 1  -> c++
    // 2  -> 
    // 3  -> java
    // 4  -> ruby
    // 5  -> bash shell
    // 6  -> python
    // 7  -> php
    // 8  -> perl
    // 9  -> 
    // 10 -> 
    // 11 -> 

    //const char * CP_C[]  = { "gcc", "-std=c99", "-O2", "-Wall", "-o", "Main", "Main.c", "-lm", "--static", NULL };
    //const char * CP_X[]  = { "g++", "-O2", "-Wall", "-o", "Main", "Main.cpp", "-lm", "--static", "-DONLINE_JUDGE", NULL };
    //const char * CP_P[]  = { "fpc", "Main.pas", "-O2","-Co", "-Ct","-Ci", NULL };
    //const char * CP_J[] = { "javac", "-J-Xms32m", "-J-Xmx256m", "Main.java",NULL };

    //const char * CP_R[]  = { "ruby", "-c", "Main.rb", NULL };
    //const char * CP_B[]  = { "chmod", "+rx", "Main.sh", NULL };
    //const char * CP_Y[]  = { "python","-c","import py_compile; py_compile.compile(r'Main.py')", NULL };
    //const char * CP_PH[] = { "php", "-l","Main.php", NULL };
    //const char * CP_PL[] = { "perl","-c", "Main.pl", NULL };
    //const char * CP_CS[] = { "gmcs","-warn:0", "Main.cs", NULL };
    //const char * CP_OC[] = { "gcc","-o","Main","Main.m","-fconstant-string-class=NSConstantString","-I","/usr/include/GNUstep/","-L","/usr/lib/GNUstep/Libraries/","-lobjc","-lgnustep-base",NULL};
    //const char * CP_BS[] = { "fbc","-static","Main.bas",NULL};
    
#if 0
    char javac_buf[4][16];
    char *CP_J[5];
    
    for (int i = 0; i < 4; i++) {
        CP_J[i] = javac_buf[i];
    }
    
    sprintf(CP_J[0], "javac");
    sprintf(CP_J[1], "-J%s", java_xms);
    sprintf(CP_J[2], "-J%s", java_xmx);
    sprintf(CP_J[3], "Main.java");
    CP_J[4] = (char *)NULL;
#endif

    //---- 1. 新建一个子进程
    int pid = fork();

    if (-1 == pid) {
        return OJERR_SYSCALL_FORK;
    }
    else if (pid == 0) { // child process
    // 子进程执行编译逻辑

        //---- 2. 限制子进程的资源
        struct rlimit LIM;
        LIM.rlim_max = 60;
        LIM.rlim_cur = 60;
        setrlimit(RLIMIT_CPU, &LIM);   // CPU clocks limit

        LIM.rlim_max = 90 * STD_MB;
        LIM.rlim_cur = 90 * STD_MB;
        setrlimit(RLIMIT_FSIZE, &LIM); // file size limit

        LIM.rlim_max = 1024 * STD_MB;
        LIM.rlim_cur = 1024 * STD_MB;
        setrlimit(RLIMIT_AS, &LIM);    // 
        
        //---- 3. 打开错误输出文件
        if (lang != 2) {
            freopen("ce.txt", "w", stderr);
        }
        else {
            freopen("ce.txt", "w", stdout);
        }

        //---- 4. 根据语言类型执行对应的编译语句
        if (lang >= 0 && lang <= 11) {
            retval = execvp(compile_cmd[lang][0], (char * const *)compile_cmd[lang]);
            return retval;
        }
        else {
            return OJERR_ARG_LANG_INVALID;
        }
    }
    else { // parent process 
    // 父进程等待子进程结束，

        waitpid(pid, &status, 0);
        
        if (lang > 3 && lang < 7) {
            //status = get_file_size("ce.txt");
        }
    }

    tc.Stop();

    tc.Disp();

    return 0;
}
