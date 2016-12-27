#ifndef _OJ_TOOLS_INC
#define _OJ_TOOLS_INC

void ErrDesc(long errcode, char * desc);
int get_proc_status(int pid, const char * mark);
long get_file_size(const char * filename);

#endif
