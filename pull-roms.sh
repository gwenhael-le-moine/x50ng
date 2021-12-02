TEMPDIR=${TEMPDIR:-$(pwd)/tmpdir}
FWDIR=${FWDIR:-$(pwd)/firmware}

mkdir -p $FWDIR

echo Retrieving Bootloader from hpcalc.org for HP 49g+ and HP 50g...
mkdir -p $TEMPDIR
pushd $TEMPDIR
wget https://www.hpcalc.org/hp49/pc/rom/hp-arm-models-bootloader.zip
unzip -j hp-arm-models-bootloader.zip -d $TEMPDIR
cp *.bin $FWDIR/
popd
rm -rf $TEMPDIR


echo Retrieving ROM 2.15 from hpcalc.org for HP 49g+ and HP 50g...
mkdir -p $TEMPDIR
pushd $TEMPDIR
wget https://www.hpcalc.org/hp49/pc/rom/hp4950v215.zip
unzip -j hp4950v215.zip -d $TEMPDIR
cp *.bin $FWDIR/
popd
rm -rf $TEMPDIR

echo Retrieving latest Unofficial newRPL build...
mkdir -p $TEMPDIR
pushd $TEMPDIR
wget https://hpgcc3.org/downloads/newrplfw.bin
cp newrplfw.bin $FWDIR/
popd
rm -rf $TEMPDIR
