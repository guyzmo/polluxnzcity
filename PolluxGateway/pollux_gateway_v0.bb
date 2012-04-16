DESCRIPTION = "Pollux Gateway"
HOMEPAGE = "http://polluxnzcity.org"
LICENSE = "GPLv3 (not yet)"
SECTION = "console/utils"
DEPENDS = "libcurl libjson libdl libgcrypt"
PROVIDES = "pollux_gateway"
SRC_URI = "git://git.bearstech.com/polluxnzcity.git;branch=master;protocol=ssh;user=ckab;subpath=PolluxGateway"

inherit systemd gitver

PV = "0.1+git${GITVER}"
PR = "r0"  # Package Revision, Update this whenever you change the recipe.

# For tarball packages (as opposed to git / svn which include the commit in the URI)
SRC_URI[md5sum] = ""
SRC_URI[sha256sum] = ""
S = "${WORKDIR}/pollux_gateway-${PV}"

do_compile () {
    oe_runmake all
}
do_install () {
    oe_runmake install
}

SYSTEMD_PACKAGES = "${PN}-systemd"
SYSTEMD_SERVICE = "${PN}.service"

