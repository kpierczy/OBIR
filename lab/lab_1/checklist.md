1. Install Mozilla Firefox                                         [[ - ]]
    - install Copper extension                                      [ - ]
2. Install Wireshark                                               [[ - ]]
3. Write simple CoAP server                                        [[ - ]]
    - read list and attributes of server resources using Copper     [ - ]
    - catch CoAP packages using Wireshark                           [ - ]
    - save caught packages to the file                              [ - ]
4. Create UDP server listening at port 2392                        [[ - ]]
    - server should return recorder message written backwards       [ - ]                             
    - write bash script that sends example commands to the server   [ - ]
    - catch UDP packages using Wireshark                            [ - ]
    - save caught packages to the file                              [ - ]
5. Modify program written at pt. 4                                 [[ - ]]
    - `NIECHN [hex]` command should update the first variable       [ - ]
      hold by the server                                                 
    - `NIECHO [hex]` command should update the second variable      [ - ]
      hold by the server                                                 
    - `*` command should perform multiply on both variable and put  [ - ]
      the result in the third variable                                    
    - `PODAJ` command should return value of the thirs variable     [ - ]
      in the hex format                                                 
    - write bash script that sends example commands to the server   [ - ]
    - catch UDP packages using Wireshark                            [ - ]
    - save caught packages to the file                              [ - ]
6. Write report                                                    [[ - ]]
    - place payload of the UDP packages sent to server (so it       [ - ]
      would be possible to compare it with caught packages)         [ - ]
    - place `netcat` commands used in the project
