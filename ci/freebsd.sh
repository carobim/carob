#!/bin/bash
arch=aarch64
cpu=$(nproc)
mem=1G
#version=14.4
version=15.1

case $1 in
    up) up=1;;
    build) build=1;;
    down) down=1;;
    "") up=1; build=1 down=1;;
esac

cd "${0%/*}"
repo=$PWD

vm_dir=~/.local/share/ci/freebsd-$version-$arch
port=2222

ssh="ssh -p $port -o ConnectTimeout=1 -o LogLevel=ERROR -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null root@127.0.0.1"

cd $vm_dir

if [[ -n $up ]]; then
    echo '### Setting up VM'
    qemu-system-$arch -machine virt,accel=hvf -cpu host -smp $cpu -m $mem -bios /opt/homebrew/share/qemu/edk2-$arch-code.fd -drive file=build.qcow2,format=qcow2,if=virtio,snapshot=on -nic user,model=virtio-net-pci,hostfwd=tcp:127.0.0.1:$port-:22 -display none -daemonize -pidfile qemu.pid

    sleep 0.1
    pid=$(cat qemu.pid)

    if [[ -n $down ]]; then
        trap "kill $pid" EXIT
    fi

    until $ssh true 2>/dev/null; do sleep 0.5; done
else
    pid=$(cat qemu.pid)
fi

if [[ -n $build ]]; then
    (
        set -e

        echo '### Uploading'
        COPYFILE_DISABLE=1 tar -C $repo --no-xattrs -cf - $(git -C $repo ls-tree --name-only HEAD) | $ssh '
            rm -rf /build
            mkdir /build
            tar -xf - -C /build
        '

        echo '### Building'
        $ssh '
            cd /build
            mkdir build
            cd build
            cmake .. -G Ninja -DUNITS=1
            cmake --build .
        '

        echo '### Running unit tests'
        $ssh '
            cd /build
            build/units
        '

        echo '### OK'
    )
fi

if [[ -n $down ]]; then
    echo '### Tearing down VM'
    kill $pid
    trap "" EXIT
    while kill -0 $pid 2>/dev/null; do sleep 1; done
fi
