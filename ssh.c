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

int ssh_open(char *hostname,char *username,char *password) {
  #ifdef WIN32
  WSAStartup(MAKEWORD(2,0), &wsadata);
  #endif

  unsigned long hostaddr;
  hostaddr = inet_addr(hostname);
   
  struct sockaddr_in sin;

  sock = socket(AF_INET, SOCK_STREAM, 0);
  printf("socket: %d\n",sock);

  sin.sin_family = AF_INET;
  sin.sin_port = htons(22);
  sin.sin_addr.s_addr = hostaddr;
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
  for(i = 0; i < 20; i++) {
      fprintf(stderr, "%02X ", (unsigned char)fingerprint[i]);
  }
  fprintf(stderr, "\n");
  /* check what authentication methods are available */
  char *userauthlist = libssh2_userauth_list(session, username, strlen(username));
  fprintf(stderr, "Authentication methods: %s\n", userauthlist);
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
    return -2;
  } else {
    fprintf(stderr, "\tAuthentication by password succeeded.\n");
  }
      ///* Or by public key */
      //if (libssh2_userauth_publickey_fromfile(session, username, keyfile1,
      //                                        keyfile2, password)) {
      //    fprintf(stderr, "\tAuthentication by public key failed!\n");
      //    goto shutdown;
      //} else {
      //    fprintf(stderr, "\tAuthentication by public key succeeded.\n");
      //}
  /* Some environment variables may be set,
   * It's up to the server which ones it'll allow though
   */

/* Request a shell */ 
  if (!(channel = libssh2_channel_open_session(session))) {
    fprintf(stderr, "Unable to open a session\n");
    return -4;
  }

  libssh2_channel_setenv(channel, "TERM", "xterm");

  /* Request a terminal with 'vanilla' terminal emulation
   * See /etc/termcap for more options
   */
  int error=0;
  if (error = libssh2_channel_request_pty(channel, "vanilla")) {
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
  libssh2_channel_set_blocking(channel,1);
  return 1;
}

int ssh_write(char *bytes,int len) {
  libssh2_channel_write(channel,bytes,len);
     // libssh2_channel_write_stderr()
     
     // Blocking mode may be (en|dis)abled with: libssh2_channel_set_blocking()
     // If the server send EOF, libssh2_channel_eof() will return non-0
     // To send EOF to the server use: libssh2_channel_send_eof()
     // A channel can be closed with: libssh2_channel_close()
     // A channel can be freed with: libssh2_channel_free()
}

int ssh_read(char *bytes,int len) {
  printf("in read\n");
  return libssh2_channel_read(channel,bytes,len);
     // libssh2_channel_read_stderr()
}

int ssh_close() {

    if (channel) {
        libssh2_channel_free(channel);
        channel = NULL;
    }

    /* Other channel types are supported via:
     * libssh2_scp_send()
     * libssh2_scp_recv()
     * libssh2_channel_direct_tcpip()
     */

    libssh2_session_disconnect(session,
                               "Normal Shutdown, Thank you for playing");
    libssh2_session_free(session);

#ifdef WIN32
    closesocket(sock);
#else
    close(sock);
#endif
    fprintf(stderr, "all done!\n");

    libssh2_exit();
}

int ssh_resize(int cols,int rows){}
