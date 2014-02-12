CFLAGS="-m32" LDFLAGS=-m32 make xen XEN_TARGET_ARCH=x86_32 -j4
objcopy -O binary xen/xen xen.bin
cp xen.bin ~/src/preos_dv10-0.3.4-devel/
cp xen.bin ~/src/preos_pr3.1-0.3.4-devel/
