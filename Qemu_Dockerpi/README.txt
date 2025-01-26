## Turn on Raspberry pi2 ##
# Run these instruction
docker pull lukechilds/dockerpi
docker run -it -v $HOME/.dockerpi:/sdcard lukechilds/dockerpi pi2

# Loggin
user:pi
password:raspberry

# Execution
Execute your cross-compiled binary just like executing it on E9V3

# Turn it off (use ps to get container id)
docker ps
docker kill (container id)

# Reference 
https://github.com/lukechilds/dockerpi


## Expand the size of Raspberry pi storage to 2^n Gb ##
# Do it on the host linux
sudo apt install -y qemu-system-arm qemu-utils
qemu-img resize ~/.dockerpi/filesystem.img 16G
sudo apt install -y parted
sudo modprobe nbd max_part=10
sudo qemu-nbd -c /dev/nbd0 ~/.dockerpi/filesystem.img
sudo parted /dev/nbd0 --script resizepart 2 100%
sudo qemu-nbd -d /dev/nbd0
# Turn it on again

## Mount the root of Raspberry pi2 on your host linux so you can copy file directly ##
# The file may not sync immediately so you may need to umount and remount it
sudo mkdir -p ~/.dockerpi/mnt
LOOP_DEVICE=$(sudo losetup -f --show -P ~/.dockerpi/filesystem.img)
sudo mount "${LOOP_DEVICE}p2" ~/.dockerpi/mnt

## Unmount it ##
sudo umount ~/.dockerpi/mnt
sudo losetup -d $LOOP_DEVICE

# Reference
https://ivonblog.com/posts/emulate-raspberry-pi-os-on-x86-linux/


## Why it works ##
E9V3 (Cortex-A9) cross-compiled binary runs on QEMU emulating a Raspberry Pi 2 (Cortex-A7) because both CPUs share the ARMv7-A instruction set, making them binary compatible. Additionally, both support VFPv3-D16 for floating-point operations, and your binary likely uses EABI5 with armhf (hard float ABI), which Raspberry Pi 2 fully supports. QEMU ensures a complete ARMv7 hardware and software environment, handling minor architectural differences, so the binary runs without modification.
