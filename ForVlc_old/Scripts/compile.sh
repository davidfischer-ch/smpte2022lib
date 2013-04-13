#!/bin/bash

#******************************************************************************#
#     OPTIMIZED AND CROSS PLATFORM SMPTE 2022-1 FEC LIBRARY IN C, +TESTBENCH
#
#  Description : Get VLC from git, add smpte2022lib and compile it
#  Authors     : David Fischer
#  Contact     : david.fischer.ch@gmail.com / david.fischer@hesge.ch
#  Copyright   : 2012 smpte2022lib Team. All rights reserved.
#******************************************************************************#
#
# This file is part of smpte2022lib.
#
# This project is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This project is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this project. If not, see <http://www.gnu.org/licenses/>
# 
# Retrieved from:
#   svn co https://claire-et-david.dyndns.org/prog/smpte2022lib

# In order to compile vlc succesfully, you need to install tools & build deps.
# Tip : In debian based system "sudo apt-get build-dep vlc" can be your friend !

cpuCount=`grep -c 'model name' /proc/cpuinfo`
jobCount=$((2*cpuCount))

cd ../Build || exit 1

if [ -d 'vlc_git' ]; then
  cd vlc_git && git pull
  cd ..
else
  git clone git://git.videolan.org/vlc.git --depth 1 vlc_git
fi

rsync -rtvh --exclude=.svn --progress --delete vlc_git/ vlc_smpte2022/

find ../Code -type f | while read file
do
  if echo $file | grep -q 'patch'; then
    name=`basename "$file" .patch`
    patch -c "vlc_smpte2022/modules/access/rtp/$name" "$file" || exit 1
  else
    echo "Copying file $file ..."
    cp "$file" vlc_smpte2022/modules/access/rtp/ || exit 1
  fi
done

exit 0
cd vlc_smpte2022 || exit 1
./bootstrap
./configure #--enable-xvideo --enable-sdl --enable-avcodec --enable-avformat --enable-swscale --enable-mad --enable-a52 --enable-dca --enable-libmpeg2 --enable-dvdnav --enable-faad --enable-vorbis --enable-ogg --enable-theora --enable-mkv --enable-freetype --enable-fribidi --enable-speex --enable-flac --enable-live555 --with-live555-tree=/home/jeremie/Documents/lib_live555/live/ --enable-caca --disable-gme --enable-skins2 --enable-alsa --enable-qt4 --enable-ncurses --enable-debug
make -j$jobCount


