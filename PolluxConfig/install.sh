install -d /etc/pollux
install config.json /etc/pollux
install sensors.json /etc/pollux
install sensors_list.json /etc/pollux
install pollux_webserv.conf /etc/pollux
install pollux_config.service /lib/systemd/system/
systemctl daemon-reload
python setup.py install
systemctl enable pollux_config.service
echo
echo "start me with: systemctl start pollux_config.service"
echo "or reboot"
