# ===============================================================================
# Script runs one of three simple tests that provide way to catch appropriate 
# logs to the report using Wireshark (TShark)
# ===============================================================================

SERVER=192.168.0.143
PORTS=("5683" "2392" "2392")
PROJECTS=("coap_server" "udp_becho" "udp_calc")

# ------------------------------------ Tests ------------------------------------

main(){

    for exercise in 1 2 3; do

        echo
        echo "----------------------------- Exercise $exercise -----------------------------"
        echo 

        PROJECT=${PROJECTS[$(($exercise - 1))]}
        PORT=${PORTS[$(($exercise - 1))]}

        # Build project
        echo $IDX
        cd $PROJECT_HOME/lab/lab_1/app/$PROJECT
        echo "Building $PROJECT project..."
        if ! make 2>&1 | tee > /dev/null; then
            echo "ERR: Building exercise $exercise failed. Leaving..."
            exit
        fi

        # Run CoAP server
        echo "LOG: Flashing ESP with project"
        make flash 2>&1 | tee > /dev/null
        while [[ ! $? -eq 0 ]]
        do
            echo "ERR: Flashing failed. Retrying..."
            make flash 2>&1 | tee > /dev/null
        done
        sleep 3

        # Run TShark with an appropriate filter
        echo "LOG: Opening tshark tool"
        tshark -f "udp port $PORT" -i any -a duration:5 -w - 1> $PROJECT_HOME/lab/lab_1/report/data/ex_$exercise.pcapng  2> /dev/null & 
        sleep 1

        # Run test
        case "$exercise" in
            1) 
                exercise_1
                ;;
            2) 
                exercise_2
                ;;
            3) 
                exercise_3
                ;;
        esac

        sleep 2

    done

    echo
}

# ---------------------------------- Functions ----------------------------------

# Interacting with a CoAP server
exercise_1(){

    coap-client -m    get coap://$SERVER:$PORT/.well-known/core &> /dev/null 
    coap-client -m    get coap://$SERVER:$PORT/time             &> /dev/null 
    coap-client -m delete coap://$SERVER:$PORT/time             &> /dev/null 
    coap-client -m    get coap://$SERVER:$PORT/colour           &> /dev/null 
    coap-client -m delete coap://$SERVER:$PORT/colour           &> /dev/null 

    echo -n "12 728 -20"  | coap-client -m put coap://$SERVER:$PORT/colour -f - &> /dev/null

    coap-client -m    get coap://$SERVER:$PORT/colour           &> /dev/null

}

# Interacting with a UDP server
exercise_2(){

    echo -n "abcdef"                               | nc -u $SERVER $PORT -w0
    sleep 1
    echo -n "Alice wants to send a message to Bob" | nc -u $SERVER $PORT -w0
    sleep 1

}

# Interacting with a UDP-base calculator
exercise_3(){

    echo -n        "PODAJ" | nc -u $SERVER $PORT -w0
    sleep 1

    echo -n "NIECHN 0x576" | nc -u $SERVER $PORT -w0
    echo -n "NIECHO 0x298" | nc -u $SERVER $PORT -w0
    echo -n            "*" | nc -u $SERVER $PORT -w0
    echo -n        "PODAJ" | nc -u $SERVER $PORT -w0
    sleep 1

    echo -n "NIECHN 0x311" | nc -u $SERVER $PORT -w0
    echo -n "NIECHO 0x987" | nc -u $SERVER $PORT -w0
    echo -n            "*" | nc -u $SERVER $PORT -w0
    echo -n        "PODAJ" | nc -u $SERVER $PORT -w0
    sleep 1

}

# ------------------------------------- Run -------------------------------------

main
