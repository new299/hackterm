#include "recentrw.h"
#include <stdio.h>
#import <Foundation/Foundation.h>

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

  NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
  NSString *fpath = [paths objectAtIndex:0];

  fpath = [fpath stringByAppendingString:@"/recentservers.txt"];

  printf("read file path: %s\n",[fpath cStringUsingEncoding:NSUTF8StringEncoding]);

  FILE *recentf = fopen([fpath cStringUsingEncoding:NSUTF8StringEncoding],"r");

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
      char *r = fgetstr(hostnames[n],99,recentf);
    }
    if(feof(recentf)) usernames[n][0]=0; else fgetstr(usernames[n],99,recentf);
    if(feof(recentf)) passwords[n][0]=0; else fgetstr(passwords[n],99,recentf);
  }
  fclose(recentf);
}

void writeall_connections(char **hostnames,char **usernames,char **passwords) {

  NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
  NSString *fpath = [paths objectAtIndex:0];

  fpath = [fpath stringByAppendingString:@"/recentservers.txt"];

  printf("write file path: %s\n",[fpath cStringUsingEncoding:NSUTF8StringEncoding]);

  FILE *recentf = fopen([fpath cStringUsingEncoding:NSUTF8StringEncoding],"w");
  if(recentf==0) return;
  for(int n=0;n<RECENTCONNECTIONS;n++) {
    fprintf(recentf,"%s %s %s\n",hostnames[n],usernames[n],passwords[n]);
  }
  fclose(recentf);

}

void write_connection(const char* hostname,const char *username,const char *password) {

  if(hostname[0]==0) return;

  char *hostnames[RECENTCONNECTIONS];
  char *usernames[RECENTCONNECTIONS];
  char *passwords[RECENTCONNECTIONS];

  readall_connections(&hostnames,&usernames,&passwords);
  

  for(int n=(RECENTCONNECTIONS-1);n>=1;n--) {
    strcpy(hostnames[n],hostnames[n-1]);
    strcpy(usernames[n],usernames[n-1]);
    strcpy(passwords[n],passwords[n-1]);
  }

  int lastitem=RECENTCONNECTIONS-1;
  for(int n=0;n<RECENTCONNECTIONS;n++) {
    if(hostnames[n][0] == 0) {lastitem=n; break;}
  }


  // check if already present
  for(int n=1;n<RECENTCONNECTIONS;n++) {
    if(
    (strcmp(hostname,hostnames[n])==0) &&
    (strcmp(username,usernames[n])==0) &&
    (strcmp(password,passwords[n])==0)
    ) {
      strcpy(hostnames[0],hostname);
      strcpy(usernames[0],username);
      strcpy(passwords[0],password);
      
      for(int i=n;i<(RECENTCONNECTIONS-1);i++) {
        strcpy(hostnames[n],hostnames[n+1]);
        strcpy(usernames[n],usernames[n+1]);
        strcpy(passwords[n],passwords[n+1]);
      }
      hostnames[lastitem][0]=0;
      usernames[lastitem][0]=0;
      passwords[lastitem][0]=0;
    }
  }
  
  strcpy(hostnames[0],hostname);
  strcpy(usernames[0],username);
  strcpy(passwords[0],password);

  writeall_connections(hostnames,usernames,passwords);
}

