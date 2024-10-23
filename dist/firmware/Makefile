.PHONY: get-roms mrproper

all: bootloaders official newrpl

mrproper:
	rm -fr ./Readme.txt ./boot*.bin ./hp4950v215 ./newrpl

bootloaders:
	wget https://www.hpcalc.org/hp49/pc/rom/hp-arm-models-bootloader.zip
	unzip -j hp-arm-models-bootloader.zip
	rm -f hp-arm-models-bootloader.zip

official:
	wget https://www.hpcalc.org/hp49/pc/rom/hp4950v215.zip
	unzip -j hp4950v215.zip -d hp4950v215
	rm -f hp4950v215.zip
	unzip -j hp4950v215/2MB\ FIX.zip -d hp4950v215/2MB_FIX
	rm -f hp4950v215/2MB\ FIX.zip

newrpl:
	mkdir -p newrpl-latest
	wget https://hpgcc3.org/downloads/newrplfw.bin -O newrpl-latest/newrplfw.bin
	echo "newrplfw.bin" > newrpl-latest/update.scp
