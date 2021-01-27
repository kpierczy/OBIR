# ================================================================================================================
# @ Author: Krzysztof Pierczyk
# @ Create Time: 2020-01-27 13:24:12
# @ Modified time: 2020-01-27 13:24:12
# @ Description:
#
#     Sourcing this script will provide the user's terminal with a set of handy commands used widely
#     when working with docker.
#
# @ Note: Modify it to suit your needs!
# ================================================================================================================

# Show running containers
alias dps='sudo docker ps'

# Stop and remove all containers
alias drm='                                                  \
    if [[ $(sudo docker ps -a -q) != "" ]] > /dev/null; then \
        sudo docker stop $(sudo docker ps -a -q) &&          \
        sudo docker rm $(sudo docker ps -a -q);              \
    fi'

# Stop and remove all containers. Prune intermediate images.
alias prune='                                                \
    if [[ $(sudo docker ps -a -q) != "" ]] > /dev/null; then \
        sudo docker stop $(sudo docker ps -a -q) &&          \
        sudo docker rm $(sudo docker ps -a -q);              \
    fi && sudo docker image prune'

# Show docker images
alias dimg='sudo docker images'

# Remove a docker image
alias dimgrm='sudo docker rmi'