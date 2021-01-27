# OBIR Project

This directory contains realisation of the OBIR project. It encapsulates rewritten version of the popular `libcoap`
library adjusted to the project's needings. Modifications concern:

- removal of CoAP over TCP
- removal of security protocol's support
- removal of multiplatform support
- providing consistent, complete documentation for library's API
- modifications of implementation details with respect to limited resources of the choosen platform (ESP8266)

# How to run the project

To conviniently run the project on various systems, the Docker approach is recommended. Sourcing `project/scripts/virt.bash`
provides local Docker's installation with the preconfigured, Ubuntu-based image of the system ready to interact with the
project's source code via ESP8266_RTOS_SDK-provided tools. Before you source, make sure that you've set `PROJECT_HOME` variable
inside the script. After you source the file a virtual machine will be run in an interactive mode. As soon as the command prompt
appears, source the `sourceMeVirt.bash` script to perform basic configuration. The whole `project` directory will be mounted on 
the VM so you will have a direct access to it.

An application is stored in the `coap_server` folder. To build it one needs to enter it and call `idf.py build`. It may be
necessary to run `idf.py fullclean` if an earlier build was performed on the host machine.

When the build finishes the binary image can be loaded to the MCU via `idf.py flash`. `idf.py monitor` enables serial monitor
between PC and the device. Alternatively one can call `idf.py flash monitor` to run both commands sequentially.
