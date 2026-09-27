#!/bin/sh
# HiDPI (1440x2960 @ ~529 ppi): GTK scale 2, Qt too
export GDK_SCALE=2
export QT_SCALE_FACTOR=2
exec dbus-launch --exit-with-session startxfce4
