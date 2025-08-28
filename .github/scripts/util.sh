
. .github/scripts/conf.sh

SOURCE_DIR=`pwd`

log_group() {
    echo "::group::$*"
}

log_endgroup() {
    echo "::endgroup::"
}

clone_source() {
    local pkgname="$1"
    local url="$2"
    local ref="$3"

#    log_group "Cloning $pkgname"
    $SOURCE_DIR/.github/scripts/git-smart-checkout.sh \
        --name "$pkgname" \
        --url "$url" \
        --ref "$ref"
#    log_endgroup
}

build_meson() {
    local pkgname="$1"
    local url="$2"
    local ref="$3"
    shift
    shift
    shift || true
    if [ -f $X11_PREFIX/$pkgname.DONE ]; then
        echo "package $pkgname already built"
    else
        log_group "Build $pkgname (meson)"
        clone_source "$pkgname" "$url" "$ref"
        (
            cd $pkgname
            meson "$@" build -Dprefix=$X11_PREFIX
            ninja -j${FDO_CI_CONCURRENT:-4} -C build install
        )
        touch $X11_PREFIX/$pkgname.DONE
        log_endgroup
    fi
}

build_ac() {
    local pkgname="$1"
    local url="$2"
    local ref="$3"
    shift
    shift
    shift || true
    if [ -f $X11_PREFIX/$pkgname.DONE ]; then
        echo "package $pkgname already built"
    else
        log_group "Build $pkgname (autoconf)"
        clone_source "$pkgname" "$url" "$ref"
        (
            cd $pkgname
            ./autogen.sh --prefix=$X11_PREFIX
            make -j${FDO_CI_CONCURRENT:-4} install
        )
        touch $X11_PREFIX/$pkgname.DONE
        log_endgroup
    fi
}

build_drv_ac() {
    local pkgname="$1"
    local url="$2"
    local ref="$3"
    shift
    shift
    shift || true
    log_group "Build driver $pkgname (autoconf)"
    clone_source "$pkgname" "$url" "$ref"
    (
        cd $pkgname
        ./autogen.sh # --prefix=$X11_PREFIX
        make -j${FDO_CI_CONCURRENT:-4} # install
    )
    log_endgroup
}

build_ac_xts() {
    local pkgname="$1"
    local url="$2"
    local ref="$3"
    shift
    shift
    shift || true
    if [ -f $X11_PREFIX/$pkgname.DONE ]; then
        echo "package $pkgname already built"
    else
        log_group "Build XTS (autoconf)"
        clone_source "$pkgname" "$url" "$ref"
        (
            cd $pkgname
            CFLAGS='-fcommon'
            ./autogen.sh --prefix=$X11_PREFIX CFLAGS="$CFLAGS"
            if [ "$X11_OS" = "Darwin" ]; then
                make -j${FDO_CI_CONCURRENT:-4} install tetexec.cfg
            else
                xvfb-run make -j${FDO_CI_CONCURRENT:-4} install tetexec.cfg
            fi
        )
        touch $X11_PREFIX/$pkgname.DONE
        log_endgroup
    fi
}
