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

for dir in ${dirs}
do
    find ${dir} -depth -maxdepth 5 -type f -regextype posix-egrep -iregex ${regex_pat} -exec bash -c 'do_file "$0"' '{}' \;
    #find ${dir} -depth -maxdepth 5 -type f -regextype posix-egrep -iregex ${regex_pat} -exec bash -c 'do_file_with_exc "$0"' '{}' \;
done
