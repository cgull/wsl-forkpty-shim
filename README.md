##forkpty shlib hack##

The WSL pty subsystem, as of build 14332, is too broken to use with
glibc's `forkpty()`.  The blocker I found is that `statvfs("/dev/pts/")`
fails, but I'm sure there are others.

However, the ptys are not completely unusable.  This is a simpler
implementation that dodges some pty bugs, so that script and Mosh's
mosh-server (and optional termemu dev tool) can run.  There are still
problems:

* `TIOC{G,S}WINSZ` fail on a pty master.
* I've been unable to make the slave a controlling tty yet.
* Device permissions are broken, requiring running as root.
* The library contains both a `forkpty()` and `openpty()` but only
  `forkpty()` works because of the above problems.

So...if you use this you get terminal sessions with no controlling
pty, no job control, no keyboard interrupts, broken window resize, and
some other termios brokenness I haven't investigated yet.  But it's
just barely usable enough to start hacking on pty-using programs in
WSL.

to use:
```shell
make
sudo sh -c 'LD_PRELOAD=./forkpty.so /usr/bin/script /tmp/barf'
```
