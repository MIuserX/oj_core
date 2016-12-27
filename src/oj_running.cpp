#include <iostream>

/* <stdio.h>
 *     freopen - 一般用来重定向文件描述符
 *
 **/
#include <stdio.h>
#include <errno.h>

/* <unistd.h>
 *     nice   - 改变程序优先级
 *     chdir  - 改变工作目录
 *     chroot - 改变 root 目录
 *     sleep  - 
 *     execl  - 
 *     
 **/
#include <sys/types.h>
#include <unistd.h>

/* <sys/ptrace.h>
 *     ptrace - 
 *
 **/
#include <sys/ptrace.h>

/* <sys/resource.h>
 *     setrlimit - 
 *
 **/
#include <sys/time.h>
#include <sys/resource.h>

#include "../inc/oj_running.h"
#include "../inc/oj_errcodes.h"

using namespace std;

static int oi_mode = 1;

#define STD_F_LIM (1024)
#define STD_MB (1024*1024)

int oj_running(RunArg & arg, int & lang, char * work_dir, int & time_lmt, int & usedtime, int & mem_lmt)
{
    int retval = -1;
    FILE *retptr = NULL;

    //---- 1. 给程序的优先级加上 19 [-20 ~ 19]
    errno = 0;
    retval = nice(19);
    if (-1 == retval && 0 != errno) {
        // output something
        return OJERR_SYSCALL_NICE;
    }

    printf("Add nice 19\n");

    //---- 2. change work directory
    // now the user is "judger"
    if (0 != chdir(work_dir)) {
        return OJERR_SYSCALL_CHDIR;
    }

    printf("chdir to %s\n", work_dir);

    //---- 3. redirect stdin, stdout, stderr
    retptr = freopen("data.in", "r", stdin);
    if (NULL == retptr) {
        printf("freopen data.in failed!\n");
        return 88;
    }
    retptr = freopen("user.out", "w", stdout);
    if (NULL == retptr) {
        cout << "freopen user.out failed!" << endl;
        return 88;
    }
    retptr = freopen("error.out", "a+", stderr);
    if (NULL == retptr) {
        return 88;
    }

    printf("stdin, stdout, stderr redirected\n");

    //---- 5. chroot
    if (lang != 3) {
        retval = chroot(work_dir);
        if (-1 == retval) {
            return OJERR_SYSCALL_CHROOT;
        }
    }

    printf("chroot to %s\n", work_dir);

    //---- 6. 
    while(setgid(1700) != 0) {
        sleep(1);
    }
    
    while(setuid(1700) != 0) {
        sleep(1);
    }
    
    while(setresuid(1700, 1700, 1700) != 0) {
        sleep(1);
    }
#if 0
    //---- 7. 设置自己的 CPU时间，单位为秒
    // char java_p1[BUFFER_SIZE], java_p2[BUFFER_SIZE];
    // child
    // set the limit
    struct rlimit LIM; // time limit, file limit& memory limit
    // time limit
    if (oi_mode) {
        LIM.rlim_cur = time_lmt+1;
    }
    else {
        LIM.rlim_cur = (time_lmt - usedtime / 1000) + 1;
    }
    LIM.rlim_max = LIM.rlim_cur;

    setrlimit(RLIMIT_CPU, &LIM);
    
    //---- 8. 设置自己的运行时间上限
    alarm(0);
    alarm(time_lmt*10);

    //---- 9. 设置进程建立文件大小限制
    // file limit
    LIM.rlim_max = STD_F_LIM + STD_MB;
    LIM.rlim_cur = STD_F_LIM;

    setrlimit(RLIMIT_FSIZE, &LIM);
    
    //---- 10. 用户最大可拥有的进程数
    // proc limit
    switch (lang) {
        case 3:  //java
            LIM.rlim_cur = LIM.rlim_max=50;
            break;

        case 5: //bash
            LIM.rlim_cur = LIM.rlim_max=3;
            break;

        case 9: //C#
            LIM.rlim_cur = LIM.rlim_max=3;
            break;

        default:
            LIM.rlim_cur = LIM.rlim_max=1;
    }
    setrlimit(RLIMIT_NPROC, &LIM);

    //---- 12. 设置进程堆栈限制，以字节为单位
    LIM.rlim_cur = STD_MB << 6;
    LIM.rlim_max = STD_MB << 6;

    setrlimit(RLIMIT_STACK, &LIM);
    
    //---- 13. 设置进程最大虚拟空间限制，以字节为单位
    LIM.rlim_cur = STD_MB * mem_lmt/2*3;
    LIM.rlim_max = STD_MB * mem_lmt*2;
    if (lang<3) {
        setrlimit(RLIMIT_AS, &LIM);
    }
#endif

    printf("Before execl\n");

    //---- 14. 运行 solution 程序
    switch (lang) {
        case  0: // c
        case  1: // c++
        case  2: //
        case 10: //
        case 11: // 
            retval = execl("./Main", "./Main", (char *)NULL);
            break;
        #if 0
        case 3: // java
            execl("/usr/bin/java", "/usr/bin/java", java_xms,java_xmx,
                  "-Djava.security.manager",
                  "-Djava.security.policy=./java.policy", "Main", (char *)NULL);
            break;
        #endif
        case 4: // ruby
            execl("/ruby", "/ruby", "Main.rb", (char *)NULL);
            break;
        
        case 5: // bash
            execl("/bin/bash", "/bin/bash", "Main.sh", (char *)NULL);
            break;
        
        case 6: // Python
            execl("/python", "/python", "Main.py", (char *)NULL);
            break;
        
        case 7: // php
            execl("/php", "/php", "Main.php", (char *)NULL);
            break;
        
        case 8: // perl
            execl("/perl", "/perl", "Main.pl", (char *)NULL);
            break;
        
        case 9: // Mono C#
            execl("/mono", "/mono", "--debug", "Main.exe", (char *)NULL);
            break;

        default:
            
            break;
    }
    
    return OJERR_SYSCALL_EXECL;
}
