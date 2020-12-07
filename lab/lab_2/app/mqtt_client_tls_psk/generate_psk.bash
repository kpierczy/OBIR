#!/usr/bin/env bash

# ============================================================================================================
#  File: generate_psk.bash
#  Author: Krzysztof Pierczyk
#  Modified time: 2020-12-07 17:16:02
#  Description:
#
#      Creates a PSK files for both Mosquitto broker and ESP app. Configures Mosquitto
#      to listen on 8883 port with TLS 1.2 protocol.
#
#  Note: Script should be run with 'sudo -E'
# ============================================================================================================

LAB_HOME=$PROJECT_HOME/lab/lab_2/app/mqtt_client_tls_psk

# Broker's PORT
SERVER_PORT=1883
SERVER_TLS_PORT=8883

# Folders that result files will be stored into
TARGET_FOLDER=$LAB_HOME/data
KEY_NAME=psk.key


# --------------------------------------------------------------------------------------------------------------- #
# ==================================================== Code ===================================================== #
# --------------------------------------------------------------------------------------------------------------- #

mkdir -p $TARGET_FOLDER

# --------------------------------------- Certificate authority's creation -------------------------------------- #

# Generate the PSK
PSK=$(openssl rand -hex 32)

# Generate the binary form for the ESP8266 app
echo $PSK > $TARGET_FOLDER/$KEY_NAME

# Change ownership over the created file from root to the caller
chown -R $SUDO_USER $TARGET_FOLDER
chgrp -R $SUDO_USER $TARGET_FOLDER

# ---------------------------------------------- Configure Mosquitto -------------------------------------------- #

# Write created PSK to the Mosquitto's folder
mkdir -p /etc/mosquitto/psk
echo "esp8266: $PSK" > /etc/mosquitto/psk/$KEY_NAME


# Edit the mosquitto configuration to use the PSK
echo "# Configuration for the ESP8266 projects          "  > /etc/mosquitto/conf.d/default.conf
echo "port $SERVER_PORT                                 " >> /etc/mosquitto/conf.d/default.conf
echo "                                                  " >> /etc/mosquitto/conf.d/default.conf
echo "# TLS configuration                               " >> /etc/mosquitto/conf.d/default.conf
echo "listener $SERVER_TLS_PORT                         " >> /etc/mosquitto/conf.d/default.conf
echo "psk_hint cris-broker                              " >> /etc/mosquitto/conf.d/default.conf
echo "psk_file /etc/mosquitto/psk/$KEY_NAME             " >> /etc/mosquitto/conf.d/default.conf
echo "use_identity_as_username true                     " >> /etc/mosquitto/conf.d/default.conf
echo "tls_version tlsv1.2                               " >> /etc/mosquitto/conf.d/default.conf

# Restart mosquitto
systemctl restart mosquitto