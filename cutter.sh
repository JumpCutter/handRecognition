#!/bin/bash
# #!/bin/sh
# #!/usr/bin/env sh

ext=${1##*.}
vidname=0
fps=`ffmpeg -i $1 2>&1 | sed -n "s/.*, \(.*\) fp.*/\1/p"`
frame=$(echo "0/$fps" | bc -l | sed 's/^\./0\./')

framecount=$(ffmpeg -i $1 -map 0:v:0 -c copy -f null - 2>&1 | \
        grep frame | sed -e 's/frame=\W*\([0-9]*\).*/\1/')
echo $framecount 1>&2

tempdir=$(mktemp -d tmpvids.XXXXXX)
trap 'rm -rf "$tempdir"' 0 1 2 9 15

./handrecog $1 | \
while read fr event gest; do
    [ -n "$fr" ] || continue
    echo $fr $event $gest "$(echo $fr/$fps | bc)s"
    if [ "$event" == 'end' ]; then
        frame=$fr
    elif [ "$gest" == 'up' ]; then
        #echo $frame $fr $fps
        s=$(echo "$frame/$fps" | bc -l | sed 's/^\./0\./')
        e=$(echo "$fr   /$fps" | bc -l | sed 's/^\./0\./')
        #echo $s $e
        ffmpeg -nostdin -y -i $1 -ss $s -to $e $tempdir/$vidname.$ext 2>/dev/null
        echo "file '$vidname.$ext'" >> $tempdir/filelist.txt
        vidname=$((vidname+1))
    fi
done

ffmpeg -nostdin -y -f concat -i $tempdir/filelist.txt -c copy $1 2>/dev/null
# ffmpeg -nostdin -y -f concat -i $tempdir/filelist.txt -c copy "/app/test2.mp4" 2>/dev/null
# chmod a+wx "$1"
