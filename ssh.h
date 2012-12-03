#ifndef HTSSH_H
#define HTSSH_H

int ssh_open(char *hostname,char *password);
int ssh_close();
int ssh_sendbytes(char *bytes,int len);
int ssh_receivebytes(char *bytes,int len);

#endif
