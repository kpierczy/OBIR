#===========================================================================
# This file performs actions that are required to configure project 
# environment (i.e. downloading dependencies, setting envs, etc.)
#
# NOTE : ESP8266_RTOS_SDK building tools use /usr/bin/python which by
#        default points to /usr/bin/python2. To change it to python3
#        make use of 'update-alternatives'.
#===========================================================================

# ESP8266 SDK download
if [[ ! -e common/ESP8266_RTOS_SDK ]]; then
    git submodule update --recursive
fi
# ESP8266 Toolchain download [5.2.0]
if [[ ! -e common/xtensa-lx106-elf ]]; then
    wget "https://dl.espressif.com/dl/xtensa-lx106-elf-linux64-1.22.0-100-ge567ec7-5.2.0.tar.gz"
    tar -xzf xtensa-lx106-elf-linux64-1.22.0-100-ge567ec7-5.2.0.tar.gz
    mv xtensa-lx106-elf common/ 
    rm xtensa-lx106-elf-linux64-1.22.0-100-ge567ec7-5.2.0.tar.gz
fi

# ESP8266 exports
export     PATH=$(readlink -f common/xtensa-lx106-elf/bin):$PATH
export IDF_PATH=$(readlink -f common/ESP8266_RTOS_SDK/)