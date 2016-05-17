##forkpty shlib hack##

This is a hack to get ptys somewhat working on Windows 10 fast-ring
builds with Bash on Windows / Windows Subsystem for Linux installed.
It's mostly intended for programs that use `forkpty()`.

The WSL pty subsystem, as of build 14332, is too incomplete to use
with glibc's `forkpty()`.  The blocker I found is that
`statvfs("/dev/pts/")` fails, but I'm sure there are others.

However, the ptys are not completely unusable.  This is a simpler
implementation that dodges some pty bugs, so that `script` and [with
some hacks](https://github.com/cgull/mosh/tree/wsl-workarounds))
Mosh's `mosh-server` (and optional `termemu` example program) can run.
It implements `forkpty()`/`openpty()`/`login_tty()`, the core BSD API
to ptys.  It also implements `posix_openpt()` and glibc's `getpt()`.
There are still problems:

* `TIOC{G,S}WINSZ` fail on a pty master or slave.
* I've been unable to make the slave a controlling tty yet
  via `open()` or `ioctl(..., TIOCSCTTY, ...)`.
* Device permissions are broken, requiring running as root.
* `openpty()` works but callers may themselves trip on unimplemented
  termios features if they don't use `login_tty()`
* There are several different ways to access ptys:  the BSD
  `forkpty()` calls, the POSIX API, glibc's API, `open()` + `ioctl()`,
  and more.  This only implements a subset.

So...if you use this you get terminal sessions with no controlling
pty, no job control, no keyboard interrupts, broken window resize, and
some other termios brokenness I haven't investigated yet.  As of Build
14332, I think ptys don't even have a real termio implementation, only
a stub implementation that copies bytes back and forth and adds CR on
output.  But it's just barely usable enough to start further work on
pty-using programs in WSL.

to use:
```shell
make
sudo sh -c 'LD_PRELOAD=./forkpty.so /usr/bin/script /tmp/barf'
```
