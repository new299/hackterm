#ifndef HTLOCAL_H
#define HTLOCAL_H

int local_open(char *a,char *b);
int local_close();
int local_write(char *bytes,int len);
int local_read(char *bytes,int len);
int local_resize(int rows,int cols);

#endif
