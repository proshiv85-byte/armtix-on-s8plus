[ -f ~/.bashrc ] && . ~/.bashrc

# /run/user/<uid> is created at boot by /etc/local.d/console.start
[ -d /run/user/$(id -u) ] && export XDG_RUNTIME_DIR=/run/user/$(id -u)

# Autologin on the phone screen (tty1) -> sway (GPU via libhybris).
# Log: ~/.local/state/sway.log
if [ -z "$DISPLAY$WAYLAND_DISPLAY" ] && [ "$(tty)" = /dev/tty1 ]; then
	exec armtix-sway
fi
