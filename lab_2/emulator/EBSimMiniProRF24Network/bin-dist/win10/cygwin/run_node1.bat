@rem DLL preparing
@rem For EXE compiled under cyginw-win10
@rem 
PATH=./dll-win10;%PATH%%

@echo "Run simulation"

@rem ObirRF24Network_helloworld taken from RF24Network helloworld rx/tx example (nodes 0 and 1)
EBSimMiniProRF24Network ..\..\wsady-testowe\ObirRF24Network_helloworld\para-0000-0001\ObirRF24Network_helloworld_rx_20200505.ino.hex
@rem EBSimMiniProRF24Network ..\..\wsady-testowe\ObirRF24Network_helloworld\para-0000-0001\ObirRF24Network_helloworld_tx_20200505.ino.hex

@rem ObirRF24Network_helloworld taken from RF24Network helloworld rx/tx example (nodes 0xCCDD and 0xAABB)
@rem EBSimMiniProRF24Network ..\..\wsady-testowe\ObirRF24Network_helloworld\para-CCDD-AABB\ObirRF24Network_helloworld_rx_20200505.ino.hex
@rem EBSimMiniProRF24Network ..\..\wsady-testowe\ObirRF24Network_helloworld\para-CCDD-AABB\ObirRF24Network_helloworld_tx_20200505.ino.hex

@rem ObirRF24Network client and server called as simple_rpc (not useful works)
@rem EBSimMiniProRF24Network ..\..\wsady-testowe\ObirRF24Network_rpc\simple_rpc\ObirRF24Network_server_20200508.ino.hex
@rem EBSimMiniProRF24Network ..\..\wsady-testowe\ObirRF24Network_rpc\simple_rpc\ObirRF24Network_client_20200508.ino.hex

@rem ObirRF24Network RPC based on nanoRPC protocol
@rem EBSimMiniProRF24Network ..\..\wsady-testowe\ObirRF24Network_rpc\nanoRPC_example\ObirRF24Network_nanoRPC_server_20200511.ino.hex
@rem EBSimMiniProRF24Network ..\..\wsady-testowe\ObirRF24Network_rpc\nanoRPC_example\ObirRF24Network_nanoRPC_client_20200511.ino.hex