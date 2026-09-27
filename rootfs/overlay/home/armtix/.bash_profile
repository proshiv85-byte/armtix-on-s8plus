[ -f ~/.bashrc ] && . ~/.bashrc

# /run/user/<uid> is created at boot by /etc/local.d/console.start
[ -d /run/user/$(id -u) ] && export XDG_RUNTIME_DIR=/run/user/$(id -u)

# Autologin on the phone screen (tty1) -> XFCE. Log: ~/.xsession.log
if [ -z "$DISPLAY" ] && [ "$(tty)" = /dev/tty1 ]; then
	exec startx -- -keeptty vt1 > ~/.xsession.log 2>&1
fi
