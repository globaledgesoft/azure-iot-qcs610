// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

/*
CAUTION: Checking of return codes and error values shall be omitted for brevity in this sample.
This sample is to demonstrate azure IoT client concepts only and is not a guide design principles or style.
Please practice sound engineering practices when writing production code.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> 
#include <sys/types.h> 
#include <signal.h> 

#include <pthread.h>

#include "iothub.h"
#include "iothub_device_client.h"
#include "iothub_client_options.h"
#include "iothub_message.h"
#include "azure_c_shared_utility/threadapi.h"
#include "azure_c_shared_utility/crt_abstractions.h"
#include "azure_c_shared_utility/platform.h"
#include "azure_c_shared_utility/shared_util_options.h"
#include "azure_c_shared_utility/tickcounter.h"


//#include "video_record.h"
volatile int myflag = -1;
extern void sendEos();


IOTHUB_DEVICE_CLIENT_HANDLE device_handle;


// The protocol you wish to use should be uncommented

#define SAMPLE_MQTT

#ifdef SAMPLE_MQTT
    #include "iothubtransportmqtt.h"
#endif // SAMPLE_MQTT


#ifdef SET_TRUSTED_CERT_IN_SAMPLES
#include "certs.h"
#endif // SET_TRUSTED_CERT_IN_SAMPLES

/* Paste in your device connection string  */
static const char* connectionString = "<device connection string>";


static bool g_continueRunning = true;
int status;
int g_interval = 10000;  // 10 sec send interval initially
static size_t g_message_count_send_confirmations = 0;

volatile int action_no = -1, action_prev = -1;
pid_t userpid;

static void send_confirm_callback(IOTHUB_CLIENT_CONFIRMATION_RESULT result, void* userContextCallback)
{
    (void)userContextCallback;
    // When a message is sent this callback will get invoked
    g_message_count_send_confirmations++;
    (void)printf("Confirmation callback received for message %lu with result %s\r\n", (unsigned long)g_message_count_send_confirmations, MU_ENUM_TO_STRING(IOTHUB_CLIENT_CONFIRMATION_RESULT, result));
}

 void send_message(IOTHUB_DEVICE_CLIENT_HANDLE handle, char* message)
{
            
	IOTHUB_MESSAGE_HANDLE message_handle = IoTHubMessage_CreateFromString(message);

	// Set system properties
	(void)IoTHubMessage_SetMessageId(message_handle, "MSG_ID");		// Construct the iothub message
	(void)IoTHubMessage_SetCorrelationId(message_handle, "CORE_ID");
	(void)IoTHubMessage_SetContentTypeSystemProperty(message_handle, "application%2fjson");
	(void)IoTHubMessage_SetContentEncodingSystemProperty(message_handle, "utf-8");


	// Add custom properties to message
    (void)IoTHubMessage_SetProperty(message_handle, "property_key", "property_value");

    (void)printf("\r\nSending message %d to IoTHub\r\nMessage: %s\r\n", (int)(1), message);
    IoTHubDeviceClient_SendEventAsync(handle, message_handle, send_confirm_callback, NULL);

	IoTHubMessage_Destroy(message_handle);
}


static IOTHUBMESSAGE_DISPOSITION_RESULT receive_msg_callback(IOTHUB_MESSAGE_HANDLE message, void* user_context)
{
     (void)user_context;
    const char* messageId;
    const char* correlationId;

    // Message properties
    if ((messageId = IoTHubMessage_GetMessageId(message)) == NULL)
    {
        messageId = "<unavailable>";
    }

    if ((correlationId = IoTHubMessage_GetCorrelationId(message)) == NULL)
    {
        correlationId = "<unavailable>";
    }

    IOTHUBMESSAGE_CONTENT_TYPE content_type = IoTHubMessage_GetContentType(message);
    if (content_type == IOTHUBMESSAGE_BYTEARRAY)
    {
        const unsigned char* buff_msg;
        size_t buff_len;

        if (IoTHubMessage_GetByteArray(message, &buff_msg, &buff_len) != IOTHUB_MESSAGE_OK)
        {
            (void)printf("Failure retrieving byte array message\r\n");
        }
        else
        {
            (void)printf("Received Binary message\r\nMessage ID: %s\r\n Correlation ID: %s\r\n Data: <<<%.*s>>> & Size=%d\r\n", messageId, correlationId, (int)buff_len, buff_msg, (int)buff_len);
        }

            if (memcmp(buff_msg ,"starttcp", 8)==0){
 				send_message(device_handle, "start streaming");
                action_no = 1 ;
      	}

			if (memcmp(buff_msg ,"stoptcp", 7)==0){			
 				send_message(device_handle, "stop streaming");
				action_no = 2 ;                
			}
      
    }
    else
    {
        const char* string_msg = IoTHubMessage_GetString(message);
        if (string_msg == NULL)
        {
            (void)printf("Failure retrieving byte array message\r\n");
        }
        else
        {
            (void)printf("Received String Message\r\nMessage ID: %s\r\n Correlation ID: %s\r\n Data: <<<%s>>>\r\n", messageId, correlationId, string_msg);
        }
    }
    return IOTHUBMESSAGE_ACCEPTED;
}


static int device_method_callback(const char* method_name, const unsigned char* payload, size_t size, unsigned char** response, size_t* resp_size, void* userContextCallback)
{
    const char* SetTelemetryIntervalMethod = "SetTelemetryInterval";
    const char* device_id = (const char*)userContextCallback;
    char* end = NULL;
    int newInterval;

    int status = 501;
    const char* RESPONSE_STRING = "{ \"Response\": \"Unknown method requested.\" }";

    (void)printf("\r\nDevice Method called for device %s\r\n", device_id);
    (void)printf("Device Method name:    %s\r\n", method_name);
    (void)printf("Device Method payload: %.*s\r\n", (int)size, (const char*)payload);

    if (strcmp(method_name, SetTelemetryIntervalMethod) == 0)
    {
        if (payload)
        {
            newInterval = (int)strtol((char*)payload, &end, 10);

            // Interval must be greater than zero.
            if (newInterval > 0)
            {
                // expect sec and covert to ms
                g_interval = 1000 * (int)strtol((char*)payload, &end, 10);
                status = 200;
                RESPONSE_STRING = "{ \"Response\": \"Telemetry reporting interval updated.\" }";
            }
            else
            {
                status = 500;
                RESPONSE_STRING = "{ \"Response\": \"Invalid telemetry reporting interval.\" }";
            }
        }
    }

    (void)printf("\r\nResponse status: %d\r\n", status);
    (void)printf("Response payload: %s\r\n\r\n", RESPONSE_STRING);

    *resp_size = strlen(RESPONSE_STRING);
    if ((*response = malloc(*resp_size)) == NULL)
    {
        status = -1;
    }
    else
    {
        memcpy(*response, RESPONSE_STRING, *resp_size);
    }
            

    return status;
}


static void connection_status_callback(IOTHUB_CLIENT_CONNECTION_STATUS result, IOTHUB_CLIENT_CONNECTION_STATUS_REASON reason, void* user_context)
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
    }
}


void * action(void *arg)
{

while(1) {
if ((action_no == 1) && (action_prev == -1)) {
     // playing 
      int ret = fork();
       if(ret == 0){
           char *strelement[3] = {"dummy","tcp","127.0.0.1"}; 
           video_record(3, strelement);      
      }else{
       userpid = ret;
      action_no = -1;   
      action_prev = 1;
      }
}
else if((action_no == 2)&&(action_prev == 1) ){
         action_no = -1;
         action_prev = -1;
      // stoping 
       kill(userpid,SIGTERM);  
        usleep(1000000);
}
else if((action_no == 1)||(action_no == 2)){
  g_print("invalid_state");
 action_no = -1; 
} 


usleep(100000);
}
return 0;
}



int main(void)
{
    IOTHUB_CLIENT_TRANSPORT_PROVIDER protocol;

    IOTHUB_MESSAGE_HANDLE message_handle;

    printf("\r\nThis sample will send messages continuously and accept C2D messages.\r\nPress Ctrl+C to terminate the sample.\r\n\r\n");

    // Select the Protocol to use with the connection
#ifdef SAMPLE_MQTT
    protocol = MQTT_Protocol;
#endif // SAMPLE_MQTT

    // Used to initialize IoTHub SDK subsystem
    (void)IoTHub_Init();

    (void)printf("Creating IoTHub handle\r\n");
    // Create the iothub handle here
    device_handle = IoTHubDeviceClient_CreateFromConnectionString(connectionString, protocol);
    if (device_handle == NULL)
    {
        (void)printf("Failure creating IotHub device. Hint: Check your connection string.\r\n");
    }
    else
    {

        // Setting message callback to get C2D messages
        (void)IoTHubDeviceClient_SetMessageCallback(device_handle, receive_msg_callback, NULL);
        (void)IoTHubDeviceClient_SetDeviceMethodCallback(device_handle, device_method_callback, NULL);
        // Setting connection status callback to get indication of connection to iothub
        (void)IoTHubDeviceClient_SetConnectionStatusCallback(device_handle, connection_status_callback, NULL);

        tickcounter_ms_t ms_delay = 10;
        (void)IoTHubDeviceClient_SetOption(device_handle, OPTION_DO_WORK_FREQUENCY_IN_MS, &ms_delay);

#ifdef SET_TRUSTED_CERT_IN_SAMPLES
        // Setting the Trusted Certificate. This is only necessary on systems without
        // built in certificate stores.
        (void)IoTHubDeviceClient_SetOption(device_handle, OPTION_TRUSTED_CERT, certificates);
#endif // SET_TRUSTED_CERT_IN_SAMPLES


#if defined SAMPLE_MQTT || defined SAMPLE_MQTT_OVER_WEBSOCKETS
        bool urlEncodeOn = true;
        (void)IoTHubDeviceClient_SetOption(device_handle, OPTION_AUTO_URL_ENCODE_DECODE, &urlEncodeOn);
#endif
  
             pthread_t action_thread;
    		 pthread_attr_t tattr4;
    		 pthread_attr_init(&tattr4);
    		 pthread_attr_setdetachstate(&tattr4, PTHREAD_CREATE_DETACHED);
    		 pthread_create(&action_thread, &tattr4, action, (void*)NULL); 
         
            while(1)
			{
              usleep(1000000);
            }

                char *sthr;
                pthread_join(action_thread,(void**)&sthr);
                free(sthr);

        // Clean up the iothub sdk handle
        IoTHubDeviceClient_Destroy(device_handle);
    }
    // Free all the sdk subsystem
    IoTHub_Deinit();

    return 0;
}
