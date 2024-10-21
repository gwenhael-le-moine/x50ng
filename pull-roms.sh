CWD=$(pwd)
TEMPDIR=${TEMPDIR:-$CWD/tmpdir}
FWDIR=${FWDIR:-$CWD/firmware}

mkdir -p "$FWDIR"
mkdir -p "$TEMPDIR"
cd "$TEMPDIR" || exit 1

echo Retrieving Bootloader from hpcalc.org for HP 49g+ and HP 50g...
wget https://www.hpcalc.org/hp49/pc/rom/hp-arm-models-bootloader.zip
unzip -j hp-arm-models-bootloader.zip -d "$TEMPDIR"
mv ./*.bin "$FWDIR"/

echo Retrieving ROM 2.15 from hpcalc.org for HP 49g+ and HP 50g...
wget https://www.hpcalc.org/hp49/pc/rom/hp4950v215.zip
unzip -j hp4950v215.zip -d "$TEMPDIR"
mv ./*.bin "$FWDIR"/

echo Retrieving latest Unofficial newRPL build...
wget https://hpgcc3.org/downloads/newrplfw.bin
mv ./newrplfw.bin "$FWDIR"/

cd "$CWD" || exit 1
rm -rf "$TEMPDIR"
