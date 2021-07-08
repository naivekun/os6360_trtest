1. Build cpss before run buildroot
	./build.sh all
2. copy the rootfs from cpss to buildroot overlay folder
	rsync -aAXv --exclude=/mnt ./cpss-ac3/linux/uni/4.4/kernel/mvebu_v7_lsp_noprestera_defconfig/rootfs/  ./buildroot/buildroot-2015.11-16.08.0/user_files/
3. enter docker container
	./build.sh enter-container
4. enter buildroot dir	
	cd buildroot/buildroot-2015.11-16.08.0
5. make 	
	make os6360_defconfig
	make
6. rootfs would be in buildroot-2015.11-16.08.0/output/images/rootfs.ubifs
