#!/bin/bash

#**************************************************************************************************#
#       OPTIMIZED AND CROSS PLATFORM SMPTE 2022-1 FEC LIBRARY IN C, JAVA, PYTHON, +TESTBENCH
#
#   Description    : Get VLC from git, add smpte2022lib and compile it
#   Main Developer : David Fischer (david.fischer.ch@gmail.com)
#   VLC Integrator : Jérémie Rossier (jeremie.rossier@gmail.com)
#   Copyright      : Copyright (c) 2008-2013 smpte2022lib Team. All rights reserved.
#   Sponsoring     : Developed for a HES-SO CTI Ra&D project called GaVi
#                    Haute école du paysage, d'ingénierie et d'architecture @ Genève
#                    Telecommunications Laboratory
#**************************************************************************************************#
#
# This file is part of smpte2022lib Project.
#
# This project is free software: you can redistribute it and/or modify it under the terms of the
# EUPL v. 1.1 as provided by the European Commission. This project is distributed in the hope that
# it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
# or FITNESS FOR A PARTICULAR PURPOSE.
#
# See the European Union Public License for more details.
#
# You should have received a copy of the EUPL General Public License along with this project.
# If not, see he EUPL licence v1.1 is available in 22 languages:
#     22-07-2013, <https://joinup.ec.europa.eu/software/page/eupl/licence-eupl>
#
# Retrieved from https://github.com/davidfischer-ch/smpte2022lib.git

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


