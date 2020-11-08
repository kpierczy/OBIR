#===========================================================================
# This file performs actions that are required to configure project 
# environment (i.e. downloading dependencies, setting envs, etc.)
#
# NOTE : ESP8266_RTOS_SDK building tools use /usr/bin/python which by
#        default points to /usr/bin/python2. To change it to python3
#        make use of 'update-alternatives'.
#===========================================================================

PROJECT_HOME="/home/cris/Desktop/Pierczyk_Krzysztof"

# ----------------------------------- ESP ----------------------------------

# ESP8266 SDK download
if [[ ! -d common/ESP8266_RTOS_SDK ]]; then
    git submodule update --init --recursive
fi
# ESP8266 Toolchain download [5.2.0]
if [[ ! -d common/xtensa-lx106-elf ]]; then
    wget "https://dl.espressif.com/dl/xtensa-lx106-elf-gcc8_4_0-esp-2020r3-linux-amd64.tar.gz"
    tar -xzf xtensa-lx106-elf-gcc8_4_0-esp-2020r3-linux-amd64.tar.gz -C $PROJECT_HOME/common
    rm xtensa-lx106-elf-gcc8_4_0-esp-2020r3-linux-amd64.tar.gz
fi

# ESP8266 exports
export     PATH=$PROJECT_HOME/common/xtensa-lx106-elf/bin:$PATH
export IDF_PATH=$PROJECT_HOME/common/ESP8266_RTOS_SDK/

# --------------------------------- Copper ---------------------------------

# Check if firexof 55.0.3 is installed
if [[ ! -d $PROJECT_HOME/common/firefox ]]; then

    wget "https://ftp.mozilla.org/pub/firefox/releases/55.0.3/linux-x86_64/en-GB/firefox-55.0.3.tar.bz2"
    tar -xjf firefox-55.0.3.tar.bz2 -C $PROJECT_HOME/common

    # If mozilla was not installed yet, the ~/.mozilla folder does not exist
    if [[ ! -d $HOME/.mozilla ]]; then
        # We need to run firefox for a while to create the folder
        echo "Please wait for firefox to open and close it"
        $PROJECT_HOME/common/firefox/firefox > /dev/null &
        wait $!
        # Then, we need to reinstall firefox in case it aut-updated in the first run
        rm -r $PROJECT_HOME/common/firefox
        tar -xjf firefox-55.0.3.tar.bz2 -C $PROJECT_HOME/common
    
    fi

    rm firefox-55.0.3.tar.bz2
    
fi

# Modify firefox settings if nessesary
DEFAULT=$(find $HOME/.mozilla/firefox/ -name '*.default')
SETTINGS=(
    'user_pref("browser.tabs.remote.autostart.2", true);'
    'user_pref("browser.tabs.remote.autostart", true);'  
    'user_pref("app.update.auto", true);'                
    'user_pref("app.update.enabled", true);'
)

for setting in ${SETTINGS[@]}; do
    if grep -q $setting $DEFAULT/prefs.js > /dev/null; then
        sed -i "s/$setting/${setting/true/false}/g" $DEFAULT/prefs.js
    else
        echo "${setting/true/false}" >> $DEFAULT/prefs.js
    fi
done

# Check if Copper addon is downloaded
if [[ ! -d $PROJECT_HOME/common/Copper ]]; then
    git submodule update --init --recursive
fi

# Set path to the Copper extension in the firefox
if [[ ! -e $DEFAULT/extensions/copper@vs.inf.ethz.ch ]]; then
    echo "$PROJECT_HOME/common/Copper/" > $DEFAULT/extensions/copper@vs.inf.ethz.ch
fi
# Allow unsigned add-ons
SIGNATURE_SETTING='user_pref("xpinstall.signatures.required", true);'
if grep -q $SIGNATURE_SETTING $DEFAULT/prefs.js > /dev/null; then
    sed -i "s/$SIGNATURE_SETTING/${SIGNATURE_SETTING/true/false}/g" $DEFAULT/prefs.js
else
    echo "${SIGNATURE_SETTING/true/false}" >> $DEFAULT/prefs.js
fi