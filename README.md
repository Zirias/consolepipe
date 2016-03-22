# xcons
### X CONsole Socket

These are two little tools for getting a "console log" on your desktop --
like `xconsole`, just different. The basic idea is having `syslogd` pipe to
a service publishing the lines over a unix domain socket.

The log lines are read from the socket and displayed by a little `curses`
tool running inside an X terminal emulator.

Use for example this line in your syslog configuration:

    *.err;kern.notice;auth.notice;mail.crit |exec /home/felix/bin/xcons_service /usr/local/var/run/xcons.sock

For display, I use `konsole` with a custom profile starting
`/home/felix/bin/xcons_curses /usr/local/var/run/xcons.sock` instead of
a shell -- In KDE, you can match that with a `kwin` window-specific rule
quite easily because `konsole` creates the window first with the name of
the profile as title. Using that, it's for example possible to keep the window
in the background all the time, to have it without decorations, to hide it
from the taskbar, and so on ...

For a screenshot,
[see this forum post](https://forums.freebsd.org/threads/55573/) for now.
