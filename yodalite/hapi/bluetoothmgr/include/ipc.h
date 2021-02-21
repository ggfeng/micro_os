#ifndef __BT_SERVICE_IPC_H_
#define __BT_SERVICE_IPC_H_

#ifdef __cplusplus
extern "C" {
#endif
int report_bluetooth_information(int element, uint8_t *buf, int len);
int method_report_reply(uint32_t msgtype, void *data, char *buf);
void bt_flora_send_msg(int element, uint8_t *buf, int len);
void bt_flora_report_reply(uint32_t msgtype, void *data, char *buf);

#ifdef __cplusplus
}
#endif
#endif
