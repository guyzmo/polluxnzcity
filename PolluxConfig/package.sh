VER=0.1.0
REV=r0
ARCH=armv7

DIST=`dirname $0`

if [ "$TARGET" != "" ]; do
	python2.7 setup.py install --install-layout deb --root $TARGET
	cd $(TARGET)
	tar -cvzf data.tar.gz etc/ usr/ lib/ var/
	cp $(DIST)/debian/* ./
	tar -cvzf control.tar.gz control postinst postrm prerm
	ar ru $(DIST)/pollux-gateway_$(VER)-$(REV)_$(ARCH).ipk debian-binary data.tar.gz control.tar.gz
else
	echo "Please provide 'TARGET' environment variable as the root of package"
done

