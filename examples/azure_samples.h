
#ifndef _AZURE_SAMPLES_H_
#define _AZURE_SAMPLES_H_

#ifdef __cplusplus
extern "C"
{
#endif

extern const char pico_az_connectionString[];
extern const char pico_az_x509connectionString[];
extern const char pico_az_x509certificate[];
extern const char pico_az_x509privatekey[];
extern const char pico_az_id_scope[];
extern const char pico_az_COMMON_NAME[];
extern const char pico_az_CERTIFICATE[];
extern const char pico_az_PRIVATE_KEY[];

extern char *Arhis_DeviceID_str;
extern char *Arhis_SSLKey_str;
extern char *Arhis_Cert_str;
#if 0
extern static const char *COMMON_NAME;// = (const *)Arhis_DeviceID_str;
extern static const char *CERTIFICATE;// = (const *)Arhis_Cert_str;
extern static const char *PRIVATE_KEY;// = (const *)Arhis_SSLKey_str;
#endif

void iothub_ll_telemetry_sample(void);
void iothub_ll_c2d_sample(void);
void iothub_ll_client_x509_sample(void);
void prov_dev_client_ll_sample(void);

void prov_dev_client_ll_Connect(void);
void prov_dev_client_ll_Process(void);
void telemetry_send(char *buffer);


#ifdef __cplusplus
}
#endif

#endif /* _AZURE_SAMPLES_H_ */
