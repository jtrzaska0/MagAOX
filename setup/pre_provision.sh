#!/bin/bash
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
set -euo pipefail
source $DIR/_common.sh
if [[ "$EUID" == 0 ]]; then
    log_error "Can't add you to the magaox-dev group when you're running as root! Aborting."
    exit 1
fi

source /etc/os-release
# without hardened_usercopy=off, the ALPAO DM driver (really the Interface Corp card driver) will
# trigger protections against suspicious copying between kernel and userspace
# and *bring down the whole system* (by rebooting when you try to run
# https://github.com/magao-x/ALPAO-interface/blob/master/initalpaoPCIe )
ALPAO_CMDLINE_FIX="hardened_usercopy=off"
# make the PCIe expansion card work
PCIEXPANSION_CMDLINE_FIX="pci=noaer"
# disable the slow Spectre mitigations
SPECTRE_CMDLINE_FIX="noibrs noibpb nopti nospectre_v2 nospectre_v1 l1tf=off nospec_store_bypass_disable no_stf_barrier mds=off mitigations=off"
# disable 3rd party nvidia drivers
NVIDIA_DRIVER_FIX="rd.driver.blacklist=nouveau nouveau.modeset=0"
# Put it all together
DESIRED_CMDLINE="nosplash $NVIDIA_DRIVER_FIX $ALPAO_CMDLINE_FIX $PCIEXPANSION_CMDLINE_FIX $SPECTRE_CMDLINE_FIX"
if [[ $ID == ubuntu ]]; then
    log_info "Skipping RT kernel install on Ubuntu"
    install_rt=false
else
    if ! grep "$DESIRED_CMDLINE" /etc/default/grub; then
        echo GRUB_CMDLINE_LINUX_DEFAULT=\""$DESIRED_CMDLINE"\" | sudo tee -a /etc/default/grub
        sudo grub2-mkconfig -o /boot/grub2/grub.cfg
        log_success "Applied kernel command line tweaks for ALPAO, Spectre, PCIe expansion"
    fi
    if [[ $(uname -v) != *"PREEMPT RT"* ]]; then
        log_info "Will install RT kernel..."
        install_rt=true
    fi
fi

if [[ ! -e /etc/modprobe.d/blacklist-nouveau.conf ]]; then
    echo "blacklist nouveau" | sudo tee /etc/modprobe.d/blacklist-nouveau.conf > /dev/null
    echo "options nouveau modeset=0" | sudo tee -a /etc/modprobe.d/blacklist-nouveau.conf > /dev/null
    log_success "Blacklisted nouveau nvidia driver"
else
    log_info "nouveau nvidia driver blacklist entry exists"
fi

$DIR/setup_users_and_groups.sh
log_success "Created users and configured groups"

if [[ $install_rt == true ]]; then
    sudo $DIR/steps/install_rt_kernel.sh
    log_success "Installed RT kernel"
fi

log_success "Reboot before proceeding"
