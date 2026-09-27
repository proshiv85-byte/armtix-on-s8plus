#!/bin/sh
# Onboard defaults for the S8+ portrait screen (run inside the session's D-Bus)
gsettings set org.onboard layout '/usr/share/onboard/layouts/Phone.onboard'
gsettings set org.onboard.window docking-enabled true
gsettings set org.onboard.window docking-edge 'bottom'
gsettings set org.onboard.window docking-shrink-workarea true
gsettings set org.onboard.window force-to-top true
gsettings set org.onboard.window.portrait dock-expand true
gsettings set org.onboard.window.portrait dock-height 420
gsettings set org.onboard.icon-palette in-use true
gsettings set org.onboard.auto-show enabled false
gsettings set org.onboard start-minimized false
