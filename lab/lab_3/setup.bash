#=================================================================================================================
# @ Author: Krzysztof Pierczyk
# @ Create Time: 2020-12-21 21:28:08
# @ Modified time: 2020-12-25 23:10:34
# @ Description:
#
#      Script for autoinstallation and uninstallation tools required for the lab.
#
#=================================================================================================================

LAB_HOME="/home/cris/Desktop/Pierczyk_Krzysztof/lab/lab_3"

# Virtual machine's folder=
VM_FOLDER="$LAB_HOME/obirvm"

# Name of the imported virtual machine
VM_NAME="obirvm"

# Virtual machine MAC address
VM_MAC="080027bb8047"

# Folder shared (to not share a folder put "" into VM_SHARED_MOUNT)
VM_SHARED="$LAB_HOME/shared"
VM_SHARED_MOUNT=""

# ----------------------------------------------------------------------------------------------------------------

cd $LAB_HOME

# Environment preparations
if [[ $1 == "" ]]; then

    # Get virtual box
    if ! which virtualbox > /dev/null; then
        sudo apt update
        sudo apt install virtualbox virtualbox-ext-pack
    fi

    # Download the system image
    if [[ ! -f "$VM_FOLDER/$VM_NAME.ova" ]]; then
        curl -o $VM_FOLDER/$VM_NAME.ova -u obirvm:obir20vm20 https://secure.tele.pw.edu.pl/~apruszko/obir/obir_20201124.ova
    fi

    # Prepare the virtual machine
    if ! vboxmanage list vms | grep $VM_NAME > /dev/null; then

        # Import the image
        vboxmanage import $VM_FOLDER/$VM_NAME.ova                \
            --vsys 0 --vmname $VM_NAME                           \
            --vsys 0 --group ""                                  \
            --vsys 0 --settingsfile "$VM_FOLDER/$VM_NAME.vbox"   \
            --vsys 0 --basefolder "$VM_FOLDER"                   \
            --vsys 0 --unit 13 --disk "$VM_FOLDER/$VM_NAME.vmdk" \
            --vsys 0 --description ""                            \
            --vsys 0 --memory 2048                               \
            --vsys 0 --cpus 2

        # Configure machine
        vboxmanage modifyvm $VM_NAME \
            --vram 128               \
            --nic1 nat               \
            --natnet1 default        \
            --macaddress1 $VM_MAC
            # --natpf1 "nodered,tcp,,1880,,1880" \
            # --natpf1 "ssh,tcp,,2222,,22"       \
            # --natpf1 "www,tcp,,80,,80"         \

        # Configure shared folder
        if [[ $VM_SHARED_MOUNT != "" ]]; then
            vboxmanage sharedfolder add $VM_NAME    \
                --name "shared"                     \
                --hostpath $VM_SHARED               \
                --automount                         \
                --auto-mount-point $VM_SHARED_MOUNT

            printf "\n\nLOG: Please run the virtual machine and install Oracle VM VirtualBox Guest Additions\n\n\n"
        fi
    fi

# Running Virtual machine and Node-Red
elif [[ $1 == "run" ]]; then

    # Virtualmachine
    vboxmanage startvm $VM_NAME

    # Red-Node center
    google-chrome 127.0.0.1:1880

# Virtual machine cleanup
elif [[ $1 == "rm" ]]; then

    # Delete the image from Virtual-Box
    if vboxmanage list vms | grep $VM_NAME > /dev/null; then
        vboxmanage unregistervm --delete $VM_NAME
    fi

# Environment cleaning
elif [[ $1 == "clean" ]]; then

    # Delete the image from Virtual-Box
    if vboxmanage list vms | grep $VM_NAME > /dev/null; then
        vboxmanage unregistervm --delete $VM_NAME
    fi

    # Remove system image
    if [[ -f "$VM_FOLDER/$VM_NAME.ova" ]]; then
        rm $VM_FOLDER/$VM_NAME.ova.ova
    fi

    # Remove virtual box
    if which virtualbox > /dev/null; then
        sudo apt remove virtualbox virtualbox-ext-pack
        sudo apt autoremove
    fi

fi