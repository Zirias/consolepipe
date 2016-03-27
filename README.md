# xcons
## X CONsole Socket

### Usage

These are two little tools for getting a "console log" on your desktop --
like `xconsole`, just different. The basic idea is having `syslogd` pipe to
a service publishing the lines over a unix domain socket.

The log lines are read from the socket and displayed by a little `curses`
tool running inside an X terminal emulator.

Use for example this line in your syslog configuration:

    *.err;kern.notice;auth.notice;mail.crit |exec /usr/local/bin/xcons_service

For display, I use `konsole` with a custom profile starting
`/usr/local/bin/xcons_curses` instead of a shell -- In KDE, you can match
that with a `kwin` window-specific rule quite easily because `konsole` creates
the window first with the name of the profile as title. Using that, it's for
example possible to keep the window in the background all the time, to have it
without decorations, to hide it from the taskbar, and so on ...

### Build and install

To build and install **xcons**, just type

    make
    make install

If you want to install stripped binaries, use `make install-strip` instead.

The following make variables are available:

  - **prefix**: root of the tree to install to, default `/usr/local`
  - **localstatedir**: base dir for local state information, default
    `[prefix]/var`
  - **runstatedir**: run state information (the socket will live here),
    default `[localstatedir]/run`
  - **V**: verbose build when set to anything other than `0`.

### Example screenshot

![xcons-0.1 in konsole on kde4](../../blob/files/xcons-0.1_kde4.png?raw=true)
This screenshot shows `xcons_curses` running in a `konsole` on kde4 (at the
bottom of the desktop)
