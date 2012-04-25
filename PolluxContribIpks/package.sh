DIST=`pwd`
TARGET=$DIST/target

ARCH=armv7a

##### LIGHTTPD-SYSTEMD
REV="r2"
VER=1.4.29

rm -rf $TARGET
mkdir -p $TARGET

install -d $TARGET/lib/systemd/system/
install -D $DIST/lighttpd-systemd/lighttpd.service $TARGET/lib/systemd/system/

cd $TARGET
tar -cvzf data.tar.gz lib 

cp $DIST/lighttpd-systemd/debian/* ./
tar -cvzf control.tar.gz control postinst postrm prerm

rm -rf $DIST/lighttpd-systemd_$VER-${REV}_$ARCH.ipk
ar ru $DIST/lighttpd-systemd_$VER-${REV}_$ARCH.ipk debian-binary data.tar.gz control.tar.gz

##### POLLUX-FULL

REV="r0"
VER=1.0.1

rm -rf $TARGET
mkdir -p $TARGET

cd $TARGET

cp $DIST/lighttpd-systemd/debian/* ./
tar -cvzf control.tar.gz control 

rm -rf $DIST/pollux-full_$VER-${REV}_$ARCH.ipk
ar ru $DIST/pollux-full_$VER-${REV}_$ARCH.ipk debian-binary control.tar.gz

#####

