@rem DLL preparing
@rem For EXE compiled under cyginw-win10
@rem 
PATH=./dll-win10;%PATH%%

@rem Exe simulation
@echo "Run simulation"
EBSimUnoEth.exe -ip 10.17.0.238 ..\\..\\wsady-testowe\\ntp-client\\ObirUdpNtpClient_20200403.ino.hex
