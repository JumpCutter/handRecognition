#!/usr/bin/env sh

ext=${1##*.}
vidname=0
frame=0
fps=`ffmpeg -i $1 2>&1 | sed -n "s/.*, \(.*\) fp.*/\1/p"`

mkdir -p tmpvids
echo "" > filelist.txt

./handrecog $1 | \
while read fr event gest; do
    if [ $event = 'end' ]; then
        frame=$fr
    elif [ $gest = 'up' ]; then
        #echo $frame $fr $fps
        s=$(echo "$frame/$fps" | bc -l | sed 's/^\./0\./')
        e=$(echo "$fr   /$fps" | bc -l | sed 's/^\./0\./')
        #echo $s $e
        ffmpeg -y -i $1 -ss $s -to $e -c copy tmpvids/$vidname.$ext 2>/dev/null
        echo "file 'tmpvids/$vidname.$ext'" >> filelist.txt
        vidname=$((vidname+1))
    fi
done

ffmpeg -y -f concat -i filelist.txt -c copy "${2:-out.$ext}" 2>/dev/null
