#ifndef _OJ_ERRCODES_INC
#define _OJ_ERRCODES_INC

//---- oj_compiler ----
#define OJERR_ARG_LANG_INVALID   (10001)
#define OJERR_SYSCALL_FORK       (10002)

//---- oj_running ----
#define OJERR_SYSCALL_NICE   (20001)
#define OJERR_SYSCALL_CHDIR  (20002)
#define OJERR_SYSCALL_PTRACE (20003)
#define OJERR_SYSCALL_CHROOT (20004)
#define OJERR_SYSCALL_EXECL  (20005)

void ErrDesc(long errcode, char * desc);

#endif
