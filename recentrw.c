#include "recentrw.h"
#include <stdio.h>


char *fgetstr(char *dest,int len,FILE *f) {

  for(;;) {
    int c = getc(f);
    
    if(feof(f)) break;
    
    if(c== ' ') return dest;
    if(c=='\n') return dest;
    
    dest[0]=c;
    dest[1]=0;
    dest++;
  }
  return dest;
}

void readall_connections(char **hostnames,char **usernames,char **passwords) {

  FILE *recentf = fopen("recentservers.txt","rw");

  for(int n=0;n<RECENTCONNECTIONS;n++) {
    hostnames[n] = malloc(100);
    usernames[n] = malloc(100);
    passwords[n] = malloc(100);
  }
  
  for(int n=0;n<RECENTCONNECTIONS;n++) {
    hostnames[n][0]=0;
    usernames[n][0]=0;
    passwords[n][0]=0;
  }
  
  if(recentf==0) { return; }

  for(int n=0;n<RECENTCONNECTIONS;n++) {
    if(feof(recentf)) {
      hostnames[n][0]=0;
    } else {
//      char mystring [100];
      char *r = fgetstr(hostnames[n],99,recentf);
//      strcpy(hostnames[n],mystring);
    }
    if(feof(recentf)) usernames[n][0]=0; else fgetstr(usernames[n],99,recentf);
    if(feof(recentf)) passwords[n][0]=0; else fgetstr(passwords[n],99,recentf);
  }
  fclose(recentf);
}

void writeall_connections(char **hostnames,char **usernames,char **passwords) {

  FILE *recentf = fopen("recentservers.txt","w");
  for(int n=0;n<RECENTCONNECTIONS;n++) {
    fprintf(recentf,"%s %s %s\n",hostnames[n],usernames[n],passwords[n]);
  }
  fclose(recentf);
}

void write_connection(const char* hostname,const char *username,const char *password) {

  char *hostnames[RECENTCONNECTIONS];
  char *usernames[RECENTCONNECTIONS];
  char *passwords[RECENTCONNECTIONS];

  readall_connections(&hostnames,&usernames,&passwords);

  for(int n=(RECENTCONNECTIONS-1);n>=1;n--) {
    strcpy(hostnames[n],hostnames[n-1]);
    strcpy(usernames[n],usernames[n-1]);
    strcpy(passwords[n],passwords[n-1]);
  }
  
  strcpy(hostnames[0],hostname);
  strcpy(usernames[0],username);
  strcpy(passwords[0],password);

  writeall_connections(hostnames,usernames,passwords);
}

