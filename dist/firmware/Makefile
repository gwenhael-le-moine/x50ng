.PHONY: get-roms mrproper

all: bootloaders official newrpl

mrproper:
	rm -f ./hp-arm-models-bootloader ./hp4950v215 ./newrpl

bootloaders:
	wget https://www.hpcalc.org/hp49/pc/rom/hp-arm-models-bootloader.zip
	unzip -j hp-arm-models-bootloader.zip -d hp-arm-models-bootloader
	rm hp-arm-models-bootloader.zip

official:
	wget https://www.hpcalc.org/hp49/pc/rom/hp4950v215.zip
	unzip -j hp4950v215.zip -d hp4950v215
	rm hp4950v215.zip

newrpl:
	mkdir -p newrpl
	wget https://hpgcc3.org/downloads/newrplfw.bin -O newrpl/newrplfw.bin
	echo "newrplfw.bin" > newrpl/update.scp
