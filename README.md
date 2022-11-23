# Raspberry pi pico W5100 Azure message

1. board image

<p align="center">
    <img width="100%" src="./image/board.jpg" />
</p>

2. How to connect raspberry pi and raspberry pi pico.
<p align="center">
    <img width="100%" src="./image/pico_Azure_connect-p-2.drawio.png" />
</p>
3. Raspberry pi sends cert data to Raspberry pi pico using serial communication.

- Send Device ID
<p align="center">
    <img width="100%" src="./image/serial_RPI_deviceID.png" />
</p>

- Send key data
<p align="center">
    <img width="100%" src="./image/serial_RPI_Key_001.png" />
</p>

- Send Cert data
<p align="center">
    <img width="100%" src="./image/serial_RPI_Cert_001.png" />
</p>

- serial communicaiton format
<p align="center">
    <img width="100%" src="./image/serial_data_format.png" />
</p>

4. Pico Azure Connect

- Pico Log
<p align="center">
    <img width="100%" src="./image/pico_Azure_connect_seccess.png" />
</p>

- Azure IoT Explorer
<p align="center">
    <img width="100%" src="./image/pico_Azure_connect_explorer.png" />
</p>

5. Azure Telemetry receive
 - Pico Send Telemetry message
<p align="center">
    <img width="100%" src="./image/pico_Azure_telemetry_send.png" />
</p>

 - Explorer receives the message
<p align="center">
    <img width="100%" src="./image/pico_Azure_telemetry_explorer.png" />
</p>


 6. Pico Direct method receive
 - Explorer sends Direct method
<p align="center">
    <img width="100%" src="./image/pico_Azure_dirctMessageSend_explorer.png" />
</p>

 - Pico receives the direct method
 <p align="center">
    <img width="100%" src="./image/pico_Azure_DirectMethodRecv.png" />
</p>