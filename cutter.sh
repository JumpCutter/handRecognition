#!/usr/bin/env sh

cutstr=''
first=1
frame=0
./handrecog $1 | while read -r fr event gest; do
    if [ $event == 'end' ]; then
        frame=$fr
    elif [ $gest == 'up' ]; then
        if [ first == 0]; then
            cutstr=$cutstr'+'
        fi
        first=0
        $cutstr=$cutstr"gt(n\,$frame)*lt(n\,$fr)"
    fi
done
ffmpeg -i $1 -vf select="$cutstr" ${2:-out.${1:e}}"
