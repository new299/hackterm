#include <stdio.h>
#include <time.h>


int main() {
 
  FILE *f = fopen("tttt","r");
 

  setbuf(stdout, NULL); 
  setbuf(stdin, NULL); 
  int n=0;

  struct timespec tt;

  tt.tv_sec  = 0;
  tt.tv_nsec = 5000000;

  for(;!feof(f);) {
 //   int i = getc(stdin);
 //   if(i == 'a') break;
    char c = getc(f);
    nanosleep(&tt,NULL);

    putchar(c);
    fflush(stdout);
    n++;
  }
  printf("output: %d\n",n);

}
