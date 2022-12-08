# Cross-compilation for the Raspberry PI 4 (aarch64)  
  * Based on the VSCode docker compose on Ubuntu 20.04 LTS  

## KrakenSDR
  * Build with the SoapySDR library

## Environment
Currently supported:
  * Ubuntu Linux 20.04 with GCC  10.2.1

## Getting Started
  * Gets ip address of the target:
  sudo arp-scan -l --interface=enp3s0
  * Creates root file system for the cross-compile:
  rsync -vR --progress -rl --delete-after --safe-links user@ip_adress:/{lib,usr,etc/ld.so.conf.d,opt/vc/lib} $RASPBIAN_ROOTFS
  * Run Docker Compose from VSCode


## Additional References
  * [Cross-compilation Guide](https://tttapa.github.io/Pages/Raspberry-Pi/C++-Development-RPiOS/)  
