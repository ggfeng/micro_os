//
// Created by tomato on 17-7-18.
//

#define LOG_TAG "and_link"

#include <jni.h>
#include "utils/Log.h"
#include <cstdlib>
#include "Qlink_API.h"
#include <pthread.h>
#include <iostream>
#include <string>
#include <unistd.h>
#include <fstream>

static jclass cls = NULL;
static jobject obj = NULL;
static jmethodID callback = NULL;
static jmethodID regCallack = NULL;
static jmethodID onUnbindCallack = NULL;
static JavaVM* g_jvm = NULL;
#define MAX_RESP_DATA_LEN 512


char* jstringToChar(JNIEnv *env,jstring jstr)
{
    char* pStr = NULL;
    jclass jstrObj = env->FindClass("java/lang/String");
    jstring encode = env->NewStringUTF("utf-8");
    jmethodID methodId = env->GetMethodID(jstrObj,"getBytes","(Ljava/lang/String;)[B");
    jbyteArray byteArray = (jbyteArray)env->CallObjectMethod(jstr,methodId,encode);
    jsize strLen = env->GetArrayLength(byteArray);
    jbyte *jBuf = env->GetByteArrayElements(byteArray,JNI_FALSE);
    if(jBuf > 0)
    {
        pStr = (char*)malloc(strLen + 1);
        if(!pStr)
        {
            return NULL;
        }
        memcpy(pStr,jBuf,strLen);
        pStr[strLen] = 0;
    }

    env->ReleaseByteArrayElements(byteArray,jBuf,0);
    return pStr;
}

jstring strToJstringForLen(JNIEnv *env,const char* pStr,int len)
{
    int strLen = strlen(pStr);
    if(strLen > len){
        strLen = len;
    }
    jclass jstrObj = env->FindClass("java/lang/String");
    jmethodID methodId = env->GetMethodID(jstrObj,"<init>","([BLjava/lang/String;)V");
    jbyteArray byteArray = env->NewByteArray(strLen);
    jstring encode = env->NewStringUTF("utf-8");
    env->SetByteArrayRegion(byteArray,0,strLen,(jbyte*)pStr);
    return (jstring)env->NewObject(jstrObj,methodId,byteArray,encode);
}

jstring strToJstring(JNIEnv *env,const char* pStr)
{
    int strLen = strlen(pStr);
    jclass jstrObj = env->FindClass("java/lang/String");
    jmethodID methodId = env->GetMethodID(jstrObj,"<init>","([BLjava/lang/String;)V");
    jbyteArray byteArray = env->NewByteArray(strLen);
    jstring encode = env->NewStringUTF("utf-8");
    env->SetByteArrayRegion(byteArray,0,strLen,(jbyte*)pStr);
    return (jstring)env->NewObject(jstrObj,methodId,byteArray,encode);
}

int regRespCallback(JNIEnv *env,unsigned char *data,int len){
    int ret = 0;
    int i;

    ALOGI("regRespCallback: enter--->data is %s; len is: %d",data,len);
    if(len <= 0){
        ALOGE("regRespCallback: len error.");
        return -1;
    }

    if(regCallack == NULL){
        ALOGE("regRespCallback: callback is NULL");
        return -1;
    }

    if(env == NULL){
        ALOGE("regRespCallback: getRuntimeEnv error.");
        g_jvm->DetachCurrentThread();
        return -1;
    }
    if(cls == NULL){
        ALOGE("ReciveInternetChannelCallback: cls is NULL");
        return -1;
    }

    ALOGI("regRespCallback: before strToJstring--->.");
    jstring result = strToJstringForLen(env,(const char*)data,len);
    ALOGI("regRespCallback: call java method begin--->.");
    env->CallStaticIntMethod(cls,regCallack,result);
    ALOGI("regRespCallback: call java method end--->.");
    //regCallack = NULL;
    return ret;
}

int ReciveInternetChannelCallback(unsigned char *data, int len)
{
    int ret = 0;
    int i;
    JNIEnv *env;

    ALOGI("ReciveInternetChannelCallback: enter--->data is %s; len is: %d",data,len);
    if(len <= 0){
        ALOGE("ReciveInternetChannelCallback: len error.");
        return -1;
    }

    if(callback == NULL){
        ALOGE("ReciveInternetChannelCallback: callback is NULL");
        return -1;
    }
    if(cls == NULL){
        ALOGE("ReciveInternetChannelCallback: cls is NULL");
        return -1;
    }
    g_jvm->AttachCurrentThread(&env, NULL);
    if(env == NULL){
        ALOGE("ReciveInternetChannelCallback: getRuntimeEnv error.");
        g_jvm->DetachCurrentThread();
        return -1;
    }

    ALOGI("ReciveInternetChannelCallback: before strToJstring--->.");
    jstring result = strToJstringForLen(env,(const char*)data,len);
    ALOGI("ReciveInternetChannelCallback: call java method begin--->.");
    env->CallStaticIntMethod(cls,callback,result);
    ALOGI("ReciveInternetChannelCallback: call java method end--->.");
    g_jvm->DetachCurrentThread();

    return ret;
}

int onUnbindCallback(unsigned char *data, int len)
{
    int ret = 0;
    int i;
    JNIEnv *env;

    ALOGI("onUnbindCallback: enter--->data is %s; len is: %d",data,len);
    if(len <= 0){
        ALOGE("onUnbindCallback: len error.");
        return -1;
    }

    if(onUnbindCallack == NULL){
        ALOGE("onUnbindCallback: callback is NULL");
        return -1;
    }
    if(cls == NULL){
        ALOGE("onUnbindCallback: cls is NULL");
        return -1;
    }
    g_jvm->AttachCurrentThread(&env, NULL);
    if(env == NULL){
        ALOGE("onUnbindCallback: getRuntimeEnv error.");
        g_jvm->DetachCurrentThread();
        return -1;
    }

    ALOGI("onUnbindCallback: before strToJstring--->.");
    jstring result = strToJstringForLen(env,(const char*)data,len);
    ALOGI("onUnbindCallback: call java method begin--->.");
    env->CallStaticIntMethod(cls,onUnbindCallack,result);
    ALOGI("onUnbindCallback: call java method end--->.");
    g_jvm->DetachCurrentThread();

    return ret;
}

void *startThread(void* argc)
{
    ALOGI("startThread.");
    int ret = Qlink_StartCoapServer();
    ALOGI("Qlink_StartCoapServer result is %d.",ret);
    return ((void*)ret);
}

jstring qlink_get_version(JNIEnv *env, jobject thiz)
{
    jstring version;
    const char* data = Qlink_GetVersion();
    version = strToJstring(env,data);
    ALOGI("qlink_get_version data: %s",data);
    return version;
}

jstring qlink_get_hidden_ssid(JNIEnv *env, jobject thiz)
{
    jstring ssid;
    const char* data = Qlink_GetHiddenSSID();
    ssid = strToJstring(env,data);
    ALOGI("qlink_get_hidden_ssid data: %s",data);
    return ssid;
}

jstring qlink_get_hidden_ssid_pwd(JNIEnv *env, jobject thiz)
{
    jstring pwd;
    const char* data = Qlink_GetHiddenSSIDPWD();
    pwd = strToJstring(env,data);
    ALOGI("qlink_get_hidden_ssid_pwd data: %s",data);
    return pwd;
}

jstring qlink_find_gateway_ip(JNIEnv *env, jobject thiz)
{
    jstring ip;
    const char* data = Qlink_FindGatewayIP();
    ip = strToJstring(env,data);
    ALOGI("qlink_find_gateway_ip data: %s",data);
    return ip;
}

jint qlink_notify_gateway_join_boot(JNIEnv *env, jobject thiz,jstring deviceMac,jstring deviceType)
{
    char* device_mac = jstringToChar(env,deviceMac);
    char* device_type = jstringToChar(env,deviceType);
    int ret = Qlink_notifyGatewayJoinBoot(device_mac,device_type);
    ALOGI("Qlink_notifyGatewayJoinBoot deviceMac: %s",device_mac);
    ALOGI("Qlink_notifyGatewayJoinBoot device_type: %s",device_type);
    ALOGI("Qlink_notifyGatewayJoinBoot result: %d",ret);
    free(device_mac);
    free(device_type);
    return ret;
}

jint qlink_start_coap_server(JNIEnv *env, jobject thiz)
{
    pthread_t id;
    int i,ret;
    ret = pthread_create(&id,NULL,startThread,NULL);
    ALOGI("qlink_start_coap_server ret: %d",ret);
    if(ret != 0){
        return -1;
    }
    pthread_detach(pthread_self());
    return 0;
}

jint qlink_set_receive_internet_channel_callback(JNIEnv *env, jobject thiz)
{
    ALOGI("qlink_set_receive_internet_channel_callback");
    Qlink_setReciveInternetChannelCallback(ReciveInternetChannelCallback);
    return 0;
}


jint qlink_ack_qlink_netinfo(JNIEnv *env, jobject thiz,jstring deviceMac,jstring deviceType)
{
    char* device_mac = jstringToChar(env,deviceMac);
    char* device_type = jstringToChar(env,deviceType);
    int ret = Qlink_ackQlinkNetinfo(device_mac,device_type);
    ALOGI("qlink_ack_qlink_netinfo-->ret: %d",ret);
    free(device_mac);
    free(device_type);
    return ret;
}

jint qlink_register(JNIEnv *env,jobject thiz,jstring deviceMac,jstring deviceType,jstring productToken,jstring timeStamp){
    char* device_mac = jstringToChar(env,deviceMac);
    char* device_type = jstringToChar(env,deviceType);
    char* product_token = jstringToChar(env,productToken);
    char* time_stamp = jstringToChar(env,timeStamp);
    char response_data[MAX_RESP_DATA_LEN];
    int len = MAX_RESP_DATA_LEN;
    int ret = Qlink_Reg(device_mac,device_type,product_token,time_stamp,response_data,&len);
    regRespCallback(env,(unsigned char *)response_data,len);
    free(device_mac);
    free(device_type);
    free(product_token);
    free(time_stamp);
    return ret;
}

jint qlink_bootup(JNIEnv *env,jobject thiz,jstring deviceId,jstring deviceType,jstring firmwareVersion,jstring softwareVersion,jstring ipAddress,jstring timestamp){
    char* device_id = jstringToChar(env,deviceId);
    char* device_type = jstringToChar(env,deviceType);
    char* firmware_version = jstringToChar(env,firmwareVersion);
    char* software_version = jstringToChar(env,softwareVersion);
    char* ip_address = jstringToChar(env,ipAddress);
    char* time_stamp = jstringToChar(env,timestamp);
    int ret = Qlink_boot(device_id,device_type,firmware_version,software_version,ip_address,time_stamp);
    free(device_id);
    free(device_type);
    free(firmware_version);
    free(software_version);
    free(ip_address);
    free(time_stamp);
    return ret;
}

jint qlink_hearbeats(JNIEnv *env,jobject thiz,jstring deviceId){
    char* device_id = jstringToChar(env,deviceId);
    int ret = Qlink_Heartbeat(device_id);
    free(device_id);
    return ret;
}

static const JNINativeMethod gMethods[] = {
    { "qlinkGetVersion", "()Ljava/lang/String;", (void*)qlink_get_version },
    { "qlinkGetHiddenSSID", "()Ljava/lang/String;", (void *)qlink_get_hidden_ssid },
    { "qlinkGetHiddenSSIDPWD", "()Ljava/lang/String;",(void *)qlink_get_hidden_ssid_pwd },
    { "qlinkFindGatewayIP", "()Ljava/lang/String;", (void *)qlink_find_gateway_ip },
    { "qlinkNotifyGatewayJoinBoot","(Ljava/lang/String;Ljava/lang/String;)I", (void *)qlink_notify_gateway_join_boot },
    { "qlinkStartCoapServer",  "()I", (void *)qlink_start_coap_server },
    { "qlinkSetReciveInternetChannelCallback", "()I", (void *)qlink_set_receive_internet_channel_callback },
    { "qlinkAckQlinkNetinfo", "(Ljava/lang/String;Ljava/lang/String;)I", (void *)qlink_ack_qlink_netinfo },
    { "qlinkReg", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)I", (void *)qlink_register },
    { "qlinkBoot", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)I", (void *)qlink_bootup },
    { "qlinkHearBeat", "(Ljava/lang/String;)I", (void *)qlink_hearbeats },

};


int registerMethods(JNIEnv* env)
{
    const char* className = "com/rokid/andlink/RKAndlinkApi";
    cls = env->FindClass(className);
    if(cls == NULL)
    {
        ALOGE("registerMethods-->findClass failed");
        return -1;
    }
    if(env->RegisterNatives(cls,gMethods,sizeof(gMethods)/sizeof(gMethods[0])) != JNI_OK)
    {
        ALOGE("fail registerNative methods");
        return -1;
    }

    callback = env->GetStaticMethodID(cls,"receiveInternetChannelCall","(Ljava/lang/String;)I");
    regCallack = env->GetStaticMethodID(cls,"regRespCallback","(Ljava/lang/String;)I");
    onUnbindCallack = env->GetStaticMethodID(cls,"onUnbind","(Ljava/lang/String;)I");
    Qlink_setUnbindCallback(onUnbindCallback);
    return 0;
}

jint JNI_OnLoad(JavaVM* vm,void* reserved)
{
    JNIEnv* env;
    g_jvm = vm;

    if(vm->GetEnv((void**)&env,JNI_VERSION_1_4) != JNI_OK)
    {
        ALOGE("%s: JNI_OnLoad failed", "and_link");
        return -1;
    }
    if(env == NULL)
    {
        return -1;
    }
    if(registerMethods(env) != 0)
    {
        ALOGE("ERROR: native registration failed");
        return -1;
    }
    return JNI_VERSION_1_4;
}