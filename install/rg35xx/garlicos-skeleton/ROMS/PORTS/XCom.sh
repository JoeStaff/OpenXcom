#!/bin/sh
progdir=$(dirname "$0")/openxcom
cd $progdir
HOME=$progdir

if ! [ -d ".config" ]; then
    mkdir .config
fi
if ! [ -d ".config/openxcom" ]; then
    mkdir .config/openxcom
fi
if ! [ -f ".config/openxcom/options.cfg" ]; then
    {
        echo options:
        echo \ \ displayHeight: 480
        echo \ \ displayWidth: 640
        echo \ \ keepAspectRatio: false
    } > .config/openxcom/options.cfg
fi

./openxcom
sync
