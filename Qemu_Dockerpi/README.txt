# Reference 
https://github.com/lukechilds/dockerpi

# Run these instruction
docker pull lukechilds/dockerpi
docker run -it lukechilds/dockerpi pi2

# Loggin
user:pi
password:raspberry

# Execution
Execute your cross-compiled binary just like executing it on E9V3

# Why it works
E9V3 (Cortex-A9) cross-compiled binary runs on QEMU emulating a Raspberry Pi 2 (Cortex-A7) because both CPUs share the ARMv7-A instruction set, making them binary compatible. Additionally, both support VFPv3-D16 for floating-point operations, and your binary likely uses EABI5 with armhf (hard float ABI), which Raspberry Pi 2 fully supports. QEMU ensures a complete ARMv7 hardware and software environment, handling minor architectural differences, so the binary runs without modification.
