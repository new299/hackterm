#include "local.h"

#ifdef LOCAL_ENABLE
#include <unistd.h>
#include <pty.h>
#include <fcntl.h>
#endif

int pid;
int flag;
int fd;

int local_open(char *a,char *b,char *c) {
#ifdef LOCAL_ENABLE
  pid = forkpty(&fd,0,0,0);
  flag = fcntl(fd,F_GETFL,0);

  char *termset = "TERM=xterm";
  putenv(termset);

  printf("fd: %d",fd);
  if(pid == 0) {
    char args[3];
    args[0] = "/bin/bash";
    args[1] =""; 
    args[2] = 0;

    execl("/bin/bash","bash",NULL);
    return 1;
  }

  printf("fd: %d\n",fd);
  
#endif
  return 1;
}

int local_write(char *buffer,int len) {
#ifdef LOCAL_ENABLE
  return write(fd,buffer,len);
#endif
}

int local_read(char *buffer,int len) {
#ifdef LOCAL_ENABLE
  return read(fd,buffer,len);
#endif
}

int local_resize(int rows,int cols) {
#ifdef LOCAL_ENABLE
  struct winsize size = { rows, cols, 0, 0 };
  ioctl(fd, TIOCSWINSZ, &size);
#endif
}

int local_close() {
#ifdef LOCAL_ENABLE
  close(fd);
#endif
}
