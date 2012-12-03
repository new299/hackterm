#ifndef HTSSH_H
#define HTSSH_H

int ssh_open(char *hostname,char *username,char *password);
int ssh_close();
int ssh_write(char *bytes,int len);
int ssh_read(char *bytes,int len);
int ssh_resize(int cols,int rows);

#endif
