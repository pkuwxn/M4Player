#!/bin/bash

strip --strip-all bin/Release/*.so
strip --strip-all bin/Release/m4

cp bin/Release/*.so ../Assets/Plugins
cp bin/Release/m4 ../Assets

BLD_DATE=$(date +%Y%m%d)
rm -f M4Player*.tar.gz

cd ../Assets
PCK_CMD="tar -c -a --exclude=Default.Playlist  --exclude=Plugins.* --exclude=*.lrc --exclude=Thumbs.db -f ../Build/M4Player.${BLD_DATE}.tar.gz ."
$(${PCK_CMD})

