#!/bin/sh
# ARMtix dream2lte status page (busybox httpd CGI)
echo "Content-type: text/plain"
echo
echo "== ARMtix on Samsung Galaxy S8+ (dream2lte) =="
echo
uname -a
echo
echo "Uptime:"
uptime
echo
echo "--- Battery / charging ---"
for f in status capacity voltage_now current_now temp charge_type health; do
	if [ -r "/sys/class/power_supply/battery/$f" ]; then
		printf '%-14s %s\n' "$f:" "$(cat /sys/class/power_supply/battery/$f 2>/dev/null)"
	fi
done
echo
echo "--- Power supply classes present ---"
ls /sys/class/power_supply/ 2>/dev/null
echo
echo "--- USB state ---"
cat /sys/class/power_supply/usb/online 2>/dev/null && echo " (1 = cable connected)"
echo
echo "--- Network ---"
ip addr show usb0 2>/dev/null | grep -E 'inet |state'
echo
echo "--- Last kernel messages ---"
dmesg 2>/dev/null | tail -25
