# ================================================================================================================
# @ Author: Krzysztof Pierczyk
# @ Create Time: 2021-01-27 13:45:18
# @ Modified time: 2021-01-27 13:45:18
# @ Description:
#
#     Image of the virtual environment used along with the project's workflow.    
# 
# ================================================================================================================

FROM ubuntu:focal

# Create project directory
WORKDIR /home/project

# Prepare environment
ENV PROJECT_HOME   /home/project
ENV IDF_TOOLS_PATH /home/common/idf-tools

# Install sudo
RUN /bin/bash -c "apt-get update && apt-get -y install sudo"

# ----------------------------------------- ESP8266_RTOS_SDK dependancies ----------------------------------------

# Install git
RUN /bin/bash -c "sudo apt install -y git"

# Install python
RUN /bin/bash -c "sudo apt -y install python3"
RUN /bin/bash -c "sudo apt -y install python3-pip"

# Set alternatives to 'python' command (SDK requires python3)
RUN /bin/bash -c "sudo update-alternatives --install /usr/bin/python python /usr/bin/python3.8 3"

# Install tzdata (requred by CMake)
RUN /bin/bash -c "DEBIAN_FRONTEND='noninteractive' apt-get -y install tzdata"

# Install CMake
RUN /bin/bash -c "sudo apt-get -y install cmake"

# --------------------------------------------- ESP8266_RTOS_SDK setup -------------------------------------------

# Clonse SDK
RUN /bin/bash -c "git clone --recursive https://github.com/espressif/ESP8266_RTOS_SDK.git /home/common/ESP8266_RTOS_SDK"

# Setup environment
ENV SDK_HOME /home/common/ESP8266_RTOS_SDK

# Install dependancies
RUN /bin/bash -c "/home/common/ESP8266_RTOS_SDK/install.sh"
