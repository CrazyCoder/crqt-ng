#!/bin/sh

dirs="src"
regex_pat=".*\.(h|c|cpp)"

die()
{
    echo $*
    exit 1
}

do_file()
{
    local f=$1
    echo -n "Processing file \"${f}\"... "
    clang-format -i "${f}" > /dev/null 2>&1
    echo "done"
    return 0
}

export -f die
export -f do_file

if [ ! -f .clang-format ]
then
    die "You can only run this script in <top_srcdir>!"
fi

uname_s=`uname -s`
is_darwin=no
test "x${uname_s}" = "xDarwin" && is_darwin="yes"

if [ "x${is_darwin}" = "xyes" ]
then
    for dir in ${dirs}
    do
        find -E ${dir} -depth -maxdepth 5 -type f -iregex ${regex_pat} -exec bash -c 'do_file "$0"' '{}' \;
    done
else
    for dir in ${dirs}
    do
        find ${dir} -depth -maxdepth 5 -type f -regextype posix-egrep -iregex ${regex_pat} -exec bash -c 'do_file "$0"' '{}' \;
    done
fi
