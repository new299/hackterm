#include "recentrw.h"

void readall_connections(char **hostnames,char **usernames,char **passwords) {

  FILE *recentf = ifopen("recentservers.txt","r");

  for(int n=0;n<RECENTCONNECTIONS;n++) {
    hostnames[n] = malloc(100);
    usernames[n] = malloc(100);
    passwords[n] = malloc(100);
  }

  for(int n=0;n<RECENTCONNECTIONS;n++) {
    if(feof(recentf)) hostnames[n][0]=0; else fgets(hostnames[n],99,recentf);
    if(feof(recentf)) usernames[n][0]=0; else fgets(usernames[n],99,recentf);
    if(feof(recentf)) passwords[n][0]=0; else fgets(passwords[n],99,recentf);
  }
}

void writeall_connections(char **hostnames,char **usernames,char **passwords) {

  FILE *recentf = ifopen("recentservers.txt","w");
  for(int n=0;n<RECENTCONNECTIONS;n++) {
    fprintf(recentf,"%s %s %s\n",hostnames[n],usernames[n],passwords[n]);
  }
}

void write_connection(const char* hostname,const char *username,const char *password) {

  char *hostnames[RECENTCONNECTIONS];
  char *usernames[RECENTCONNECTIONS];
  char *passwords[RECENTCONNECTIONS];

  readall_connections(&hostnames,&usernames,&passwords);

  for(int n=1;n<RECENTCONNECTIONS;n++) {
    strcpy(hostnames[n-1],hostnames[n]);
    strcpy(usernames[n-1],usernames[n]);
    strcpy(passwords[n-1],passwords[n]);
  }
  
  strcpy(hostnames[0],hostname);
  strcpy(usernames[0],username);
  strcpy(passwords[0],password);

  writeall_connections(hostnames,usernames,passwords);
}

