VER=`python -c "import pollux._version; print pollux._version.__version__"`
REV=r0
ARCH=armv7

DIST=`pwd`
TARGET=$DIST/target

[ -d "$TARGET" ] || mkdir -p $TARGET
python2.7 setup.py install --install-layout unix --root $TARGET
cd $TARGET
install -d etc/pollux
install -d etc/sudoers.d
install -d etc/defaults
cp $DIST/conf/*.json etc/pollux/
cp $DIST/conf/pollux_webserv.conf etc/defaults
cp $DIST/conf/pollux.sudoers etc/sudoers.d/pollux
chmod 440 etc/sudoers.d/pollux
tar -cvzf data.tar.gz ./etc/ ./usr/
cp $DIST/debian/* ./
sed -i "s/^Version: .*$/Version: $VER/g" control
tar -cvzf control.tar.gz control postinst postrm prerm
rm -rf $DIST/pollux-config_$VER-${REV}_$ARCH.ipk
ar ru $DIST/pollux-config_$VER-${REV}_$ARCH.ipk debian-binary data.tar.gz control.tar.gz

install -d $DIST/../feed/
cp -f $DIST/pollux-config_$VER-${REV}_$ARCH.ipk $DIST/../feed/

