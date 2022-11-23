#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/irq.h"
#include "w5100s.h"
#include "crc16.h"
#include "proto_pi_CMD.h"
#include "manage_process.h"
#include "azure_samples.h"


#define UART_ID uart1
#define BAUD_RATE 115200
#define DATA_BITS 8
#define STOP_BITS 1
#define PARITY    UART_PARITY_NONE

#define UART_TX_PIN 8
#define UART_RX_PIN 9


serial_data proto_data;
uint8_t uart_flag = 0;

void proto_recv(void)
{
    uint8_t ch;
    //to do time check
    while (uart_is_readable(UART_ID)) 
    {
        ch = uart_getc(UART_ID);
        //printf("%02x ", ch);
        #if DEBUG_PRINT
        uart_putc(uart1, ch);
        #endif
        switch(proto_data.seq)
        {
            case 0:     //stx
                if(ch == 0x02)
                {
                    proto_data.seq++;
                    #if DEBUG_PRINT
                    uart_puts(uart1, "[s]");
                    #endif
                }
                break;
            case 1:     //Len
                proto_data.buffer[proto_data.index++] = ch;
                if(proto_data.index>=2)
                {
                    proto_data.len = (proto_data.buffer[0] << 8) | proto_data.buffer[1];
                    //printf("Len[%d]", proto_data.len);
                    proto_data.seq++;
                }
                break;
            case 2:     //CMD
                proto_data.buffer[proto_data.index++] = ch;
                //if((proto_data.index >= proto_data.len +2 + 2) && (ch !=0x03))
                if(proto_data.index > (proto_data.len + 2 + 2))
                {
                    if(ch == 0x03)
                    {
                        proto_data.flag = 1;
                        #if DEBUG_PRINT
                        printf("proto recv complete[%d]\r\n", proto_data.index);
                        #endif
                    }
                    else
                    {
                        proto_data.seq = 0;
                        proto_data.index = 0;
                        #if DEBUG_PRINT
                        printf("proto recv fail[%d][%02x]\r\n", proto_data.index, ch);
                        #endif
                    }
                }
                break;
        }
    }
    proto_data.lastTime = time_us_64();
}

int proto_send(uint8_t *CMD, uint8_t *Data, uint16_t Len)
{
    uint8_t *buff = NULL;
    uint8_t *temp_buff = NULL;
    uint8_t *CMD_Index = NULL;
    uint16_t temp_crc = 0, cnt = 0;
    buff = (uint8_t *)calloc(Len + 8 , sizeof(char));
    temp_buff = buff;
    *temp_buff++ = 0x02;
    *temp_buff++ = ((Len +2)>>8)&0x00ff;
    *temp_buff++ = (Len +2)&0x00ff;
    CMD_Index = temp_buff;
    *temp_buff++ = *CMD;
    CMD++;
    *temp_buff++ = *CMD;
    if(Len != 0)
    {
        memcpy(temp_buff, Data, Len);
        temp_buff += Len;
    }
    #if DEBUG_PRINT
    printf("CMD : %s|\r\n", CMD_Index);
    #endif
    temp_crc = crc16(CMD_Index, Len + 2);
    #if DEBUG_PRINT
    printf("CRC : %04x \r\n", temp_crc);
    #endif
    *temp_buff++ = (temp_crc >> 8) & 0x00ff;
    *temp_buff++ = temp_crc & 0x00ff;
    *temp_buff = 0x03;
    temp_buff = buff;
    for(cnt = 0; cnt < Len + 8; cnt++)
    {
        uart_putc(UART_ID, *temp_buff++);
    }
    #if DEBUG_PRINT
    printf("send complete\r\n");
    #endif
    #if DEBUG_UDP_EN
    //Debug_Len = sprintf(Debug_buff, "CMD:%s, CRC: %04x send\r\n", CMD, temp_crc);
    Debug_Len = sprintf(Debug_buff, "CMD:%s, data:%s[Send]\r\n", CMD, Data);
    DebugSend(Debug_buff, Debug_Len);
    //DebugSend(Data, Len);
    #endif
    free(buff);
    return Len + 8;
}
void uart_rx_irq()
{
    proto_recv();
#if 0
    uint8_t ch;

    while (uart_is_readable(UART_ID)) {
        ch = uart_getc(UART_ID);
        switch(proto_seq)
        {
            case 0:     //stx
                break;
            case 1:     //Len
                break;
            case 2:     //CMD
                break;
        }
        // Can we send it back?
        #if 0
        if (uart_is_writable(UART_ID)) {
            // Change it slightly first!
            ch;
            uart_putc(UART_ID, ch);
        }
        chars_rxed++;
        #endif
    }
#endif
}


void proto_uart_init(void)
{
    uart_init(UART_ID, 2400);

    // Set the TX and RX pins by using the function select on the GPIO
    // Set datasheet for more information on function select
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    // Actually, we want a different speed
    // The call will return the actual baud rate selected, which will be as close as
    // possible to that requested
    int __unused actual = uart_set_baudrate(UART_ID, BAUD_RATE);

    // Set UART flow control CTS/RTS, we don't want these, so turn them off
    uart_set_hw_flow(UART_ID, false, false);

    // Set our data format
    uart_set_format(UART_ID, DATA_BITS, STOP_BITS, PARITY);

    // Turn off FIFO's - we want to do this character by character
    uart_set_fifo_enabled(UART_ID, false);

    // Set up a RX interrupt
    // We need to set up the handler first
    // Select correct interrupt for the UART we are using
    int UART_IRQ = UART_ID == uart0 ? UART0_IRQ : UART1_IRQ;

    // And set up and enable the interrupt handlers
    irq_set_exclusive_handler(UART_IRQ, uart_rx_irq);
    irq_set_enabled(UART_IRQ, true);

    // Now enable the UART to send interrupts - RX only
    uart_set_irq_enables(UART_ID, true, false);

    uart_puts(UART_ID, "\nHello, uart interrupts\n");
    proto_data.flag = 0;
    proto_data.index = 0;
    proto_data.seq = 0;
}

int W5100s_ping(void)
{
    char send_ip[4] = {8,8,8,8};
    char slir_value = 0x00;
    setSLPIPR(send_ip);
    // set SOCKET-less Retransmission Time, 100ms(0x03E8) 5s(0xEA60)(단위, 100us)
    setSLRTR(0xEA60);//0x03E8;
    /* set SOCKET-less Retransmission counter, 5 */
    setSLRCR(0x03);
    /* set Interrupt Mask Bit */
    setSLIMR(0x07); //|TIMEOUT|ARP|PING| 0x07 = 0b0111
    setPINGIDR(0x5100);
    setPINGSEQR(0x1234);
    setSLCR(0x01);
    while(slir_value == 0x00)
    {
        slir_value = getSLIR();
    }
    if(slir_value & 0x01) 
    {
        #if DEBUG_PRINT
        printf("Ping Replay OK");
        #endif
        #if DEBUG_UDP_EN
        DebugSend("Ping Replay OK", strlen("Ping Replay OK"));
        #endif
        return 1;
    }
    else if(slir_value & 0x04) 
    {
        #if DEBUG_PRINT
        printf("Timeout Error");
        #endif
        #if DEBUG_UDP_EN
        DebugSend("Timeout Error", strlen("Timeout Error"));
        #endif
    }
    return 0;
    //SLIRCLR = 0xFF;
}

int proto_CMC_NS(void)
{
    uint8_t ret = 0;
    ret = W5100s_ping();
    if(ret == 1 )
    {
        proto_send("NS", "SUCCESS", 7);
        #if DEBUG_PRINT
        printf("ping send Success\r\n");
        #endif
        return 1;
    }
    proto_send("NS", "FAIL", 4);
    #if DEBUG_PRINT
    printf("ping fail\r\n");
    #endif
    return 0;
}
int proto_CMD_Process(uint8_t *CMD, uint8_t *Data, uint16_t Len)
{
    uint16_t ret = 0;
    float CMD_volt_data[2];
    uint8_t Temp_Buff[1024];
    uint16_t Temp_BuffLen = 0;
    #if DEBUG_PRINT
    printf("CMD Process : %c, %c \r\n ", *CMD, *(CMD + 1));
    #endif
    #if DEBUG_UDP_EN
    Debug_Len = sprintf(Debug_buff, "CMD_Process CMD:%s, Len:%d \r\n", CMD, Len);
    DebugSend(Debug_buff, Debug_Len);
    #endif
    switch(*CMD)
    {
        case 'C':
            if(*(CMD+1) == 'D')     //Cert Data
            {
                if(Len == 0)
                {   //get
                    if(Test_CertLen == 0xffff)
                    {
                        proto_send("CD", "NONE", 4);
                    }
                    else
                    {
                        proto_send("CD", Test_Cert, Test_CertLen);
                    }

                    return 1;
                }
                else
                {   //set
                    ret = proto_flash_write(FLASH_CERT_OFFSET, Data, Len);
                    Test_Cert = (uint8_t *)proto_flash_read(FLASH_CERT_OFFSET, &Test_CertLen);
                    FREE_Cert();
                    Load_Cert();
                    #if DEBUG_PRINT
                    printf("Data[%d]:[%s]", Len, Data);
                    printf("cert flash write[%d]\r\n", ret);
                    print_buf_c(Test_Cert, Test_CertLen);
                    #endif
                    #if 0
                    if(ret > 0)
                        proto_send("OK", 0, 0);
                    else
                        proto_send("ER", 0, 0);
                    #endif
                }
            }
            else
            {
                #if DEBUG_PRINT
                printf("CMD no match \r\n");
                #endif
                return -1;
            }
            break;
        case 'D':
            if(*(CMD+1) == 'E')     //Device ID
            {
                if(Len == 0)
                {   //get
                    if(Test_DeviceIDLen == 0xffff)
                    {
                        proto_send("DE", "NONE", 4);
                    }
                    else
                    {
                        proto_send("DE", Test_DeviceID, Test_DeviceIDLen);
                    }
                    return 1;
                }
                else
                {   //set
                    ret = proto_flash_write(FLASH_DEVICEID_OFFSET, Data, Len);
                    Test_DeviceID = (uint8_t *)proto_flash_read(FLASH_DEVICEID_OFFSET, &Test_DeviceIDLen);
                    FREE_DeviceID();
                    Load_DeviceID();
                    #if DEBUG_PRINT
                    printf("Data[%d]:[%s]\r\n", Len, Data);
                    printf("DeviceID flash write[%d]\r\n", ret);
                    print_buf_c(Test_DeviceID, Test_DeviceIDLen);
                    #endif
                    return 1;
                }
            }
            else
            {
                #if DEBUG_PRINT
                printf("CMD no match \r\n");
                #endif
                return -1;
            }
            break;
        case 'E':
            if(*(CMD+1) == 'R')     //ERROR
            {
                if(Len == 0)
                {   //get
                }
                else
                {   //set
                }
            }
            else
            {
                #if DEBUG_PRINT
                printf("CMD no match \r\n");
                #endif
                return -1;
            }
            break;
        case 'N':
            if(*(CMD+1) == 'S')     //network status (ping)
            {
                if(Len == 0)
                {   //get
                    proto_CMC_NS();
                    return 1;
                }
                else
                {   //set
                }
            }
            else if(*(CMD+1) == 'D') //Network Data (MAC...)
            {
            }
            else
            {
                printf("CMD no match \r\n");
                return -1;
            }
            break;
        case 'O':
            if(*(CMD+1) == 'K')      //ok
            {
                if(Len == 0)
                {   //get
                }
                else
                {   //set
                }
            }
            else
            {
                printf("CMD no match \r\n");
                return -1;
            }
            break;
        case 'S':
            if(*(CMD+1) == 'K')     //ssl key
            {
                if(Len == 0)
                {   //get
                    if(Test_SSLKeyLen == 0xffff)
                    {
                        proto_send("SK", "NONE", 4);
                    }
                    else
                    {
                        proto_send("SK", Test_SSLKey, Test_SSLKeyLen);
                    }
                    return 1;
                }
                else
                {   //set
                    ret = proto_flash_write(FLASH_KEY_OFFSET, Data, Len);
                    Test_SSLKey = (uint8_t *)proto_flash_read(FLASH_KEY_OFFSET, &Test_SSLKeyLen);
                    FREE_SSLKey();
                    Load_SSLKey();
                    #if DEBUG_PRINT
                    printf("Data[%d]:[%s]", Len, Data);
                    printf("SSL key flash write[%d]\r\n", ret);
                    print_buf_c(Test_SSLKey, Test_SSLKeyLen);
                    #endif
                    return 1;
                }
            }
            else
            {
                printf("CMD no match \r\n");
                return -1;
            }
            break;
        case 'T':
            if(*(CMD+1) == 'M')     //telemetry message send
            {
                if(Len == 0)
                {   //get
                    proto_send("TM", "NOT Message", strlen("NOT Message"));
                    return 1;
                }
                else
                {   //set
                    Temp_BuffLen =sprintf(Temp_Buff, "{\"DeviceID\": \"%s\", \"Message\": \"%s\",\"TIMES\":%ld,\"STATUS\":\"SUCCESS\"}", Test_DeviceID_str, Data, time_us_32());
                    telemetry_send(Temp_Buff);
                    #if DEBUG_PRINT
                    printf("Data[%d]:[%s]", Len, Data);
                    printf("telemetryData[%d]:%s\r\n", Temp_BuffLen, Temp_Buff);
                    #endif
                    return 1;
                }
            }
            else
            {
                printf("CMD no match \r\n");
                return -1;
            }
            break;
        default :
            printf("CMD no match \r\n");
            break;
    }
    return 0;
}
int proto_uart_process(void)
{
    /*if(uart_flag == 1)
    {
        proto_recv();
        uart_flag = 0;
    }*/
    
    uint64_t nowTime = time_us_64();

    if(proto_data.flag)
    {
        //CRC check
        uint16_t temp_CRC = 0;
        uint8_t CMD[2] = {0,};
        temp_CRC = crc16(proto_data.buffer+2, proto_data.len);
        #if DEBUG_PRINT
        printf("cal CRC 0x%04x in CRC 0x%02x%02x \r\n", temp_CRC, proto_data.buffer[proto_data.len + 2], proto_data.buffer[proto_data.len + 2 + 1]);
        #endif
        if(temp_CRC ==(((proto_data.buffer[proto_data.len + 2]<<8)&0xff00)|(proto_data.buffer[proto_data.len + 2 + 1]&0x00ff)))
        {
            #if DEBUG_PRINT
            printf("CRC success!! \r\n");
            #endif
        }
        else
        {
            #if DEBUG_PRINT
            printf("CRC fail!! \r\n");
            #endif
            proto_data.flag = 0;
            proto_data.index = 0;
            proto_data.seq = 0;
            return 0;
        }
        CMD[0] = proto_data.buffer[2];
        CMD[1] = proto_data.buffer[3];
        #if DEBUG_PRINT
        printf("CMD = %s \r\n", CMD);
        #endif
        //CMD parse -> CMD_process(CMD, DATA, LEN)
        //proto_data.buffer[proto_data.len + 2] = 0;
        proto_CMD_Process(CMD, proto_data.buffer + 4, proto_data.len - 2);
        proto_data.flag = 0;
        proto_data.index = 0;
        proto_data.seq = 0;
        
    }
    if(proto_data.seq > 0)
    {
        if((int64_t)(nowTime - proto_data.lastTime) > 3000000)
        {
            #if DEBUG_PRINT
            printf("time out %lld\r\n", nowTime - proto_data.lastTime);
            #endif
            proto_data.lastTime = nowTime;
            proto_data.seq = 0;
            proto_data.index = 0;
        }
    }
}



