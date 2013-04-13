#!/bin/bash

if [ ! $# -eq 6 ]
then
  echo "Usage: $0 sourceFilename L D P rawDestFilename davidDestFilename"
  echo "Example: $0 LesInconnusBioman.mpg 5 5 0.002 raw.mpg david.mpg"
  exit 1
fi

source=${1}
l=${2}
d=${3}
p=${4}
raw=${5}
david=${6}

./FecGenerator source=${source} dest=tmp-gen l=${l} d=${d} vvv    || exit 1
./ErrorsGenerator source=tmp-gen dest=tmp-err p=${p} prob=0  vvv  || exit 1
./FecDecoder source=tmp-err destRaw=${raw} destDavid=${david} vvv || exit 1

rm tmp-gen
rm tmp-err

echo "Quick test finished !"
