#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/util/datetime.h"

#include "wizchip_conf.h"
#include "netif.h"
#include "iothub.h"
#include "iothub_device_client_ll.h"
#include "iothub_client_options.h"
#include "iothub_message.h"
#include "iothub_client_version.h"
#include "azure_c_shared_utility/threadapi.h"
#include "azure_c_shared_utility/tickcounter.h"
#include "azure_c_shared_utility/crt_abstractions.h"
#include "azure_c_shared_utility/shared_util_options.h"
#include "azure_c_shared_utility/agenttime.h"
#include "azure_c_shared_utility/http_proxy_io.h"
#include "azure_prov_client/prov_device_ll_client.h"
#include "azure_prov_client/prov_security_factory.h"

#include "mbedtls/debug.h"
#include "azure_samples.h"

#include "hardware/timer.h"
#include "hardware/gpio.h"

/* This sample uses the _LL APIs of iothub_client for example purposes.
Simply changing the using the convenience layer (functions not having _LL)
and removing calls to _DoWork will yield the same results. */

// The protocol you wish to use should be uncommented
//
#define SAMPLE_MQTT
//#define SAMPLE_MQTT_OVER_WEBSOCKETS
//#define SAMPLE_AMQP
//#define SAMPLE_AMQP_OVER_WEBSOCKETS
//#define SAMPLE_HTTP

#ifdef SAMPLE_MQTT
#include "iothubtransportmqtt.h"
#endif // SAMPLE_MQTT
#ifdef SAMPLE_MQTT_OVER_WEBSOCKETS
#include "iothubtransportmqtt_websockets.h"
#endif // SAMPLE_MQTT_OVER_WEBSOCKETS
#ifdef SAMPLE_AMQP
#include "iothubtransportamqp.h"
#endif // SAMPLE_AMQP
#ifdef SAMPLE_AMQP_OVER_WEBSOCKETS
#include "iothubtransportamqp_websockets.h"
#endif // SAMPLE_AMQP_OVER_WEBSOCKETS
#ifdef SAMPLE_HTTP
#include "iothubtransporthttp.h"
#endif // SAMPLE_HTTP

#ifdef SET_TRUSTED_CERT_IN_SAMPLES
#include "certs.h"
#endif // SET_TRUSTED_CERT_IN_SAMPLES

/* Paste in the your iothub connection string  */
// static const char* connectionString = "[device connection string]";
static const char *connectionString = pico_az_connectionString;

//#define MESSAGE_COUNT 3
static bool g_continueRunning = true;
static size_t g_message_count_send_confirmations = 0;
static size_t g_message_recv_count = 0;

const uint RESET_PIN = 25; // TEST PICO LED

// const uint PI_READ_PIN = 2; // PI에서 GPIO read 하는 핀
// const uint LIGHT_PIN = 3;  //도광판 제어 (3)핀
// const uint BUTTON_PIN = 4; // Button Pin
// const uint STATUS_PIN = 5; // RPi status gpio
// const uint RESET_PIN = 6;  // Reset raspberry pi

/*Global value*/
// int status_value = 0;
// int status_flag = 0;

// int light_blink = 0;

// int64_t alarm_callback(alarm_id_t id, void *user_data)
// {
//     int g_read = gpio_get(STATUS_PIN);
//     if (g_read == 1)
//     {
//         status_value = status_value + 1;
//     }
//     else if (g_read == 0)
//     {
//         status_value = status_value - 1;
//     }
//     printf("gpio read value = %d\n", status_value);
//     if (status_value >= 20 || status_value <= -20)
//     {
//         printf("Service not operation\n");
//         status_value = 0;
//         status_flag = 1;
//     }
//     else
//     {
//         status_flag = 0;
//     }
//     return 500000;
// }

static void send_confirm_callback(IOTHUB_CLIENT_CONFIRMATION_RESULT result, void *userContextCallback)
{
    (void)userContextCallback;
    // When a message is sent this callback will get envoked
    g_message_count_send_confirmations++;
    (void)printf("Confirmation callback received for message %lu with result %s\r\n", (unsigned long)g_message_count_send_confirmations, MU_ENUM_TO_STRING(IOTHUB_CLIENT_CONFIRMATION_RESULT, result));
}

static void connection_status_callback(IOTHUB_CLIENT_CONNECTION_STATUS result, IOTHUB_CLIENT_CONNECTION_STATUS_REASON reason, void *user_context)
{
    (void)reason;
    (void)user_context;
    // This sample DOES NOT take into consideration network outages.
    if (result == IOTHUB_CLIENT_CONNECTION_AUTHENTICATED)
    {
        (void)printf("The device client is connected to iothub\r\n");
    }
    else
    {
        (void)printf("The device client has been disconnected\r\n");
        g_continueRunning = false;
    }
}

static int deviceMethodCallback(const char *method_name, const unsigned char *payload, size_t size, unsigned char **response, size_t *response_size, void *userContextCallback)
{
    (void)userContextCallback;
    (void)payload;
    (void)size;

    int result;

    gpio_init(RESET_PIN);
    gpio_set_dir(RESET_PIN, GPIO_OUT);

    // User led on
    printf("\n====================================================\n");
    printf("Device method %s arrived...\n", method_name);

    if (strcmp("reset", method_name) == 0)
    {
        printf("\nReceived device powerReset request.\n");

        const char deviceMethodResponse[] = "{ \"Response\": \"arhis_device Reset OK\" }";
        *response_size = sizeof(deviceMethodResponse) - 1;
        *response = malloc(*response_size);
        (void)memcpy(*response, deviceMethodResponse, *response_size);

        printf("POWER SHUTDOWN\r\n");
        gpio_put(RESET_PIN, 1);
        sleep_ms(3000);
        printf("POWER ON\r\n");
        gpio_put(RESET_PIN, 0);

        printf("Sent signal to Raspberry Pi.\n\n");

        result = 200;
    }
    else
    {
        // All other entries are ignored.
        const char deviceMethodResponse[] = "{ }";
        *response_size = sizeof(deviceMethodResponse) - 1;
        *response = malloc(*response_size);
        (void)memcpy(*response, deviceMethodResponse, *response_size);

        result = -1;
    }
    printf("====================================================\n\n");

    return result;
}

void iothub_ll_c2d_sample(void)
{
    IOTHUB_CLIENT_TRANSPORT_PROVIDER protocol;
    size_t messages_count = 0;
    IOTHUB_MESSAGE_HANDLE message_handle;
    size_t messages_sent = 0;
    int g_read;

    float telemetry_temperature;
    float telemetry_humidity;
    const char *telemetry_scale = "Celsius";
    // const char* telemetry_msg = "test_message";
    char telemetry_msg_buffer[80];

    // add_alarm_in_ms(500, alarm_callback, NULL, false);

    // Select the Protocol to use with the connection
#ifdef SAMPLE_MQTT
    protocol = MQTT_Protocol;
#endif // SAMPLE_MQTT
#ifdef SAMPLE_MQTT_OVER_WEBSOCKETS
    protocol = MQTT_WebSocket_Protocol;
#endif // SAMPLE_MQTT_OVER_WEBSOCKETS
#ifdef SAMPLE_AMQP
    protocol = AMQP_Protocol;
#endif // SAMPLE_AMQP
#ifdef SAMPLE_AMQP_OVER_WEBSOCKETS
    protocol = AMQP_Protocol_over_WebSocketsTls;
#endif // SAMPLE_AMQP_OVER_WEBSOCKETS
#ifdef SAMPLE_HTTP
    protocol = HTTP_Protocol;
#endif // SAMPLE_HTTP

    // Used to initialize IoTHub SDK subsystem
    (void)IoTHub_Init();
    IOTHUB_DEVICE_CLIENT_LL_HANDLE device_ll_handle;

    (void)printf("Creating IoTHub Device handle\r\n");
    // Create the iothub handle here
    device_ll_handle = IoTHubDeviceClient_LL_CreateFromConnectionString(connectionString, protocol);
    if (device_ll_handle == NULL)
    {
        (void)printf("Failure creating IotHub device. Hint: Check your connection string.\r\n");
    }
    else
    {
        // Set any option that are neccessary.
        // For available options please see the iothub_sdk_options.md documentation
#ifndef SAMPLE_HTTP
        // Can not set this options in HTTP
        bool traceOn = true;
        IoTHubDeviceClient_LL_SetOption(device_ll_handle, OPTION_LOG_TRACE, &traceOn);
#endif

#ifdef SET_TRUSTED_CERT_IN_SAMPLES
        // Setting the Trusted Certificate. This is only necessary on systems without
        // built in certificate stores.
        IoTHubDeviceClient_LL_SetOption(device_ll_handle, OPTION_TRUSTED_CERT, certificates);
#endif // SET_TRUSTED_CERT_IN_SAMPLES

#if defined SAMPLE_MQTT || defined SAMPLE_MQTT_OVER_WEBSOCKETS
        // Setting the auto URL Encoder (recommended for MQTT). Please use this option unless
        // you are URL Encoding inputs yourself.
        // ONLY valid for use with MQTT
        bool urlEncodeOn = true;
        IoTHubDeviceClient_LL_SetOption(device_ll_handle, OPTION_AUTO_URL_ENCODE_DECODE, &urlEncodeOn);
#endif

#ifdef SAMPLE_HTTP
        unsigned int timeout = 241000;
        // Because it can poll "after 9 seconds" polls will happen effectively // at ~10 seconds.
        // Note that for scalabilty, the default value of minimumPollingTime
        // is 25 minutes. For more information, see:
        // https://azure.microsoft.com/documentation/articles/iot-hub-devguide/#messaging
        unsigned int minimumPollingTime = 9;
        IoTHubDeviceClient_LL_SetOption(device_ll_handle, OPTION_MIN_POLLING_TIME, &minimumPollingTime);
        IoTHubDeviceClient_LL_SetOption(device_ll_handle, OPTION_HTTP_TIMEOUT, &timeout);
#endif // SAMPLE_HTTP
       // Setting connection status callback to get indication of connection to iothub
        (void)IoTHubDeviceClient_LL_SetConnectionStatusCallback(device_ll_handle, connection_status_callback, NULL);

        if (IoTHubDeviceClient_LL_SetDeviceMethodCallback(device_ll_handle, deviceMethodCallback, &messages_count) != IOTHUB_CLIENT_OK)
        {
            (void)printf("ERROR: IoTHubClient_LL_SetMessageCallback..........FAILED!\r\n");
        }
        else
        {
            do
            {
                // printf("stat flag = %d\n", status_flag);
                // if (status_flag == 1)
                if (RESET_PIN == 1)
                {
                    // // Construct the iothub message from a string or a byte array
                    // message_handle = IoTHubMessage_CreateFromString(telemetry_msg);
                    // //message_handle = IoTHubMessage_CreateFromByteArray((const unsigned char*)msgText, strlen(msgText)));

                    // Construct the iothub message
                    sprintf(telemetry_msg_buffer, "{\"message\" : \"arhis device not operated\"}");
                    message_handle = IoTHubMessage_CreateFromString(telemetry_msg_buffer);

                    // Set Message property
                    (void)IoTHubMessage_SetMessageId(message_handle, "MSG_ID");
                    (void)IoTHubMessage_SetCorrelationId(message_handle, "CORE_ID");
                    (void)IoTHubMessage_SetContentTypeSystemProperty(message_handle, "application%2fjson");
                    (void)IoTHubMessage_SetContentEncodingSystemProperty(message_handle, "utf-8");

                    // Add custom properties to message
                    //(void)IoTHubMessage_SetProperty(message_handle, "property_key", "property_value");
                    // dont use blank, special char. need encoding
                    (void)IoTHubMessage_SetProperty(message_handle, "display_message", "Hello_RP2040_W5100S");

                    //(void)printf("Sending message %d to IoTHub\r\n", (int)(messages_sent + 1));
                    // IoTHubDeviceClient_LL_SendEventAsync(device_ll_handle, message_handle, send_confirm_callback, NULL);
                    (void)printf("\r\nSending message to IoTHub\r\nMessage: %s\r\n", telemetry_msg_buffer);
                    // IoTHubDeviceClient_SendEventAsync(device_handle, message_handle, send_confirm_callback, NULL);
                    IoTHubDeviceClient_LL_SendEventAsync(device_ll_handle, message_handle, send_confirm_callback, NULL);

                    // The message is copied to the sdk so the we can destroy it
                    IoTHubMessage_Destroy(message_handle);
                }

                IoTHubDeviceClient_LL_DoWork(device_ll_handle);
                sleep_ms(500); // wait for

            } while (true);
        }
        // Clean up the iothub sdk handle
        IoTHubDeviceClient_LL_Destroy(device_ll_handle);
    }
    // Free all the sdk subsystem
    IoTHub_Deinit();
} //===========================