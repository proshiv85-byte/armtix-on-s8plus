[ -f ~/.bashrc ] && . ~/.bashrc

# /run/user/<uid> is created at boot by /etc/local.d/console.start
[ -d /run/user/$(id -u) ] && export XDG_RUNTIME_DIR=/run/user/$(id -u)

# Autologin on the phone screen (tty1) -> sway (GPU via libhybris), or XFCE
# with `echo xfce > ~/.config/armtix-session`.
# Logs: ~/.local/state/sway.log, ~/.xsession.log
if [ -z "$DISPLAY$WAYLAND_DISPLAY" ] && [ "$(tty)" = /dev/tty1 ]; then
	case $(cat ~/.config/armtix-session 2>/dev/null) in
		xfce) exec startx -- -keeptty vt1 > ~/.xsession.log 2>&1 ;;
		*)    exec armtix-sway ;;
	esac
fi
