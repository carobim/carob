#!/bin/bash
set -e

arch=aarch64
#version=14.4
version=15.1

vm_dir=~/.local/share/ci/freebsd-$version-$arch

mkdir -p $vm_dir
cd $vm_dir

if [[ ! -e base.qcow2 ]]; then
    case $arch in
        aarch) arch2=arm64;;
        *) echo >&2 'Unknown arch'; exit 1;;
    esac
    echo '### Downloading base image'
    curl -f https://download.freebsd.org/releases/VM-IMAGES/$version-RELEASE/$arch/Latest/FreeBSD-$version-RELEASE-$arch2-$arch-BASIC-CLOUDINIT-ufs.qcow2.xz | xz -d > base.qcow2

    echo '### Compressing base image'
    qemu-img convert -O qcow2 -c -o compression_type=zstd base.qcow2 zst.qcow2
    mv zst.qcow2 base.qcow2
fi

echo '### Creating configuration image'
mkdir conf
cat >conf/meta-data <<EOF
instance-id: freebsd
local-hostname: freebsd
EOF
cat >conf/user-data <<EOF
#cloud-config
write_files:
  - path: /etc/ssh/sshd_config
    append: true
    content: |
      PermitEmptyPasswords yes
      PermitRootLogin yes

packages:
  - cmake
  - llvm
  - ninja

runcmd:
  - pw usermod root -w none
  - service sshd restart
EOF
rm -f conf.iso
hdiutil makehybrid -iso -joliet -default-volume-name cidata -o conf.iso conf >/dev/null
rm -r conf

echo '### Setting up VM'
qemu-img create -F qcow2 -b base.qcow2 -f qcow2 build.qcow2 10G >/dev/null

qemu-system-$arch -machine virt,accel=hvf -cpu host -smp $(nproc) -m 1G -bios /opt/homebrew/share/qemu/edk2-$arch-code.fd -drive file=build.qcow2,format=qcow2,if=virtio -drive file=conf.iso,format=raw,media=cdrom,readonly=on -nic user,model=virtio-net-pci,hostfwd=tcp:127.0.0.1:$port-:22 -display none -daemonize -pidfile qemu.pid

sleep 0.1
pid=$(cat qemu.pid)
trap "kill $pid" EXIT

ssh='ssh -p 2222 -o ConnectTimeout=1 -o LogLevel=ERROR -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null root@127.0.0.1'

echo '### Waiting on VM'
until $ssh 2>/dev/null test ! -f /var/run/ready; do sleep 1; done

echo '### Tearing down VM'
$ssh shutdown -p now &>/dev/null
trap "" EXIT
while kill -0 $pid 2>/dev/null; do sleep 1; done
rm conf.iso

echo '### Compressing build image'
qemu-img convert -O qcow2 -c -o compression_type=zstd build.qcow2 zst.qcow2
mv zst.qcow2 build.qcow2

echo '### OK'
