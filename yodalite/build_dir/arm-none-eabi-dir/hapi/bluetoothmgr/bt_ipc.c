#include "common.h"


int report_bluetooth_information(int element, uint8_t *buf, int len) {
    bt_flora_send_msg(element, buf, len);
    return 0;
}

int method_report_reply(uint32_t msgtype, void *data, char *buf) {
    bt_flora_report_reply(msgtype, data, buf);
    return 0;
}

