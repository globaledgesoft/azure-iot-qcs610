import random
import sys
from azure.iot.hub import IoTHubRegistryManager
import argparse


MESSAGE_COUNT = 1
MSG_TXT = "default"

CONNECTION_STRING = "Iothub connection string"
DEVICE_ID = "Device_Name"

def iothub_messaging_sample_run():
    try:
        # Create IoTHubRegistryManager
        registry_manager = IoTHubRegistryManager(CONNECTION_STRING)

        for i in range(0, MESSAGE_COUNT):
            print ( 'Sending message: {0}'.format(i) )
            data = MSG_TXT

            props={}
            # optional: assign system properties
            props.update(messageId = "message_%d" % i)
            props.update(correlationId = "correlation_%d" % i)
            props.update(contentType = "application/json")

            # optional: assign application properties
            prop_text = "PropMsg_%d" % i
            props.update(testProperty = prop_text)

            registry_manager.send_c2d_message(DEVICE_ID, data, properties=props)

    except Exception as ex:
        print ( "Unexpected error {0}" % ex )
        return
    except KeyboardInterrupt:
        print ( "IoT Hub C2D Messaging service sample stopped" )

if __name__ == '__main__':
    print ( "Starting the Python IoT Hub C2D Messaging service ..." )
    ap = argparse.ArgumentParser()
    ap.add_argument("-m", "--message", required=True,help="enter the message")
    args = vars(ap.parse_args())
    MSG_TXT = args["message"]
    iothub_messaging_sample_run()
