#!/bin/bash

VMDK_PATH=/home/henning/data/virtualbox/wisebed/wisebed.vmdk
MOUNTPOINT_VMDK=/home/henning/mnt/tmp
MOUNTPOINT_PARTITION=/home/henning/mnt/wisebed
HOME_PATH=/home/henning

VDFUSE="vdfuse -t VMDK -f"
PARTITION_PATH=$MOUNTPOINT_VMDK/Partition1
PARTITION_FS=ext4

if [ "$(id -u)" != "0" ]; then
	echo this must be run by root.
	exit 1
fi


case $1 in
	mount)
		echo -------- WARNING! ---------
		echo This script can very easily damage the state of your virtual
		echo machine to uselessness.
		echo Use it *only* when your VM has been propely shut down,
		echo "that is, freezing it ('save state') is NOT ENOUGH!"
		echo
		echo Even then use this as your OWN RISK! Dont work without backups!
		echo ---------------------------
		echo
		echo Press return if you still want to go on, ctrl-c otherwise.
		read
		
		echo $VDFUSE $VMDK_PATH $MOUNTPOINT_VMDK
		$VDFUSE $VMDK_PATH $MOUNTPOINT_VMDK
		echo mount -t $PARTITION_FS -o loop $PARTITION_PATH $MOUNTPOINT_PARTITION
		mount -o loop $PARTITION_PATH $MOUNTPOINT_PARTITION
		echo mount --bind /dev $MOUNTPOINT_PARTITION/dev
		mount --bind /dev $MOUNTPOINT_PARTITION/dev
		echo mount --bind /dev/pts $MOUNTPOINT_PARTITION/dev/pts
		mount --bind /dev/pts $MOUNTPOINT_PARTITION/dev/pts
		#echo mount $HOME_PATH $MOUNTPOINT_PARTITION/home/wiselib/host
		#mount --bind $HOME_PATH $MOUNTPOINT_PARTITION/home/wiselib/host
		echo mount --bind /proc $MOUNTPOINT_PARTITION/proc
		mount --bind /proc $MOUNTPOINT_PARTITION/proc
		;;
	umount)
		#umount $MOUNTPOINT_PARTITION/home/wiselib/host
		umount $MOUNTPOINT_PARTITION/dev/pts
		umount $MOUNTPOINT_PARTITION/dev
		umount $MOUNTPOINT_PARTITION/proc
		umount $MOUNTPOINT_PARTITION
		umount $MOUNTPOINT_VMDK
		;;
	fsck)
		shift
		echo $VDFUSE $VMDK_PATH $MOUNTPOINT_VMDK
		$VDFUSE $VMDK_PATH $MOUNTPOINT_VMDK
		echo fsck.$PARTITION_FS $@  $PARTITION_PATH
		fsck.$PARTITION_FS $@ $PARTITION_PATH
		echo umount $VMDK_PATH
		umount $VMDK_PATH
		;;
		
	chroot)
		chroot $MOUNTPOINT_PARTITION
		;;
esac

