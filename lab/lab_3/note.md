# Installation of the VM VirtualBox Guest Additions

1. Start guest machine
2. Click Devices|Insert Guest Additions CD Image (Download if needed)
5. Mount the CD-ROM with the command `sudo mount /dev/cdrom /media/cdrom`.
6. Change into the mounted directory with the command `cd /media/cdrom`.
7. Install the necessary dependencies with the command `sudo apt install -y dkms build-essential linux-headers-$(uname -r)`.
8. Change to the root user with the command `sudo su`.
9. Install the Guest Additions package with the command `./VBoxLinuxAdditions.run`.
10. Allow the installation to complete


# Handy links

1. VBoxManage documentation : [https://www.virtualbox.org/manual/ch08.html]
