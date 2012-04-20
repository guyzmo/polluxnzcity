install -d /etc/pollux
install -d /var/lib/pollux
install -d /usr/lib/pollux/extensions/datastores/
install -d /usr/lib/pollux/extensions/probers/
install config.json /etc/pollux
install sensors.json /etc/pollux
install sensors_list.json /etc/pollux
install pollux_webserv.conf /etc/pollux
install pollux_config.service /lib/systemd/system/

echo "pollux ALL=NOPASSWD: /bin/systemctl restart pollux_gateway.service" > /etc/sudoers.d/pollux
chmod 0440 /etc/sudoers.d/pollux

systemctl daemon-reload
python setup.py install --record /var/lib/pollux/installed_files.lst

adduser --system pollux

chown -R pollux /usr/lib/pollux
chown -R pollux /var/lib/pollux
chown -R pollux /etc/pollux

systemctl enable pollux_config.service
echo
echo "start me with: systemctl start pollux_config.service"
echo "or reboot"
