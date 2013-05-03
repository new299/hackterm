#ifndef HTSSH_H
#define HTSSH_H

int ssh_open(char *hostname,char *username,char *password,char *fingerprintstrin,char *privkeypath,char *pubkeypath);
int ssh_open_preshell(char *hostname,char *username,char *password,char *fingerprintstrin,char *pubkeypath,char *privkeypath);
int ssh_close();
int ssh_write(char *bytes,int len);
int ssh_read(char *bytes,int len);
int ssh_resize(int cols,int rows);
char *ssh_fingerprintstr();

#endif
