# ================================================================================================================
# @ Author: Krzysztof Pierczyk
# @ Create Time: 2021-01-27 13:32:12
# @ Modified time: 2021-01-27 13:32:19
# @ Description:
#
#     This script contains basic setup for project's workspace in the clean virtual environment (i.e. Docker
#     container). Before sourcing this script make sure that you have installed Docker distribution properly.
#     You also need to set configuration variables (given in the `Configuration` section below) according to 
#     your environment.
#
#     After you source this script the project directory will be mounted in the virtual machine's tree.
#     You have to run sourceMeVirt.bash from inside of the VM to finish the setup.
#
# @ Note: Environment set by the script uses `ubuntu:focal` container. See [hub.docker.com]
# ================================================================================================================

# ------------------------------------------------ Configuration -------------------------------------------------

# Absolute path to the project's directory
PROJECT_HOME=/home/cris/Desktop/Pierczyk_Krzysztof/project

# ------------------------------------------ Virtual environment setup -------------------------------------------

# Build the docker image
if [[ "$(sudo docker images obir:project | wc -l)" == "1" ]]; then

    printf "\nLOG: Building a docker image\n\n"
    builder="sudo docker build                  \
        -f $PROJECT_HOME/scripts/env.Dockerfile \
        -t obir:project"

    if ! $builder $PROJECT_HOME; then
        printf "\nERR: Building a docker image failes.\n"
        return
    fi
fi

# Run the container
printf "\nLOG: Running virtual environment\n\n"
sudo docker run                    \
    -it                            \
    --rm                           \
    --name "obir"                  \
    --network=host                 \
    --device=/dev/ttyUSB0          \
    -v $PROJECT_HOME:/home/project \
    obir:project