                                                                                                                    #include "manage_process.h"

#include "hardware/gpio.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hardware/flash.h"
#include "hardware/adc.h"
#include "pico/stdlib.h"
#include "pico/critical_section.h"
#include "flashHandler.h"
#include "wizchip_conf.h"
#include "proto_pi_CMD.h"
#include "azure_samples.h"
#include "hardware/watchdog.h"
//#include "hardware/pwm.h"


#include "socket.h"


#define UDP_DEBUG 1
#define UDP_DEBUG_PORT 50001
#define UDP_DEBUG_DEST_PORT 50002

#define RPI_LGP_IN          2
#define LGP_OUT             3
#define TEST_SWITCH         4
#define RPI_RUN             5
#define PW_RPI              6
#define TEST_PIN            7
#define PICO_LED            25
#define PIN_ADC_1           26
#define PIN_ADC_2           27
#define ADC_conversion_factor       18.85/4096.0;
#define ADC_MAX             200




const uint8_t *flash_Network_contents =  (const uint8_t *) (XIP_BASE + FLASH_TARGET_OFFSET);
const uint8_t *flash_api_contents =      (const uint8_t *) (XIP_BASE + FLASH_API_OFFSET);
const uint8_t *flash_hostaddr_contents = (const uint8_t *) (XIP_BASE + FLASH_HOSTADDR_OFFSET);
const uint8_t *flash_token_contents =    (const uint8_t *) (XIP_BASE + FLASH_TOKEN_OFFSET);
const uint8_t *flash_uuid_contents =     (const uint8_t *) (XIP_BASE + FLASH_UUID_OFFSET);
const uint8_t *flash_deviceID_contents = (const uint8_t *) (XIP_BASE + FLASH_DEVICEID_OFFSET);
const uint8_t *flash_key_contents =      (const uint8_t *) (XIP_BASE + FLASH_KEY_OFFSET);
const uint8_t *flash_cert_contents =     (const uint8_t *) (XIP_BASE + FLASH_CERT_OFFSET);

uint8_t NetStatus = 0;

 uint8_t *Arhis_ApiKey = NULL;
 uint16_t Arhis_ApiKeyLen = 0;
 uint8_t *Arhis_HostAddr = NULL;
 uint16_t Arhis_HostAddrLen = 0;
 uint8_t *Arhis_Token = NULL;
 uint16_t Arhis_TokenLen = 0;
 uint8_t *Arhis_UUID = NULL;
 uint16_t Arhis_UUIDLen = 0;
 uint8_t *Arhis_DeviceID = NULL;
 uint16_t Arhis_DeviceIDLen = 0;
 char *Arhis_DeviceID_str = NULL;
 uint8_t *Arhis_SSLKey = NULL;
 uint16_t Arhis_SSLKeyLen = 0;
 char *Arhis_SSLKey_str = NULL;
 uint8_t *Arhis_Cert = NULL;
 uint16_t Arhis_CertLen = 0;
 char *Arhis_Cert_str = NULL;
 
uint8_t *Arhis_Config = NULL;
uint16_t Arhis_ConfigLen = 0;


uint32_t TestSwitchTime;
uint16_t TestSwitchModeTime;
uint32_t RPI_RunTime;
uint8_t LGP_DATA = 0, LGP_STATUS = 0, SWTestMode = 0, SWTestSendVolt_Flag =0;
uint8_t Timer_irq_flag = 0, SwSend_flag = 0;
uint16_t ADC_Timer_cnt = 0;
uint16_t regular_time = 0;

uint16_t ADC_DATA[2][ADC_MAX];
uint16_t ADC_Index[2];
float Volt_data[2];

uint8_t Debug_buff[4096];
uint16_t Debug_Len = 0;

uint8_t EmergencyReportSEQ = 0;
uint16_t EmergencyCnt =0; 
uint32_t RPI_resp_cnt = 0;
uint8_t RPI_Status = 0;

//static critical_section_t g_flash_cri_sec;
int32_t init_Debug_UDP(void)
{
    int32_t ret;
    if((ret = socket(UDP_DEBUG, Sn_MR_UDP, UDP_DEBUG_PORT, 0x00)) != UDP_DEBUG)
        return ret;
}

int32_t DebugSend(uint8_t *buf, uint16_t Len)
{
    int32_t ret;
    uint8_t UDP_BroadIP[4] = {255,255,255,255};
    if(NetStatus > 1)
    {
        ret=sendto(UDP_DEBUG, buf, Len, UDP_BroadIP, UDP_DEBUG_DEST_PORT);
    }
    return ret;
}


int Load_Flash_Internal_Data(void)
{
    Arhis_ApiKey = (uint8_t *)proto_flash_read(FLASH_API_OFFSET, &Arhis_ApiKeyLen);
    Arhis_HostAddr = (uint8_t *)proto_flash_read(FLASH_HOSTADDR_OFFSET, &Arhis_HostAddrLen);
    Arhis_Token = (uint8_t *)proto_flash_read(FLASH_TOKEN_OFFSET, &Arhis_TokenLen);
    Arhis_UUID = (uint8_t *)proto_flash_read(FLASH_UUID_OFFSET, &Arhis_UUIDLen);
    Arhis_DeviceID = (uint8_t *)proto_flash_read(FLASH_DEVICEID_OFFSET, &Arhis_DeviceIDLen);
    Arhis_SSLKey = (uint8_t *)proto_flash_read(FLASH_KEY_OFFSET, &Arhis_SSLKeyLen);
    Arhis_Cert = (uint8_t *)proto_flash_read(FLASH_CERT_OFFSET, &Arhis_CertLen);
    Arhis_Config = (uint8_t *)proto_flash_read(FLASH_RP_CONFIG_OFFSET, &Arhis_ConfigLen);
    Arhis_ConfigLen = Arhis_ConfigLen & 0x00FFFF;
    #if DEBUG_PRINT
    printf("apikeylen:%d, hostAddrLen:%d, TokenLen:%d, UUIDLen:%d, DeviceIDLen:%d, SSLKeyLen:%d, CertLen:%d , Arhis_Config:%d[0x%04x] \r\n",Arhis_ApiKeyLen,Arhis_HostAddrLen, Arhis_TokenLen, Arhis_UUIDLen, Arhis_DeviceIDLen, Arhis_SSLKeyLen, Arhis_CertLen, Arhis_ConfigLen , Arhis_ConfigLen);
    printf("Arhis_ApiKey = %x\r\n", FLASH_API_OFFSET);
    printf("Arhis_HostAddr = %x\r\n", FLASH_HOSTADDR_OFFSET);
    printf("Arhis_Token = %x\r\n", FLASH_TOKEN_OFFSET);
    printf("Arhis_UUID = %x\r\n", FLASH_UUID_OFFSET);
    printf("Arhis_DeviceID = %x\r\n", FLASH_DEVICEID_OFFSET);
    printf("Arhis_SSLKey = %x\r\n", FLASH_KEY_OFFSET);
    printf("Arhis_Cert = %x\r\n", FLASH_CERT_OFFSET);
    printf("Arhis_Config = %x\r\n", FLASH_RP_CONFIG_OFFSET);
    
    #endif
    #if DEBUG_UDP_EN
    Debug_Len = sprintf(Debug_buff, "apikeylen:%d, hostAddrLen:%d, TokenLen:%d, UUIDLen:%d, DeviceIDLen:%d, SSLKeyLen:%d, CertLen:%d config:%d, Arhis_Config:%X \r\n",Arhis_ApiKeyLen,Arhis_HostAddrLen, Arhis_TokenLen, Arhis_UUIDLen, Arhis_DeviceIDLen, Arhis_SSLKeyLen, Arhis_CertLen, Arhis_ConfigLen);
    DebugSend(Debug_buff, Debug_Len);
    if(Arhis_ApiKeyLen == 0xffff)
    {
        Debug_Len = sprintf(Debug_buff, "Arhis_ApiKey : NONE\r\n");
        DebugSend(Debug_buff, Debug_Len);
    }
    else
    {
        Debug_Len = sprintf(Debug_buff, "Arhis_ApiKey : %s\r\n",Arhis_ApiKey);
        DebugSend(Debug_buff, Debug_Len);
    }
    if(Arhis_HostAddrLen == 0xffff)
    {
        Debug_Len = sprintf(Debug_buff, "Arhis_HostAddr : NONE\r\n");
        DebugSend(Debug_buff, Debug_Len);
    }
    else
    {
        Debug_Len = sprintf(Debug_buff, "Arhis_HostAddr : %s\r\n",Arhis_HostAddr);
        DebugSend(Debug_buff, Debug_Len);
    }
    if(Arhis_TokenLen == 0xffff)
    {
        Debug_Len = sprintf(Debug_buff, "Arhis_Token : NONE\r\n");
        DebugSend(Debug_buff, Debug_Len);
    }
    else
    {
        Debug_Len = sprintf(Debug_buff, "Arhis_Token : %s\r\n",Arhis_Token);
        DebugSend(Debug_buff, Debug_Len);
    }
    if(Arhis_UUIDLen == 0xffff)
    {
        Debug_Len = sprintf(Debug_buff, "Arhis_UUID : NONE\r\n");
        DebugSend(Debug_buff, Debug_Len);
    }
    else
    {
        Debug_Len = sprintf(Debug_buff, "Arhis_UUID : %s\r\n",Arhis_UUID);
        DebugSend(Debug_buff, Debug_Len);
    }
    if(Arhis_DeviceIDLen == 0xffff)
    {
        Debug_Len = sprintf(Debug_buff, "Arhis_DeviceID : NONE\r\n");
        DebugSend(Debug_buff, Debug_Len);
    }
    else
    {
        Debug_Len = sprintf(Debug_buff, "Arhis_DeviceID : %s\r\n",Arhis_DeviceID);
        DebugSend(Debug_buff, Debug_Len);
    }
    printf("Arhis_ConfigLen = %d\r\n", Arhis_ConfigLen);
    #if 1
    if((Arhis_ConfigLen == 0xFFFF)||(Arhis_ConfigLen <= 0))
    {
        Debug_Len = sprintf(Debug_buff, "Arhis_Config : NONE\r\n");
        DebugSend(Debug_buff, Debug_Len);
    }
    else
    {
        Debug_Len = sprintf(Debug_buff, "Arhis_Config[%d] : %s\r\n",Arhis_ConfigLen, Arhis_Config);
        DebugSend(Debug_buff, Debug_Len);
    }
    #endif
    #endif
    #if AZURE_EN
    if(Load_DeviceID() < 0)
        return -2;
    if(Load_Cert() < 0)
        return -3;
    if(Load_SSLKey() < 0)
        return -4;
    #if 0
    Arhis_DeviceID_str = calloc(Arhis_DeviceIDLen + 1, sizeof(char));
    if(Arhis_DeviceID_str == NULL)
    {
        #if DEBUG_PRINT
        printf("Arhis_DeviceID_str not alocation [%ld]\r\n", time_us_32());
        #endif
        #if DEBUG_UDP_EN
        DebugSend("Arhis_DeviceID_str not alocation \r\n", strlen("Arhis_DeviceID_str not alocation \r\n"));
        #endif
        return -2;
    }
    memcpy(Arhis_DeviceID_str, Arhis_DeviceID, Arhis_DeviceIDLen);
    #if DEBUG_PRINT
    printf("Arhis_DeviceID_str[%d][%s]\r\n",Arhis_DeviceIDLen, Arhis_DeviceID_str);
    #endif
    Arhis_SSLKey_str = calloc(Arhis_SSLKeyLen + 1, sizeof(char));
    if(Arhis_SSLKey_str == NULL)
    {
        #if DEBUG_PRINT
        printf("Arhis_SSLKey_str not alocation [%ld]\r\n", time_us_32());
        #endif
        #if DEBUG_UDP_EN
        DebugSend("Arhis_SSLKey_str not alocation \r\n", strlen("Arhis_SSLKey_str not alocation \r\n"));
        #endif
        return -3;
    }
    memcpy(Arhis_SSLKey_str, Arhis_SSLKey, Arhis_SSLKeyLen);
    #if DEBUG_PRINT
    printf("Arhis_SSLKey_str[%d][%s]\r\n",Arhis_SSLKeyLen, Arhis_SSLKey_str);
    #endif
    Arhis_Cert_str = calloc(Arhis_CertLen + 1, sizeof(char));
    if(Arhis_Cert_str == NULL)
    {
        #if DEBUG_PRINT
        printf("Arhis_Cert_str not alocation [%ld]\r\n", time_us_32());
        #endif
        #if DEBUG_UDP_EN
        DebugSend("Arhis_Cert_str not alocation \r\n", strlen("Arhis_Cert_str not alocation \r\n"));
        #endif
        return -4;
    }
    memcpy(Arhis_Cert_str, Arhis_Cert, Arhis_CertLen);
    #if DEBUG_PRINT
    printf("Arhis_SSLKey_str[%d][%s]\r\n", Arhis_CertLen ,Arhis_Cert_str);
    #endif
    #endif
    #if DEBUG_UDP_EN
    DebugSend("Arhis_DeviceID_str : \r\n", strlen("Arhis_DeviceID_str : \r\n"));
    if(Arhis_DeviceIDLen > 0xff00)
    {
        DebugSend("NONE",4);
    }
    else
    {
        DebugSend(Arhis_DeviceID_str, Arhis_DeviceIDLen);
    }
    DebugSend("Arhis_SSLKey_str : \r\n", strlen("Arhis_SSLKey_str : \r\n"));
    if(Arhis_SSLKeyLen > 0xff00)
    {
        DebugSend("NONE",4);
    }
    else
    {
        DebugSend(Arhis_SSLKey_str, Arhis_SSLKeyLen);
    }
    DebugSend("Arhis_Cert_str : \r\n", strlen("Arhis_Cert_str : \r\n"));
    if(Arhis_CertLen > 0xff00)
    {
        DebugSend("NONE",4);
    }
    else
    {
        DebugSend(Arhis_Cert_str, Arhis_CertLen);
    }
    #endif
    #endif
}
int Load_DeviceID(void)
{
    Arhis_DeviceID_str = calloc(Arhis_DeviceIDLen + 1, sizeof(char));
    if(Arhis_DeviceID_str == NULL)
    {
        #if DEBUG_PRINT
        printf("Arhis_DeviceID_str not alocation [%ld]\r\n", time_us_32());
        #endif
        #if DEBUG_UDP_EN
        DebugSend("Arhis_DeviceID_str not alocation \r\n", strlen("Arhis_DeviceID_str not alocation \r\n"));
        #endif
        return -2;
    }
    memcpy(Arhis_DeviceID_str, Arhis_DeviceID, Arhis_DeviceIDLen);
    #if DEBUG_PRINT
    printf("Arhis_DeviceID_str[%d][%s]\r\n",Arhis_DeviceIDLen, Arhis_DeviceID_str);
    #endif
    return 0;
}
int Load_Cert(void)
{
    Arhis_Cert_str = calloc(Arhis_CertLen + 1, sizeof(char));
    if(Arhis_Cert_str == NULL)
    {
    #if DEBUG_PRINT
        printf("Arhis_Cert_str not alocation [%ld]\r\n", time_us_32());
    #endif
    #if DEBUG_UDP_EN
        DebugSend("Arhis_Cert_str not alocation \r\n", strlen("Arhis_Cert_str not alocation \r\n"));
    #endif
        return -4;
    }
    memcpy(Arhis_Cert_str, Arhis_Cert, Arhis_CertLen);
    #if DEBUG_PRINT
        printf("Arhis_SSLKey_str[%d][%s]\r\n", Arhis_CertLen ,Arhis_Cert_str);
    #endif

    return 0;
}

int Load_SSLKey(void)
{
    Arhis_SSLKey_str = calloc(Arhis_SSLKeyLen + 1, sizeof(char));
    if(Arhis_SSLKey_str == NULL)
    {
        #if DEBUG_PRINT
        printf("Arhis_SSLKey_str not alocation [%ld]\r\n", time_us_32());
        #endif
        #if DEBUG_UDP_EN
        DebugSend("Arhis_SSLKey_str not alocation \r\n", strlen("Arhis_SSLKey_str not alocation \r\n"));
        #endif
        return -3;
    }
    memcpy(Arhis_SSLKey_str, Arhis_SSLKey, Arhis_SSLKeyLen);
    #if DEBUG_PRINT
    printf("Arhis_SSLKey_str[%d][%s]\r\n",Arhis_SSLKeyLen, Arhis_SSLKey_str);
    #endif
    return 0;
}
int FREE_DeviceID(void)
{
    free(Arhis_DeviceID_str);
    return 0;
}

int FREE_Cert(void)
{
    free(Arhis_Cert_str);
    return 0;
}

int FREE_SSLKey(void)
{
    free(Arhis_SSLKey_str);
    return 0;
}




void print_buf(const uint8_t *buf, size_t len) {
    for (size_t i = 0; i < len; ++i) {
        printf("%02x", buf[i]);
        if (i % 16 == 15)
            printf("\n");
        else
            printf(" ");
    }
}

void print_buf_c(const uint8_t *buf, size_t len) {
    for (size_t i = 0; i < len; ++i) {
        printf("%c", buf[i]);
    }
    printf("\n");
}

const uint8_t *proto_flash_read(uint32_t f_addr, uint16_t *d_len)
{
    //uint8_t *temp_data = NULL;
    const uint8_t *flash_data_contents =  (const uint8_t *) (XIP_BASE + f_addr);
    uint16_t temp_len = ((flash_data_contents[0] << 8) & 0xff00)| (flash_data_contents[1] &0x00ff);
    #if 0
    temp_data = calloc(temp_len , sizeof(uint8_t));
    if(temp_data == NULL)
        return NULL;
    printf("read len = %d \r\n", temp_len);
    print_buf_c(flash_data_contents + 2, temp_len);
    memcpy(temp_data, flash_data_contents + 2, temp_len);
    #endif
    *d_len = (temp_len & 0x00FFFF);
    
    //printf("temp_len = 0x%04x [%d] d_len=0x%04x [%d]\r\n", temp_len, temp_len, *d_len, *d_len);
    //return temp_data;
    return (flash_data_contents + 2);
}
uint16_t proto_flash_write(uint32_t f_addr, uint8_t *Data, uint16_t d_len)
{
    uint8_t *temp_data = NULL;
    uint8_t *temp_p = NULL;
    uint16_t temp_len = d_len +2;
    temp_data = calloc(temp_len + 2 , sizeof(uint8_t));
    if(temp_data == NULL)
        return 0;
    temp_p = temp_data;
    *temp_p++ = (d_len >> 8) & 0x00ff;
    *temp_p++ = d_len & 0x00ff;
    memcpy(temp_p , Data, d_len);
    #if DEBUG_PRINT
    printf("tempdat= %s len(%d) 0x%x%x(%d),",temp_data+2,  d_len, *temp_data, *(temp_data +1), (*temp_data<<8) |(*(temp_data +1)&0x00ff));
    #endif
    print_buf_c(temp_data+2, d_len);
    write_flash(f_addr, temp_data, temp_len + 2);
    return temp_len;
}

void flash_test(void)
{
    uint8_t api_test[]="XIx8sEc7voMlwJPw7e1wzipF7yKHCar3fY6Jnezd9T0cie";
    uint16_t api_len = 0, rst = 0;
    uint8_t *api_data = NULL;

    api_len = strlen(api_test);
    flash_critical_section_init();
    printf("load flash data \r\n");
    print_buf_c(flash_api_contents, api_len);
    printf("\r\nsave flash data \r\n");
    //flash_range_program(FLASH_TARGET_OFFSET, api_test, api_len);
    rst = proto_flash_write(FLASH_API_OFFSET, api_test, api_len);
    printf("flash write = %d \r\n ", rst);
    api_data = (uint8_t *)proto_flash_read(FLASH_API_OFFSET, &api_len);
    if(api_data == NULL)
        printf("flash read fail \r\n");
    printf("data Len %d \r\n", api_len);
    print_buf_c(api_data, api_len);
    //free(api_data);
    #if 0
    write_flash(FLASH_TARGET_OFFSET, api_test, api_len);
    printf("load flash data \r\n");
    print_buf_c(flash_api_contents, api_len);
    #endif
}
#if 0
void flash_test1(void)
{
    uint8_t random_data[FLASH_PAGE_SIZE];
    for (int i = 0; i < FLASH_PAGE_SIZE; ++i)
        random_data[i] = rand() >> 16;
    
    critical_section_init(&g_flash_cri_sec);
    printf("Generated random data:\n");
    print_buf(random_data, FLASH_PAGE_SIZE);

    // Note that a whole number of sectors must be erased at a time.
    printf("\nsection_enter_blocking...\n");
    critical_section_enter_blocking(&g_flash_cri_sec);
    printf("\nErasing target region...\n");
    flash_range_erase(FLASH_TARGET_OFFSET, FLASH_SECTOR_SIZE);
    printf("\nsection_exit...\n");
    critical_section_exit(&g_flash_cri_sec);
    printf("Done. Read back target region:\n");
    print_buf(flash_target_contents, FLASH_PAGE_SIZE);

    printf("\nProgramming target region...\n");
    flash_range_program(FLASH_TARGET_OFFSET, random_data, FLASH_PAGE_SIZE);
    printf("Done. Read back target region:\n");
    print_buf(flash_target_contents, FLASH_PAGE_SIZE);

    bool mismatch = false;
    for (int i = 0; i < FLASH_PAGE_SIZE; ++i) {
        if (random_data[i] != flash_target_contents[i])
            mismatch = true;
    }
    if (mismatch)
        printf("Programming failed!\n");
    else
        printf("Programming successful!\n");
}
#endif

void RPI_RUN_callback(uint gpio, uint32_t events) {
    //printf("IRQ G %d %x:", gpio, events);
    uint32_t nowTime = time_us_32();
    switch(gpio)
    {
        case RPI_LGP_IN:
            if(events & 0x08) // RISE
            {
                LGP_STATUS = 0;
                LGP_DATA = 0;
                ctrl_LGP_OUT(0);
            }
            else // FALL 0x04
            {
                LGP_STATUS = 1;
            }
            //printf("LGP %d\r\n", LGP_STATUS);
            break;
        case TEST_SWITCH:
            //printf("T:%lld\n", nowTime - TestSwitchTime);
            if(events & 0x08)
            {
                if((nowTime - TestSwitchTime) > 3000000) //over 3seconds
                {
                    #if DEBUG_PRINT
                    printf("over 3s %ld \r\n" , nowTime - TestSwitchTime);
                    #endif
                    #if DEBUG_UDP_EN
                    Debug_Len = sprintf(Debug_buff, "over 3s %ld \r\n", nowTime - TestSwitchTime);
                    DebugSend(Debug_buff, Debug_Len);
                    #endif
                    SWTestMode = 1;
                    TestSwitchModeTime = 0;
                    SWTestSendVolt_Flag =1;
                }
                else
                {
                    #if DEBUG_PRINT
                    printf("under 3s %ld \r\n", nowTime - TestSwitchTime);
                    #endif
                    #if DEBUG_UDP_EN
                    Debug_Len = sprintf(Debug_buff, "under 3s %ld \r\n", nowTime - TestSwitchTime);
                    DebugSend(Debug_buff, Debug_Len);
                    #endif
                    SwSend_flag = 1;
                }
                
            }
            TestSwitchTime = nowTime;
            break;
        case RPI_RUN:
            //printf("T:%lld\n", nowTime - RPI_RunTime);
            RPI_RunTime = nowTime;
            break;
    }
    //printf("IRQ G %d %x: T:%lld\n", gpio, events, time_us_32());
}
#if 1
int64_t alarm_callback(alarm_id_t id, void *user_data)
{
    static uint8_t pico_LED_data = 0;
    #if 1
    if((LGP_STATUS == 1)||(SWTestMode == 1))
    {
        //LGP Action ON -> LGP DATA ON/OFF
        if(LGP_DATA == 1)
        {
            ctrl_LGP_OUT(1);
            LGP_DATA = 0;
        }
        else
        {
            ctrl_LGP_OUT(0);
            LGP_DATA = 1;
        }
        if(SWTestMode == 1)
        {
            TestSwitchModeTime++;
            if(TestSwitchModeTime > 119)  //1min -> 60s/0.5s = 120
            {
                SWTestMode = 0;
                TestSwitchModeTime = 0;
                LGP_DATA = 0;
                ctrl_LGP_OUT(0);
                #if DEBUG_UDP_EN
                Debug_Len = sprintf(Debug_buff, "swtestmode end & reset %ld %ld %ld\r\n", time_us_32(), TestSwitchTime, time_us_32() - TestSwitchTime);
                DebugSend(Debug_buff, Debug_Len);
                #endif
            }
        }
    }
    else
    {
        //LGP Action OFF -> LGP DATA OFF
    }
    if(pico_LED_data == 1)
    {
        gpio_put(PICO_LED, (bool)pico_LED_data);
        gpio_put(TEST_PIN, 1);
        pico_LED_data = 0;
    }
    else
    {
        gpio_put(PICO_LED, (bool)pico_LED_data);
        gpio_put(TEST_PIN, 0);
        pico_LED_data = 1;
    }
    #endif
    #if 0
    if(LGP_DATA == 1)
    {
        LGP_DATA = 0;
        ctrl_LGP_OUT(LGP_DATA);
    }
    else
    {
        LGP_DATA = 1;
        ctrl_LGP_OUT(LGP_DATA);
    }
    #endif
    //printf("Timer %d \n", (int) id);
    Timer_irq_flag = 1;
    ADC_Timer_cnt++;
    EmergencyCnt++;
    regular_time++;
    RPI_resp_cnt++;
    
    return 500000;
}
#endif
void init_GPIO_IRQ(void)
{
    uint8_t gpio_set_list[4] ={3, 6, 7, 25};
    uint16_t cnt = 0;
    //TEST_SWITCH
    gpio_init(TEST_SWITCH);
    gpio_set_dir(TEST_SWITCH, GPIO_IN);
    gpio_pull_up(TEST_SWITCH);
    gpio_set_irq_enabled_with_callback(TEST_SWITCH, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &RPI_RUN_callback);

    //RPI_LGP_IN
    gpio_init(RPI_LGP_IN);
    gpio_set_dir(RPI_LGP_IN, GPIO_IN);
    gpio_pull_up(RPI_LGP_IN);
    gpio_set_irq_enabled_with_callback(RPI_LGP_IN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &RPI_RUN_callback);

    //RPI_RUN
    gpio_init(RPI_RUN);
    gpio_set_dir(RPI_RUN, GPIO_IN);
    gpio_pull_up(RPI_RUN);
    gpio_set_irq_enabled_with_callback(RPI_RUN, GPIO_IRQ_EDGE_FALL, true, &RPI_RUN_callback);

    //gpio out init -> LGP_OUT, PW_RPI, PICO_LED
    for(cnt = 0; cnt < 4; cnt ++)
    {
        gpio_init(gpio_set_list[cnt]);
        gpio_set_dir(gpio_set_list[cnt], GPIO_OUT);
        gpio_put(gpio_set_list[cnt], 0);
    }
    //gpio_put(PW_RPI, 1);  // test 보드 상용 보드에서 제거해야함
    
}
void ctrl_LGP_OUT(uint8_t val)
{
    gpio_put(LGP_OUT, (bool)val);
}
void ctrl_PW_RPI(uint8_t val)
{
    gpio_put(PW_RPI, (bool)val);
}

void ctrl_PICO_LED(uint8_t val)
{
    gpio_put(PICO_LED, (bool)val);
}
#if 1
void init_TIMER_IRQ(void)
{
    add_alarm_in_ms(500, alarm_callback, NULL, false);
}
#endif


void init_ADC(void)
{
    /* ADC SET */
    adc_init();
    adc_gpio_init(PIN_ADC_1);
    adc_gpio_init(PIN_ADC_2);
    ADC_Index[0] = 0;
    ADC_Index[1] = 0;
}
float ADC_DATA_Conv(uint16_t ADC_DATA)
{
    return (float)ADC_DATA * ADC_conversion_factor;
}

float GET_ADC_DATA(uint8_t ADC_CH)
{
    //get adc value in queue 
    uint64_t total_data = 0;
    float ret_data = 0.0;
    uint16_t cnt;
    for(cnt = 0; cnt < ADC_Index[ADC_CH]; cnt ++)
    {
        total_data += ADC_DATA[ADC_CH][cnt];
    }
    ret_data = ADC_DATA_Conv((uint16_t)(total_data / ADC_Index[ADC_CH]));
    ADC_Index[ADC_CH] = 0;
    return ret_data;
}
float GET_Volt_DATA(uint8_t ADC_CH)
{
    return Volt_data[ADC_CH];
}

void ADC_Process(void)
{
    uint16_t adc_data[2] = {0, 0};
    uint16_t cnt = 0;
    for(cnt = 0; cnt < 2; cnt++)
    {
        adc_select_input(cnt); // ADC n
        if(ADC_Index[cnt] < 200)
        {
            ADC_DATA[cnt][ADC_Index[cnt]++] = adc_read();
        }
        
    }
}
uint8_t GET_EmergenSEQ(void)
{
    return EmergencyReportSEQ;
}
void SET_EmergenSEQ(uint8_t SEQ)
{
    EmergencyReportSEQ = SEQ;
}
uint8_t GET_LGP_STATUS(void)
{
    return LGP_STATUS;
}

void SET_LGP_STATUS(uint8_t data)
{
    LGP_STATUS = data;
}

void SET_RPI_STATUS(uint8_t data)
{
    RPI_Status  = data;
    if(data == 0)
        RPI_resp_cnt = 0;
}

uint8_t GET_RPI_STATUS(void)
{
    return RPI_Status;
}


uint16_t manage_process(void)
{
    uint32_t nowTime = time_us_32();
    uint8_t buff_data[512];
    uint16_t buff_len = 0;
    char telemetry_temp_buffer[512];
    
    if(Timer_irq_flag ==1)
    {
        Timer_irq_flag = 0;
        #if 1
        watchdog_update();
        #endif
        //RPI_RUN LED check
        if(((int32_t)(nowTime - RPI_RunTime) > 10000000)&&(EmergencyReportSEQ == 0))
        {
            //check RPI uart send & emergency report
            //RPI_RunTime = nowTime;
            #if DEBUG_UDP_EN
            Debug_Len = sprintf(Debug_buff, "rpi run fail [%ld]\r\n",nowTime - RPI_RunTime);
            DebugSend(Debug_buff, Debug_Len);
            #endif
            proto_send("ST", 0, 0);
            EmergencyReportSEQ = 1;
            EmergencyCnt=0;
        }
        else if((EmergencyReportSEQ >= 1)&&(EmergencyCnt > 19))  //0.5s * 20 = 10s
        {
            //azure send emergency message -> EmergencyReportSEQ ==1 :timeout;
            //EmergencyReportSEQ ==2 :success;
            //EmergencyReportSEQ ==3 :fail;
            #if DEBUG_UDP_EN
            Debug_Len = sprintf(Debug_buff, "rpi status [%d][%ld]\r\n",EmergencyReportSEQ, RPI_resp_cnt);
            DebugSend(Debug_buff, Debug_Len);
            #endif
            if((EmergencyReportSEQ == 1) ||(EmergencyReportSEQ == 3))
            {
                //emergency report send
                #if AZURE_EN
                sprintf(telemetry_temp_buffer, "{\"DeviceID\": \"%s\",\"Volt1\":%2.1f,\"Volt2\":%2.1f,\"STATUS\":\"FAIL\"}", Arhis_DeviceID_str, Volt_data[0], Volt_data[1]);
                telemetry_send(telemetry_temp_buffer);
                #endif
                if(RPI_Status == 0)
                {
                    RPI_Status = 1;
                    #if DEBUG_UDP_EN
                        Debug_Len = sprintf(Debug_buff, "RPI reset count start [%d][%ld]\r\n",RPI_Status, RPI_resp_cnt);
                        DebugSend(Debug_buff, Debug_Len);
                    #endif
                }
            }
            else    //recv success -> EmergencyReportSEQ == 2
            {
            }
            EmergencyReportSEQ = 0;
            RPI_RunTime = nowTime;
        } 
        else 
        {
        }
        //if(RPI_resp_cnt == 432000)  //24h * 60m * 60s *0.5  = 172,800
        if((RPI_Status == 1)&&(RPI_resp_cnt >= 172800))  //0h * 10m * 120 = 432,000
        {
            //reset RPI
            #if DEBUG_UDP_EN
                Debug_Len = sprintf(Debug_buff, "RPI ST TimeOut [%ld]\r\n",RPI_resp_cnt);
                DebugSend(Debug_buff, Debug_Len);
            #endif
            ctrl_PW_RPI(1);
            sleep_ms(3000);
            ctrl_PW_RPI(0);
            RPI_resp_cnt = 0;
            RPI_Status = 0;
        }
        else if(RPI_Status == 0)
        {
            RPI_resp_cnt = 0;
        }
        
        if(ADC_Timer_cnt > 119)     //1min send volt data
        {
            ADC_Timer_cnt = 0;
            Volt_data[0] = GET_ADC_DATA(0);
            Volt_data[1] = GET_ADC_DATA(1);
            buff_len = sprintf(buff_data, "1:%2.1f 2:%2.1f",Volt_data[0], Volt_data[0]);
            #if DEBUG_UDP_EN
            Debug_Len = sprintf(Debug_buff, "send adc [%d][%s]\r\n", buff_len, buff_data);
            DebugSend(Debug_buff, Debug_Len);
            #endif
            proto_send("VD", buff_data, buff_len);
        }
        else
        {
            if((ADC_Timer_cnt % 2) == 0)
            {
                ADC_Process();
            }
        }
        if(regular_time > 7119) //regular report 1hour
        {
            regular_time = 0;
            //regular report 1hour
            sprintf(telemetry_temp_buffer, "{\"DeviceID\": \"%s\",\"Volt1\":%2.2f,\"Volt2\":%2.2f,\"STATUS\":\"SUCCESS\"}", Arhis_DeviceID_str, Volt_data[0], Volt_data[1]);
            #if DEBUG_UDP_EN
            Debug_Len = sprintf(Debug_buff, "azure send [%d][%s]\r\n", strlen(telemetry_temp_buffer), telemetry_temp_buffer);
            DebugSend(Debug_buff, Debug_Len);
            #endif
            #if AZURE_EN
            telemetry_send(telemetry_temp_buffer);
            #endif
        }
    }
    if(SWTestSendVolt_Flag == 1)
    {
        SWTestSendVolt_Flag = 0;
        buff_len = sprintf(buff_data, "1:%2.2f 2:%2.2f",Volt_data[0], Volt_data[1]);
        proto_send("VD", buff_data, buff_len);
        #if DEBUG_UDP_EN
        Debug_Len = sprintf(Debug_buff, "sw mode test[%d][%s]\r\n", buff_len, buff_data);
        DebugSend(Debug_buff, Debug_Len);
        #endif
    }
    if(SwSend_flag == 1)
    {
        SwSend_flag = 0;
        #if DEBUG_UDP_EN
        DebugSend("SW send flag \r\n", strlen("SW send flag \r\n"));
        #endif
        proto_send("SW", 0, 0);
    }
    

    
    return 0;
}

