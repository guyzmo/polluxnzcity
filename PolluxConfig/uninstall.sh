echo -n "Stopping systemd service: "
systemctl stop pollux_config.service && echo "done" || echo "failed"
echo -n "Disabling systemd service: "
systemctl disable pollux_config.service 2> /dev/null && echo "done" || echo "failed"
echo -n "Reloading systemd: "
systemctl daemon-reload && echo "done" || echo "failed"

echo -n "Removing all files: configuration: "
rm -rf  /etc/pollux/ /lib/systemd/system/pollux_config.service && echo -n "done" || echo -n "failed"
echo -n " ; python files: "
cat .installed_files.lst | xargs rm -rf && echo "done" || echo "failed"
rm -f .installed_files.lst

echo
echo "pollux config uninstalled"
