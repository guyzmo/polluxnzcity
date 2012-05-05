DIST=`pwd`
TARGET=$DIST/target

ARCH=armv7a

##### LIGHTTPD-SYSTEMD
REV="r3"
VER=1.4.29

rm -rf $TARGET
mkdir -p $TARGET

install -d $TARGET/lib/systemd/system/
install -D $DIST/lighttpd-systemd/lighttpd.service $TARGET/lib/systemd/system/lighttpd.service

cd $TARGET
tar -cvzf data.tar.gz ./lib 

cp $DIST/lighttpd-systemd/debian/* ./
tar -cvzf control.tar.gz control postrm prerm postinst

rm -rf $DIST/lighttpd-systemd_$VER-${REV}_$ARCH.ipk
ar ru $DIST/lighttpd-systemd_$VER-${REV}_$ARCH.ipk debian-binary data.tar.gz control.tar.gz

##### POLLUX-FULL

REV="r0"
VER=0.2.0

rm -rf $TARGET
mkdir -p $TARGET

install -d $TARGET/etc/cron.daily/
install -d $TARGET/etc/cron.hourly/
install -D $DIST/pollux-full/etc/cron.daily/keep_pollux_uptodate $TARGET/etc/cron.daily/keep_pollux_uptodate
install -D $DIST/pollux-full/etc/cron.hourly/keep_time_in_sync $TARGET/etc/cron.hourly/keep_time_in_sync

cd $TARGET
tar -cvzf data.tar.gz ./etc

cp $DIST/pollux-full/debian/* ./
sed -i "s/^Version: .*$/Version: $VER/g" control
VER_GW=`grep 'VERSION' $DIST/../PolluxGateway/include/version.h | awk '{print $3}'`
VER_CF=`grep '__version__' $DIST/../PolluxConfig/pollux/_version.py | sed "s/.*'\(.*\)'.*/\1/"`
sed -i "s/^Depends: .*$/Depends: pollux-gateway >=$VER_GW, pollux-config >=$VER_CF/g" control
tar -cvzf control.tar.gz control


rm -rf $DIST/pollux-full_$VER-${REV}_$ARCH.ipk
ar ru $DIST/pollux-full_$VER-${REV}_$ARCH.ipk debian-binary control.tar.gz data.tar.gz

##### HACKABLEDEVICES-FEED

REV="r0"
VER=1.0.0

rm -rf $TARGET
mkdir -p $TARGET

cd $TARGET
install -d $TARGET/etc/opkg/
install -D $DIST/polluxnzcity-feed/etc/opkg/polluxnzcity-feed.conf $TARGET/etc/opkg/
tar -cvzf data.tar.gz ./etc

cp $DIST/polluxnzcity-feed/debian/* ./
tar -cvzf control.tar.gz control

rm -rf $DIST/polluxnzcity-feed_$VER-${REV}_$ARCH.ipk
ar ru $DIST/polluxnzcity-feed_$VER-${REV}_$ARCH.ipk debian-binary control.tar.gz data.tar.gz

#####

cd $DIST
cp *.ipk ../feed

