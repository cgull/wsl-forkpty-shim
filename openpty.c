/*
    Windows Subsystem for Linux openpty library shim
    Copyright 2016 John Hood

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    In addition, as a special exception, the copyright holders give
    permission to link the code of portions of this program with the
    OpenSSL library under certain conditions as described in each
    individual source file, and distribute linked combinations including
    the two.

    You must obey the GNU General Public License in all respects for all
    of the code used other than OpenSSL. If you modify file(s) with this
    exception, you may extend this exception to your version of the
    file(s), but you are not obligated to do so. If you do not wish to do
    so, delete this exception statement from your version. If you delete
    this exception statement from all source files in the program, then
    also delete it here.
*/

// test:
// cc -DMAIN -g -o openpty openpty.c -lutil
// cc -DMAIN -DFORKPTY -g -o openpty openpty.c -lutil
// shlib:
// cc -DFORKPTY -shared -fPIC -g -o openpty.so openpty.c -lutil

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <pty.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int openpty(int *mfd, int *sfd, char *slavename, const struct termios *tp, const struct winsize *wp)
{
  char sbuf[256];
  if (!slavename) slavename = sbuf;
  // ignore termios and winsize for now
  *mfd = open("/dev/ptmx", O_RDWR | O_NOCTTY);
  // *mfd = posix_openpt(O_RDWR | O_NOCTTY);
  if (*mfd == -1) {
    printf("open master failed\n");
    return -1;
  }
  int rv = ptsname_r(*mfd, slavename, 256);
  if (rv != 0) {
    printf("ptsname failed\n");
    return -1;
  }
  printf("slavename is %s\n", slavename);
  rv = grantpt(*mfd);
  if (rv != 0) {
    printf("grantpt failed\n");
    return -1;
  }
  rv = unlockpt(*mfd);
  if (rv != 0) {
    printf("unlockpt failed\n");
    return -1;
  }
  *sfd = open(slavename, O_RDWR | O_NOCTTY);
  if (*mfd == -1) {
    printf("open slave failed\n");
    return -1;
  }
  return 0;
}

#ifdef FORKPTY
pid_t
forkpty(int *mfd, char *slavename, const struct termios *tp, const struct winsize *wp)
{
  char sbuf[256];
  if (!slavename) slavename = sbuf;
  // ignore termios and winsize for now
  *mfd = open("/dev/ptmx", O_RDWR | O_NOCTTY);
  // *mfd = posix_openpt(O_RDWR | O_NOCTTY);
  if (*mfd == -1) {
    printf("open master failed\n");
    return -1;
  }
  int rv = ptsname_r(*mfd, slavename, 256);
  if (rv != 0) {
    printf("ptsname failed\n");
    return -1;
  }
  printf("slavename is %s\n", slavename);
  rv = grantpt(*mfd);
  if (rv != 0) {
    printf("grantpt failed\n");
    return -1;
  }
  rv = unlockpt(*mfd);
  if (rv != 0) {
    printf("unlockpt failed\n");
    return -1;
  }
  int pid;
  if ((pid = fork()) == 0) {
    FILE *errfd = fopen("child.err", "w");
    setlinebuf(errfd);
    fprintf(errfd, "pid is %d, tty is %s\n", getpid(), ttyname(0));
    close(*mfd);
    close(0);
    close(1);
    close(2);
    setsid();
    int sfd = open(slavename, O_RDWR);
    ioctl(sfd, TIOCSCTTY, 0);
    dup2(sfd, 0);
    dup2(sfd, 1);
    dup2(sfd, 2);
    if (sfd > 2) close(sfd);
  }
  return pid;
}
#endif

#ifdef MAIN
int
main(int argc, char **argv)
{
  int mfd = -1, sfd = -1;
  char slavename[256];
#ifdef FORKPTY
  int pid;
  if ((pid = forkpty(&mfd, slavename, NULL, NULL)) == 0) {
    FILE *errfd = fopen("child.err", "w");
    setlinebuf(errfd);
    fprintf(errfd, "pid is %d, tty is %s\n", getpid(), ttyname(0));
    char *args[] = { "/bin/cat", NULL };
    return execv("/bin/cat", args);
    // return system("/bin/cat");
  } else if (pid < 0) {
    printf("fork failed\n");
    exit(1);
  }
  printf("master fd %d, tty name %s\n", mfd, slavename);
#else
  int rv = openpty(&mfd, &sfd, slavename, NULL, NULL);
  printf("openpty rets %d, errno %d, errstr %s master fd %d slave fd %d\n", rv, errno, strerror(errno), mfd, sfd);
  if (rv == -1) exit(1);
  int pid;
  if ((pid = fork()) == 0) {
    FILE *errfd = fopen("child.err", "w");
    setlinebuf(errfd);
    fprintf(errfd, "pid is %d, tty is %s\n", getpid(), ttyname(0));
    close(mfd);
    close(0);
    close(1);
    close(2);
    close(sfd);
    setsid();
    sfd = open(slavename, O_RDWR);
    dup2(sfd, 0);
    dup2(sfd, 1);
    dup2(sfd, 2);
    if (sfd > 2) close(sfd);
    fprintf(errfd, "tty is now %s\n", ttyname(0));
    fprintf(errfd, "login_tty rets %d, errno %d, errstr %s\n", rv, errno, strerror(errno));
    if (rv == -1) exit(1);
    system("/usr/bin/tty");
    char *args[] = { "/bin/cat", NULL };
    return execv("/bin/cat", args);
  } else if (pid < 0) {
    printf("fork failed\n");
    exit(1);
  }
#endif
  usleep(100000);
  if (kill(pid, 0)) {
    printf("child exited\n");
    return 1;
  }
  system("/usr/bin/tty");
  char *args[] = { "/bin/bash", NULL };
  execv("/bin/bash", args);
  return 0;
}
#endif