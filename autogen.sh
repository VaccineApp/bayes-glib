#!/bin/sh

# Run this to generate all the initial makefiles, etc.

srcdir=`dirname $0`
test -z "$srcdir" && srcdir=.

(test -f $srcdir/configure.ac) || {
    echo -n "**Error**: Directory "\`$srcdir\'" does not look like the"
    echo " top-level bayes-glib directory"
    exit 1
}

datadir=$srcdir/data
test -d "$datadir" || mkdir -p $datadir

libdir=$srcdir/libs
test -d "$libdir" || mkdir -p $libdir

touch ChangeLog
touch INSTALL

aclocal --install -I m4 || exit 1
gtkdocize || exit 1
autoreconf --force --install -Wno-portability || exit 1

if [ "$NOCONFIGURE" = "" ]; then
        $srcdir/configure "$@" || exit 1

        if [ "$1" = "--help" ]; then exit 0 else
                echo "Now type \`make\' to compile" || exit 1
        fi
else
        echo "Skipping configure process."
fi

set +x
