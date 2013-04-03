#include "libssh2_config.h"
#include <libssh2.h>
#include <libssh2_sftp.h>

#ifdef HAVE_WINDOWS_H
#include <windows.h>
#endif
#ifdef HAVE_WINSOCK2_H
#include <winsock2.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif

#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <ctype.h>
#include <netdb.h>

//const char *keyfile1="~/.ssh/id_rsa.pub";
//const char *keyfile2="~/.ssh/id_rsa";
//const char *username="username";
//const char *password="password";

#ifdef WIN32
WSADATA wsadata;
#endif
  
int rc, sock, i, auth_pw = 0;
LIBSSH2_SESSION *session;
LIBSSH2_CHANNEL *channel;
char fingerprintstr[100];


int ssh_open_preshell(char *hostname,char *username,char *password,char *fingerprintstrin,char *pubkeypath,char *privkeypath) {
  #ifdef WIN32
  WSAStartup(MAKEWORD(2,0), &wsadata);
  #endif
  
  struct sockaddr_in sin;

  //unsigned long hostaddr;
  //hostaddr = inet_addr(hostname);

  struct addrinfo *result;
  struct addrinfo *res;
  int error;
 
  // resolve
  error = getaddrinfo(hostname, "22", NULL, &result);
  if (error != 0) {
    return -1;
  }

  /* loop over all returned results and do inverse lookups, that the last lookup found. */
  for (res = result; res != NULL; res = res->ai_next) {   
    char hostname[1000] = "";

    error = getnameinfo(res->ai_addr, res->ai_addrlen, hostname, 1000, NULL, 0, 0); 
    if (error != 0) {
      continue;
    }

    if (*hostname != '\0') {
      printf("hostname: %s\n", hostname);
      sin = *((struct sockaddr_in *) (res->ai_addr));
      break;
    }
  }

  freeaddrinfo(result);

  sock = socket(AF_INET, SOCK_STREAM, 0);
  printf("socket: %d\n",sock);
  int set=1;
  
  //If you don't set this you'll get a sigpipe on lock on iOS.
  setsockopt(sock,SOL_SOCKET,SO_NOSIGPIPE,(void *)&set,sizeof(int));

  //sin.sin_family = AF_INET;
  sin.sin_port = htons(22);
  //sin.sin_addr.s_addr = hostaddr;
  if (connect(sock, (struct sockaddr*)(&sin),
              sizeof(struct sockaddr_in)) != 0) {
      fprintf(stderr, "failed to connect to: %s error: %d\n",hostname,errno);

      return -1;
  }

  /* Create a session instance and start it up. This will trade welcome
   * banners, exchange keys, and setup crypto, compression, and MAC layers
   */
  session = libssh2_session_init();
  if (libssh2_session_handshake(session, sock)) {
      fprintf(stderr, "Failure establishing SSH session\n");
      return -1;
  }

  /* At this point we havn't authenticated. The first thing to do is check
   * the hostkey's fingerprint against our known hosts Your app may have it
   * hard coded, may go to a file, may present it to the user, that's your
   * call
   */
  const char *fingerprint;
  fingerprint = libssh2_hostkey_hash(session, LIBSSH2_HOSTKEY_HASH_SHA1);
  fprintf(stderr, "Fingerprint: ");
  
  char *fingerpos=fingerprintstr;
  for(i = 0; i < 20; i++) {
      sprintf(fingerpos, "%02X", (unsigned char)fingerprint[i]);
      fingerpos+=2;
      if(i!=19) {
        fingerpos[0]=':';
        fingerpos++;
        fingerpos[0]=0;
      }
  }
  
  if((fingerprintstrin != 0) && (fingerprintstrin[0] != 0)) {
    //finger print was not valid.
    if(strcmp(fingerprintstr,fingerprintstrin) != 0) {
      return -5;
    }
  }
  
  fprintf(stderr,fingerprintstr);
  fprintf(stderr, "\n");
  /* check what authentication methods are available */
  char *userauthlist = libssh2_userauth_list(session, username, strlen(username));
  fprintf(stderr, "Authentication methods: %s\n", userauthlist);
  if(userauthlist == 0) return -6;
  if (strstr(userauthlist, "password") != NULL) {
      auth_pw |= 1;
  }
  if (strstr(userauthlist, "keyboard-interactive") != NULL) {
      auth_pw |= 2;
  }
  if (strstr(userauthlist, "publickey") != NULL) {
      auth_pw |= 4;
  }

  /* We could authenticate via password */
  if (libssh2_userauth_password(session, username, password)) {
    fprintf(stderr, "\tAuthentication by password failed!\n");
    //return -2;
  } else {
    fprintf(stderr, "\tAuthentication by password succeeded.\n");
    return 0;
  }
      ///* Or by public key */
  if (libssh2_userauth_publickey_fromfile(session, username, pubkeypath,
                                          privkeypath, 0)) {
      fprintf(stderr, "\tAuthentication by public key failed!\n");
      return -1;
  //    goto shutdown;
  } else {
      fprintf(stderr, "\tAuthentication by public key succeeded.\n");
      return 0;
  }
  return 0;
}

int ssh_openshell() {
  int error;
  /* Request a shell */ 
  if (!(channel = libssh2_channel_open_session(session))) {
    fprintf(stderr, "Unable to open a session\n");
    return -4;
  }

  libssh2_channel_setenv(channel, "TERM", "xterm");

  /* Request a terminal with 'vanilla' terminal emulation
   * See /etc/termcap for more options
   */
  error=0;
  if (error = libssh2_channel_request_pty(channel, "xterm")) {
    fprintf(stderr, "Failed requesting pty: %d\n",error);
    if(error == LIBSSH2_ERROR_ALLOC) printf("alloc error\n");
    if(error == LIBSSH2_ERROR_SOCKET_SEND) printf("eror socket\n");
    if(error == LIBSSH2_ERROR_CHANNEL_REQUEST_DENIED) printf("ddeeenied\n");
    if(error == LIBSSH2_ERROR_EAGAIN) printf("eagain\n");

    return -3;
  }

  /* Open a SHELL on that pty */
  if (libssh2_channel_shell(channel)) {
    fprintf(stderr, "Unable to request shell on allocated pty\n");
    return -1;
  }
  printf("connection successful\n");
  libssh2_channel_set_blocking(channel,0);//nonblocking
  return 0;
}

int ssh_open(char *hostname,char *username,char *password,char *fingerprintstrin,char *pubkeypath,char *privkeypath) {
  int r1 = ssh_open_preshell(hostname,username,password,fingerprintstrin,pubkeypath,privkeypath);
  if(r1 != 0) return r1;
  int r2 = ssh_openshell();
  return r2;
}

char *ssh_fingerprintstr() {
  return fingerprintstr;
}

int ssh_write(unsigned char *bytes,int len) {
  if(channel == 0) return -1;
  if(libssh2_channel_eof(channel) != 0) {return -1;}

  int l = libssh2_channel_write(channel,bytes,len);
     // libssh2_channel_write_stderr()
     
     // Blocking mode may be (en|dis)abled with: libssh2_channel_set_blocking()
     // If the server send EOF, libssh2_channel_eof() will return non-0
     // To send EOF to the server use: libssh2_channel_send_eof()
     // A channel can be closed with: libssh2_channel_close()
     // A channel can be freed with: libssh2_channel_free()
}

int ssh_read(char *bytes,int len) {
  if(channel == 0) return -1;
  
  if(libssh2_channel_eof(channel) != 0) {return -1;}
  
  int l = libssh2_channel_read(channel,bytes,len);
  
  if(l == LIBSSH2_ERROR_EAGAIN) return 0;
  if(l < 0) ssh_close();

  return l;
}

int is_closed() {
  if(channel == 0) return 0;
  return 1;
}

int ssh_close() {
  if (channel != 0) {
    libssh2_channel_free(channel);
    channel  = NULL;
  }

  if(session != 0) {
    libssh2_session_disconnect(session, "Sesson closed");
    libssh2_session_free(session);
  }

#ifdef WIN32
  closesocket(sock);
#else
  close(sock);
#endif
  fprintf(stderr, "all done!\n");

  session = 0;
  channel = 0;
  libssh2_exit();
  return 0;
}

int ssh_resize(int cols,int rows){

  if(channel == 0) return 1;
  if(libssh2_channel_eof(channel)!=0) {return -1;}
  libssh2_channel_request_pty_size(channel,cols,rows);
}

int ssh_getfile(char *remotepath,char *localpath) {
    /* Request a file via SCP */
    struct stat fileinfo;
    channel = libssh2_scp_recv(session, remotepath, &fileinfo);
 
    if (!channel) {
        fprintf(stderr, "Unable to open a session: %d\n",
                libssh2_session_last_errno(session));
        
        char *errstr1 = malloc(1000);
        int errlen;
        libssh2_session_last_error(session,&errstr1,&errlen,0);
        fprintf(stderr,"errstr: %s\n",errstr1);
        return -1;
    }
    
    off_t got=0;
    int fid = fopen(localpath,"w");
    while(got < fileinfo.st_size) {
        char mem[1024];
        int amount=sizeof(mem);
 
        if((fileinfo.st_size -got) < amount) {
            amount = fileinfo.st_size -got;
        }
 
        rc = libssh2_channel_read(channel, mem, amount);

        if(rc > 0) {
            for(int n=0;n<rc;n++) {
                int c = mem[n];
                fputc(c,fid);
            }
            //write(fid, mem, rc);
        }
        else if(rc < 0) {
            fprintf(stderr, "libssh2_channel_read() failed: %d\n", rc);
            return -2;
            break;
        }
        got += rc;
    }
 
    libssh2_channel_free(channel);
    fclose(fid);

    channel = NULL;
}

int ssh_getkeys(char *pubkeypath,char *privkeypath) {

  int r1 = ssh_getfile("/home/new/.ssh/id_rsa",privkeypath);
  int r2 = ssh_getfile("/home/new/.ssh/id_rsa.pub" ,pubkeypath);
  if((r1 == 0) && (r2 == 0)) return 0;
  return 1;
}