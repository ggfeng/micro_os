#ifndef _BTMGR_H_
#define _BTMGR_H_

/*BT FLORA EVT*/
#define FLORA_BT_COMMON_EVT "bluetooth.common.event"
#define FLORA_BT_BLE_EVT "bluetooth.ble.event"
#define FLORA_BT_A2DPSINK_EVT "bluetooth.a2dpsink.event"
#define FLORA_BT_A2DPSOURCE_EVT "bluetooth.a2dpsource.event"
#define FLORA_BT_HFP_EVT "bluetooth.hfp.event"

/*BT FLORA CMD*/
#define FLORA_BT_COMMON_CMD "bluetooth.common.command"
#define FLORA_BT_BLE_CMD "bluetooth.ble.command"
#define FLORA_BT_A2DPSINK_CMD "bluetooth.a2dpsink.command"
#define FLORA_BT_A2DPSOURCE_CMD "bluetooth.a2dpsource.command"
#define FLORA_BT_HFP_CMD "bluetooth.hfp.command"

#ifndef FLORA_MODULE_SLEEP
#define FLORA_MODULE_SLEEP "module.sleep"
#endif

#ifndef FLORA_POWER_SLEEP
#define FLORA_POWER_SLEEP "power.sleep"
#endif

#ifndef FLORA_VOICE_COMING
#define FLORA_VOICE_COMING  "rokid.voice_coming"
#endif

#ifndef FLORA_VOICE_CAPTURE
#define FLORA_VOICE_CAPTURE "rokid.voice_capture"
#endif

#ifndef FLORA_VOICE_END
#define FLORA_VOICE_END     "rokid.voice_end"
#endif

#define FLORA_AGET_BTMGR_URI "unix:/var/run/flora.sock#bluetoothservice-agent"

extern int btmgr_init(void);
extern int btmgr_deinit(void);

#endif
