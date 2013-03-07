#ifndef RECENTRW_H
#define RECENTRW_H

#define RECENTCONNECTIONS 10

void readall_connections(char **hostnames,char **usernames,char **passwords);
void writeall_connections(char **hostnames,char **usernames,char **passwords);
void write_connection(const char* hostname,const char *username,const char *password);

#endif
