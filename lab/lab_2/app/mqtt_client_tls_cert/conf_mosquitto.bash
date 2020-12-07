#!/usr/bin/env bash

# ============================================================================================================
#  File: conf_mosquitto.bash
#  Author: Krzysztof Pierczyk
#  Modified time: 2020-12-07 17:16:02
#  Description:
#
#      1. Creates a temporary Certificate Authority's (CA) keys pair and certificate
#      2. Creates a Mosquitto broker's keys pair
#      3. Creates broker's certificate request and signs it with the temporary CA's
#         certificate
#      4. Configures Mosquittto to use created files
#
#  Note: Script should be run with 'sudo -E'
# ============================================================================================================

LAB_HOME=$PROJECT_HOME/lab/lab_2/app/mqtt_client_tls_cert

# Broker's PORT
SERVER_PORT=1883
SERVER_TLS_PORT=8883

# Folders that result files will be stored into
TARGET_FOLDER=$LAB_HOME/data
KEYS_FOLDER=$TARGET_FOLDER/keys
CERT_FOLDER=$TARGET_FOLDER/cert

# CA subject's info
CA_NAME=ca
PASSWORD=passwd
CA_SUBJ="/C=PL"

# Broker subject's info
SERVER_NAME=server
SERVER_SUBJ="/C=PL/CN=192.168.0.94"


# --------------------------------------------------------------------------------------------------------------- #
# ==================================================== Code ===================================================== #
# --------------------------------------------------------------------------------------------------------------- #

mkdir -p $KEYS_FOLDER
mkdir -p $CERT_FOLDER

# --------------------------------------- Certificate authority's creation -------------------------------------- #

echo "< ===================== Creating CA's keys pair ===================== >"
# Create a private-public keys pair for the temporary CA
openssl genrsa                     \
    -passout=pass:$PASSWORD        \
    -out $KEYS_FOLDER/$CA_NAME.key \
    -des3                          \
    2048

echo "< ==================== Creating CA's certificate ==================== >"
# Create a certificate for the temporary CA
openssl req                        \
    -new                           \
    -x509                          \
    -days 1826                     \
    -key $KEYS_FOLDER/$CA_NAME.key \
    -out $CERT_FOLDER/$CA_NAME.crt \
    -passin=pass:$PASSWORD         \
    -subj $CA_SUBJ


# ----------------------------------------- Broker certificate's creation --------------------------------------- #

echo "< =================== Creating broker's keys pair =================== >"
# Create a private-public keys pair for the mosquitto server
openssl genrsa                         \
    -out $KEYS_FOLDER/$SERVER_NAME.key \
    2048

echo "< ============== Creating broker's certificate request ============== >"
# Create a certificate request for the mosquitto server
openssl req                              \
    -new                                 \
    -key $KEYS_FOLDER/$SERVER_NAME.key   \
    -out $TARGET_FOLDER/$SERVER_NAME.csr \
    -subj $SERVER_SUBJ

echo "< ============= Verify and sign the broker's certificate ============ >"
# Verify and sign the certificate request using temporary CA's certificate
openssl x509                            \
    -req                                \
    -in $TARGET_FOLDER/$SERVER_NAME.csr \
    -CA $CERT_FOLDER/$CA_NAME.crt       \
    -CAkey $KEYS_FOLDER/$CA_NAME.key    \
    -passin=pass:$PASSWORD              \
    -CAcreateserial                     \
    -out $CERT_FOLDER/$SERVER_NAME.crt  \
    -days 360

# Remove the broker's request files
rm $TARGET_FOLDER/$SERVER_NAME.csr
rm $CERT_FOLDER/$CA_NAME.srl

# Change ownership over the created files from root to the caller
chown -R $SUDO_USER $TARGET_FOLDER
chgrp -R $SUDO_USER $TARGET_FOLDER

# ---------------------------------------------- Configure Mosquitto -------------------------------------------- #

# Copy create files to the Mosquitto's folder
cp $CERT_FOLDER/$CA_NAME.crt /etc/mosquitto/ca_certificates/
cp $CERT_FOLDER/$SERVER_NAME.crt /etc/mosquitto/certs/
cp $KEYS_FOLDER/$SERVER_NAME.key /etc/mosquitto/certs/

# Edit the mosquitto configuration to use thos files
echo "# Configuration for the ESP8266 projects          "  > /etc/mosquitto/conf.d/default.conf
echo "port $SERVER_PORT                                 " >> /etc/mosquitto/conf.d/default.conf
echo "                                                  " >> /etc/mosquitto/conf.d/default.conf
echo "# TLS configuration                               " >> /etc/mosquitto/conf.d/default.conf
echo "listener $SERVER_TLS_PORT                         " >> /etc/mosquitto/conf.d/default.conf
echo "cafile /etc/mosquitto/ca_certificates/$CA_NAME.crt" >> /etc/mosquitto/conf.d/default.conf
echo "keyfile /etc/mosquitto/certs/$SERVER_NAME.key     " >> /etc/mosquitto/conf.d/default.conf
echo "certfile /etc/mosquitto/certs/$SERVER_NAME.crt    " >> /etc/mosquitto/conf.d/default.conf
echo "tls_version tlsv1.2                               " >> /etc/mosquitto/conf.d/default.conf

# Restart mosquitto
systemctl restart mosquitto