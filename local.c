#include "local.h"

#ifdef LOCAL_ENABLE

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#ifdef LINUX_BUILD
#include <pty.h>
#endif

#ifdef OSX_BUILD
#include <util.h>
#endif 

#endif

int pid;
int flags;
int fd;

int local_open(char *a,char *b,char *c) {
#ifdef LOCAL_ENABLE
  pid = forkpty(&fd,0,0,0);

  if(pid == -1) {
    printf("forkpty failed\n");
  }

  flags = fcntl(fd,F_GETFL,0);
  fcntl(fd, F_SETFL, flags | O_NONBLOCK);

  char *termset = "TERM=xterm";
  putenv(termset);

  printf("fd: %d",fd);
  if(pid == 0) {
    char args[3];
    args[0] = "/bin/bash";
    args[1] =""; 
    args[2] = 0;

    execl("/bin/bash","bash",NULL);
    printf("forked\n");
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
  int res = read(fd,buffer,len);

  if((res == -1) && ((errno == EAGAIN) || (errno == EWOULDBLOCK))) {
    return 0;
  }
  return res;
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
