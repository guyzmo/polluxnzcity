VER=`python -c "import pollux._version; print pollux._version.__version__"`
REV=r0
ARCH=armv7

DIST=`pwd`
TARGET=$DIST/target

[ -d "$TARGET" ] || mkdir -p $TARGET
python2.7 setup.py install --install-layout unix --root $TARGET
cd $TARGET
install -d etc/pollux
install -d etc/defaults
cp $DIST/conf/*.json etc/pollux/
cp $DIST/conf/pollux_webserv.conf etc/defaults
tar -cvzf data.tar.gz etc/ usr/
cp $DIST/debian/* ./
sed -i "s/^Version: .*$/Version: $VER/g" control
tar -cvzf control.tar.gz control postinst postrm prerm
rm -rf $DIST/pollux-config_$VER-$REV_$ARCH.ipk
ar ru $DIST/pollux-config_$VER-$REV_$ARCH.ipk debian-binary data.tar.gz control.tar.gz

