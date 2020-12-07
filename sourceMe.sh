#===========================================================================
# This file performs actions that are required to configure project  
# environment (i.e. downloading dependencies, setting envs, etc.)    
# 
# NOTE : ESP8266_RTOS_SDK building tools use /usr/bin/python which by
#        default points to /usr/bin/python2. To change it to python3 
#        make use of 'update-alternatives'.                          
#===========================================================================

# Home location of the project
PROJECT_HOME="/home/cris/Desktop/Pierczyk_Krzysztof"

# Location of SDK's additional tools (like xtensa-gcc)
IDF_TOOLS_PATH=$PROJECT_HOME/common/idf-tools

# Firefox version downloaded to run Copper
FIREFOX_VERSION="52.9.0esr"

# -------------------------------- Preparation --------------------------------

export PROJECT_HOME
export IDF_TOOLS_PATH

mkdir -p $PROJECT_HOME/common

# Just awesome tool <3 (no more struggling with 'find')
if ! which locate >> /dev/null; then
    sudo apt isntall mlocate
fi

# ------------------------------------ ESP ------------------------------------

# ESP8266 SDK download
if  [[ ! -d common/ESP8266_RTOS_SDK || -z "$(ls -A $PROJECT_HOME/common/ESP8266_RTOS_SDK)" ]]; then
    git submodule update --init --recursive
fi

# Configure ESP-IDF tools and exports
mkdir -p $IDF_TOOLS_PATH
cd common/ESP8266_RTOS_SDK
./install.sh
source ./export.sh
source ./add_path.sh

# Export additional tools for OTA updates
export PATH=$PATH:$IDF_PATH/components/app_update

cd $PROJECT_HOME

# ---------------------------------- Copper -----------------------------------

# Check if firexof 55.0.3 is installed
if [[ ! -d $PROJECT_HOME/common/firefox ]]; then

    wget "https://ftp.mozilla.org/pub/firefox/releases/$FIREFOX_VERSION/linux-x86_64/en-GB/firefox-$FIREFOX_VERSION.tar.bz2"
    tar -xjf firefox-$FIREFOX_VERSION.tar.bz2 -C $PROJECT_HOME/common

    # If mozilla was installed yet, the ~/.mozilla folder exist in unknown state
    # We decide to delete it from the system and create one more time
    if [[ -d $HOME/.mozilla ]]; then
        rm -r $HOME/.mozilla
    fi

    # We need to run firefox for a while to create the folder
    echo "Please wait for firefox to open and close it"
    echo ""
    $PROJECT_HOME/common/firefox/firefox > /dev/null &
    wait $!

    # Then, we need to reinstall firefox in case it aut-updated in the first run
    rm -r $PROJECT_HOME/common/firefox
    tar -xjf firefox-$FIREFOX_VERSION.tar.bz2 -C $PROJECT_HOME/common
    rm firefox-$FIREFOX_VERSION.tar.bz2

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
    if grep -q "$setting" $DEFAULT/prefs.js > /dev/null; then
        sed -i "s/$setting/${setting/true/false}/g" $DEFAULT/prefs.js
    else
        echo "${setting/true/false}" >> $DEFAULT/prefs.js
    fi
done

# Check if Copper addon is downloaded
if [[ ! -d $PROJECT_HOME/common/Copper || -z "$(ls -A $PROJECT_HOME/common/Copper)" ]]; then
    git submodule update --init --recursive
fi

# Set path to the Copper extension in the firefox
EXTENSION_FILE=$DEFAULT/extensions/copper@vs.inf.ethz.ch
mkdir -p $DEFAULT/extensions
echo "$PROJECT_HOME/common/Copper/" > $EXTENSION_FILE
# Allow unsigned add-ons
SIGNATURE_SETTING='user_pref("xpinstall.signatures.required", true);'
if grep -q "$SIGNATURE_SETTING" $DEFAULT/prefs.js > /dev/null; then
    sed -i "s/$SIGNATURE_SETTING/${SIGNATURE_SETTING/true/false}/g" $DEFAULT/prefs.js
else
    echo "${SIGNATURE_SETTING/true/false}" >> $DEFAULT/prefs.js
fi

# -------------------------------- Wireshark ----------------------------------

# Install Wireshark
if ! which wireshark > /dev/null; then
    echo "LOG: Wireshark will be installed..."
    sudo apt install wireshark
    sudo apt isntall libcap-dev
fi

# Install TShark
if ! which tshark > /dev/null; then
    echo "LOG: TShark will be installed..."
    sudo apt install tshark
    sudo apt isntall libcap-dev
fi
