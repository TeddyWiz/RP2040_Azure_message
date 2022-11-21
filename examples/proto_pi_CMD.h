
#ifndef _PROTO_PI_CMD_H_
#define _PROTO_PI_CMD_H_

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct serial_data_t
{
    int index;
    int len;
    char buffer[4200];
    char seq;
    char flag;
    uint64_t lastTime;
}serial_data;

//
void proto_uart_init(void);
int W5100s_ping(void);
int proto_uart_process(void);
int proto_send(uint8_t *CMD, uint8_t *Data, uint16_t Len);


#ifdef __cplusplus
}
#endif

#endif


