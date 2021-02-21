#ifndef QLINK_API_C_H_
#define QLINK_API_C_H_
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#if __cplusplus
extern "C"
{
#endif
typedef int (*onReceiveResponse)(unsigned char* body, int len);
typedef int (*onUnbind)(unsigned char* body, int len);
const char* Qlink_GetVersion();

//返回隐藏wifi的ssid
const char* Qlink_GetHiddenSSID();

//获取隐藏SSID的密码
const char* Qlink_GetHiddenSSIDPWD();

// 返回网关的IP地址
const char* Qlink_FindGatewayIP();

/*加入隐藏ssid后(ssid:CMCC-QLINK pwd:sD5fH%9k),调用此方法*/
int Qlink_notifyGatewayJoinBoot(const char* deviceMac, const char* deviceType);

//消息接收服务接口
int Qlink_StartCoapServer();

//设置上网通道的回调
void Qlink_setReciveInternetChannelCallback(onReceiveResponse _callback);

//应答网关的请求 
int Qlink_ackQlinkNetinfo(const char* deviceMac, const char* deviceType);

//注册接口
int Qlink_Reg(const char* deviceMac,const char* deviceType,const char* productToken,const char* timestamp, char *reponsedata, int *len);

//上线接口
int Qlink_boot(const char* deviceId,const char* deviceType,const char* firmwareVersion,const char* softwareVersion, const char* ipAddress, const char* timestamp);

//心跳接口
int Qlink_Heartbeat(const char* deviceId);

//设备主动上报的其他信息接口
int Qlink_notify_data(const char* deviceId,const char* eventType,const char* timestamp,const char* firmwareVersion,const char* softwareVersion);

//发送原始的coap请求接口
int Qlink_SendCaopRequest(const char* coapurl, const char *option, const char *data, char ** response, int *len);

//设置解绑回调
void Qlink_setUnbindCallback(onUnbind allback);

//云网关注册成功通知
int Qlink_Broadreg(const char* broadcast, const char* deviceId, const char* deviceType);
//联网成功广播
int Qlink_BroadNetinfo(const char* broadcast, const char* deviceMac, const char* deviceType);
#if __cplusplus
}
#endif

#endif /*QLINK_API_C_H_*/
