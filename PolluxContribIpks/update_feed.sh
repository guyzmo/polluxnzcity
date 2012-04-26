#!/bin/sh
#set -e

pkg_dir=$1
if [ -z $pkg_dir ] || [ ! -d $pkg_dir ]; then
  echo "Usage: $0 <package_directory>"
  exit 1
fi

cd $pkg_dir

echo "*** Creating packages"
for pkg in ../Pollux*; do
    if [ -f "$pkg/package.sh" ]; then
        echo "***** building package: $pkg"
        cd $pkg
        sh package.sh
        cp -f *.ipk $pkg_dir/
	cd -
    fi
done

rm Packages*

for pkg in `find $pkg_dir -maxdepth 1 -name '*.ipk' | sort`; do
  filename=`basename $pkg`
  pkgname=${filename%%_*}
  pkg_name=`echo $pkgname | cut -d'_' -f1`
  pkg_arch=`echo $pkgname | cut -d'_' -f2`
  pkg_ver=`echo $pkgname | cut -d'_' -f3`
  echo "Generating index for package ${pkgname}" >&2
  ## checking for multiple versions
  oldflag=
  for other in ${pkg_dir}/${pkgname}_*; do
    if [ $pkg != $other -a $other -nt $pkg ]; then
      oldflag=y
      break;
    fi;
  done
  if [ -z "$oldflag" ]; then
    ### Packages
    file_size=$(ls -l $pkg | awk '{print $5}')
    md5sum=$(md5sum $pkg | awk '{print $1}')
    # Take pains to make variable value sed-safe
    sed_safe_pkg=`echo $filename | sed -e 's/\\//\\\\\\//g'`
    ar x $pkg control.tar.gz
    tar xzOf control.tar.gz control | sed -e "s/^Filename:.*//g" | grep -v '^$' | sed -e "s/^Description:/Filename: $sed_safe_pkg\\
Size: $file_size\\
MD5Sum: $md5sum\\
Description:/" >> $pkg_dir/Packages
    gzip Packages
    zcat Packages.gz > Packages
    rm control.tar.gz
    ### Filelist
    ar x $pkg data.tar.gz
    if [ -f data.tar.gz ]; then
        tar tvzf data.tar.gz | awk '{print $NF}' | grep -v '/$' | sed "s,^\(.*/\)\(.*\)$,\2 $pkg_name:$pkg_arch:\1\2," | grep '.*:.*:.*/' >> $pkg_dir/Packages.filelist
        rm data.tar.gz
    fi
    ### Filestamps
    ls -l --time-style="+%s" | awk '{print $(NF-1) $(NF)}' >> $pkg_dir/Packages.stamps
  else
    echo >&2 "Skipped old file: $pkg ($other is newer)"
    mv $pkg $pkg_dir/old/
  fi
done

touch $pkg_dir/Packages.sig
