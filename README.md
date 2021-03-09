#  AZURE IoT C SDK integration into the QCS610 and Data posting to Azure Iot-hub
- The main objective of this application is integrate the azure iot c sdk and control the camera streaming and also send the status of  camera recording to azure iot-hub.

## About the project
  In this project, one can integrate the Azure IOT C sdk into the c610 platform and also create a small application to control the camera streaming and notify the status of camera to azure iot hub. The source tree of the project starts with the application directory (/home/user/azure)

          azure/ 
                       -  video_record.c
                       -  video_record.h
                       -  main.c
                       -  cloud2device.py 
                       -  lib
                                   - contain shared libraries of azure iot c sdk
                       -  azureiot
                                   - contain the azure iot sdk header files    

## Dependencies
- Ubuntu System 18.04 or above
- Install Adb tool (Android debugging bridge) on host system
- valid microsoft azure user account
- Install python3.5 or above on the host system 

## Prerequisites
- Setting up the camera Application SDK on the host system.
- Camera Environment configuration setup on the target device.
- Setting up Resource group and device creation on azure iot-hub

### Setting up the Application SDK on the host system.
       
  - To Install application sdk, Download the Application SDK from below url:
      ```
      -  https://thundercomm.s3.ap-northeast-1.amazonaws.com/shop/doc/1593776185472315/Turbox-C610_Application-SDK_v1.0.tar.gz
      ```
   - Unpack the sdk using below command 
       ```
        tar -xzvf Turbox-C610_Application-SDK_v1.0.tar.gz
       ```
   - Execute the below script file, it will ask the default target directory, press enter and input 'Y'
       ```
       ./oecore-x86_64-armv7ahf-neon-toolchain-nodistro.0.sh
       ```
   - Now the environment setup is complete.

 ### 2. Camera Environment configuration setup on the target device.
   -  To setup the camera environment configuration follow the given document ‘Turbox-C610_Open_Kit_Software_User_Manual_LE1.0_v2.0.pdf’ In given url
      ```
      “https://www.thundercomm.com/app_en/product/1593776185472315” 
      ```
   and refer section 2.10.1

### 3. Setting up Resource group and register the device on azure iot-hub
   - To setup the resource group and register the new  device on iot hub please follow the below link    
     ```
      https://docs.microsoft.com/en-us/azure/iot-hub/iot-hub-create-through-portal
     ```
   - Note: once the setup is done, note down the primary  connection string of both registered new device as well as iot hub. 

### Steps to build and run the application: 

## Building the camera application:

**Step-1** : Enter below command to set up the cross compilation environment on the host system.
      ```
        $ git clone <source repository>
        $ cd  <source repository> 
        $ source /usr/local/oecore-x86_64/environment-setup-armv7ahf-neon-oe-linux-gnueabi
      ```
    
**Step-2** :Build the camera application binary using below command.                                       
 - **Note:**  before starting building, to add secure communication, open the main.c file and replace the connection string details with device primary connection string.  
      ```
       $CC main.c video_record.c -o iottest `pkg-config --cflags --libs gstreamer-1.0`  -I ./azureiot/ -L ./lib -liothub_client -laziotsharedutil -liothub_client_mqtt_transport -liothub_client_amqp_transport -luamqp -lumqtt -lparson
      ```    
      
**Step-3** : initialize the target board with root access.
    ```
        $ adb root 
        $ adb remount 
        $ adb shell  mount -o remount,rw /
        $ adb forward tcp:8900 tcp:8900
    ```
      
**Step-4** : Push the application binary and azure iot shared library to the target board with adb command.
    ```
      $ adb push iottest /data/azure/
      $ adb push lib/  /data/azure/
    ```      
      
## Execute the binary file in the target environment.
   - To start the application, run the below commands on the qcs610 board, 
     ```
        $ adb shell
        /#
     ``` 
  -  To enable wifi connectivity on target board
      ```  
       /# wpa_supplicant -Dnl80211 -iwlan0 -c /etc/misc/wifi/wpa_supplicant.conf -ddddt &
       /# dhcpcd wlan0
      ```  
   -  Export the shared library to the LD_LIBRARY_PATH
      ```
       / # export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/data/azure/lib/
       /# cd data/azure/
  
      ```   
  - To start the application run below command
      ```
      /# ./iottest
      ```
 - After executing the above command, the qcs610 device will wait for the command from iot hub, once it receives, it will do the required command action.

## Event monitoring on azure iot hub,
  - Open the new terminal on the host system
    Install azure cli on host system
    ```   
    $ curl -sL https://aka.ms/InstallAzureCLIDeb | sudo bash
    ```
   Login to azure 
   ```
      $ az login
   ```
  - This will open the default browser, you can enter your login credentials. After login, for monitor the device messages, execute below command
    ```      
      $ az iot hub monitor-events --hub-name <iot hub name> --device-id  <devicename>
    ```  
## For sending command from iot hub to device,
   - Open the new terminal on the host system and traverse to the repo directory.  Edit the cloud2device.py file and replace the ‘CONNECTION_STRING’ details with iothub primary connection string and replace the ‘DEVICE_ID’ details with device name.   
  
  - Install the azure iot-hub on the host device.
  
    ```
      $ pip install azure-iot-hub 
    ``` 
   - Execute the python script and send the commands via command line option. To start the video streaming on qcs610 
     ```
     $ python3 cloud2device.py starttcp    
     ``` 
  -  To stop the current streaming from qcs610
     ```   
        $ python3 cloud2device.py stoptcp    
     ```

-  Once the qcs610 receives the command, it will start streaming the video. It will send the camera status to iothub and the message can be shown on the event monitoring terminal.

- To see the live streaming,  open the new terminal in the host system and enter below command. Also make sure you have installed the ‘vlc player’ on the host system in order to view video playback. 
   ```
      $ adb forward tcp:8900 tcp:8900
      $ vlc -vvv tcp://127.0.0.1:8900
   ```  
   
