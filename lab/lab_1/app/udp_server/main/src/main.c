#include "freertos/FreeRTOS.h" // FreeRTOS tasks
#include "freertos/task.h"     // xTaskNotifify API
#include "esp_log.h"           // Logging mechanisms
#include "nvs_flash.h"         // Non-Volatile storage flash
#include "lwip/sockets.h"      // LwIP stack
#include "station.h"           // WiFi connection [auth]

#define EXAMPLE_ESP_WIFI_SSID      "Andrzej"
#define EXAMPLE_ESP_WIFI_PASS      "Gnp64wpewynh"

#define UDP_SERVER_PORT            8080

static void udp_server_task(void *pvParameters);

// Source file's tag
static char *TAG = "main";

// Global variables initialization
TaskHandle_t main_handler;

/**
 * @brief Main task.
 */
void app_main(){

    // Initialize NVS flash for other components' use
    ESP_ERROR_CHECK(nvs_flash_init());

    // LOG start of the Programm
    ESP_LOGI(TAG, "main: Connecting to WiFi AP...");

    // Connect via WiFi to the AP
    wifi_connect(EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);

    // Create UDP server tasl
    xTaskCreate(udp_server_task, "udp_server", 4096, NULL, 5, NULL);
    // Wait for udpp server task to finish
    main_handler = xTaskGetCurrentTaskHandle();
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    // Disconnect from the AP
    wifi_disconnect();
}

/**
 * @brief Udp server task.
 * 
 * @param pvParameters 
 */
static void udp_server_task(void *pvParameters){

    // Wait for main to block on xTaskNotifyTake()
    vTaskDelay(pdMS_TO_TICKS(1000));

    // Server duties
    while (true) {

        /* Create strcture describing BSD socket's address
         *     .sin_addr.s_addr : address of the socket (setting to 0 picks random
         *                        addres possessed by the server. ESP has only one
         *                        IP available at the time)
         *     .sin_family : protocoly type (AF_INET = IPv4)
         *     .sin_port : port number for th socket (setting to 0 picks random port)
         * 
         * @note : htonl() and htons() functions revert IP and PORT byte order to 
         * suite network order.
         */
        struct sockaddr_in destAddr;
        destAddr.sin_addr.s_addr = htonl(INADDR_ANY);
        destAddr.sin_family = AF_INET;
        destAddr.sin_port = htons(UDP_SERVER_PORT);

        // Convert IP address from integer representation to ASCII
        char addr_str[128];
        inet_ntoa_r(destAddr.sin_addr, addr_str, sizeof(addr_str) - 1);

        // Create socket
        int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
        if (sock < 0) {
            ESP_LOGE(TAG, "udp: Unable to create socket: errno %d", errno);
            break;
        } else
            ESP_LOGI(TAG, "udp: Socket created");

        // Bind created socket to the IP address struct (destAddr)
        int err = bind(sock, (struct sockaddr *)&destAddr, sizeof(destAddr));
        if (err < 0) {
            ESP_LOGE(TAG, "udp: Socket unable to bind: errno %d", errno);
        } else
            ESP_LOGI(TAG, "udpL Socket binded");

        // Now, when socket is created and configured we can start receiving data
        while(true) {

            ESP_LOGI(TAG, "udp: Waiting for data");
            
            // Wait for message via UDP
            struct sockaddr_in sourceAddr;
            char rx_buffer[128];
            socklen_t socklen = sizeof(sourceAddr);
            int len = recvfrom(
                sock,                            // Socket descriptor to read from
                rx_buffer,                       // Data buffer
                sizeof(rx_buffer) - 1,           // Size of the data buffer
                0,                               // Flags
                (struct sockaddr *)&sourceAddr,  // Socket  address that data came from
                &socklen                         // Length of the incoming socket's struct
            );

            // Error occured during receiving
            if (len < 0) {
                ESP_LOGE(TAG, "udp: recvfrom() failed: errno %d", errno);
                break;
            }
            // Data received
            else {

                // Get the sender's ip address as string
                inet_ntoa_r(((struct sockaddr_in *)&sourceAddr)->sin_addr.s_addr, addr_str, sizeof(addr_str) - 1);

                // Null-terminate received data to it treat like a string
                rx_buffer[len] = 0; 

                // Print received data
                ESP_LOGI(TAG, "udp: Received %d bytes from %s:", len, addr_str);
                ESP_LOGI(TAG, "udp: %s", rx_buffer);

                // Resend the same data to the sender
                int err = sendto(
                    sock,                            // Socket descriptor to write to
                    rx_buffer,                       // Data buffer
                    len,                             // Size of the data buffer
                    0,                               // Flags
                    (struct sockaddr *)&sourceAddr,  // Destination address that data came from
                    sizeof(sourceAddr)               // Length of the destination socket's struct
                );
                if (err < 0) {
                    ESP_LOGE(TAG, "udp: Error occured during sending: errno %d", errno);
                    break;
                }
            }
        }

        // If socket haven't been close yet, close now
        if (sock != -1) {
            ESP_LOGE(TAG, "udp: Shutting down socket and restarting...");
            shutdown(sock, 0);
            close(sock);
        }
    }

    // Notify main to disconec from WiFi AP
    xTaskNotifyGive(main_handler);

    // Delete task itself
    vTaskDelete(NULL);
}