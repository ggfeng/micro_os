# rokid bluetooth service

<!-- markdown-toc start - Don't edit this section. Run M-x markdown-toc-refresh-toc -->
**Table of Contents**

- [rokid bluetooth service](#rokid-bluetooth-service)
    - [软件接口协议](#软件接口协议)
        - [flora](#flora)
            - [BLE](#ble)
                - [控制命令](#控制命令)
                - [ble 状态](#ble-状态)
                - [ble 数据](#ble-数据)
            - [A2DP_SINK](#a2dpsink)
                - [控制命令](#控制命令-1)
                - [状态信息](#状态信息)
                - [音量](#音量)
            - [HFP](#HFP)
                - [控制命令](#控制命令)
                - [状态命令](#状态命令)
            - [A2DP_SOURCE](#a2dpsource)
                - [控制命令](#控制命令-2)
                - [状态命令](#状态命令)
    - [调试方法](#调试方法)
        - [bt_monitor](#btmonitor)
        - [bt_service_test](#btservicetest)

<!-- markdown-toc end -->


> 目前主流的蓝牙栈包含 bsa 和 bluez 两种控制方法。rokid 提供了 bluetooth_service 来兼容这两种底层蓝牙控制协议。

## 软件接口协议
bluetooth_service 目前为了和别的应用解耦，使用 pub/sub 的设计框架，通过 IPC (进程间通讯)控制蓝牙和解析蓝牙状态。

### flora
flora 是 rokid 体统的进程间 pub/sub 设计框架，bluetooth_service 为每一种协议都提供了响应的控制接口和状态接口。具体 ipc 节点如下：
可参考demo/bluetooth_test.c

 * ble 控制 path : **bluetooth.ble.command**
 * ble 状态 path : **bluetooth.ble.event**
 * a2dpsink 控制 path : **bluetooth.a2dpsink.command**
 * a2dpsink 状态 path : **bluetooth.a2dpsink.event**
 * a2dpsource 控制 path : **bluetooth.a2dpsource.command**
 * a2dpsource 状态 path : **bluetooth.a2dpsource.event**
 * hfp 控制 path : **bluetooth.hfp.command**
 * hfp 状态 path : **bluetooth.hfp.event**

#### BLE

##### 控制命令
1. 打开 ble

``` json
{ "proto": "ROKID_BLE", "command": "ON", "name": "xxxxxx" }
```

>如果需要自动关闭非 ble 的 profile，可增加 unique 字段

```json
 { "proto": "ROKID_BLE", "command": "ON", "name": "xxxxxx", "unique": true } 
```

2.关闭 ble

``` json
 { "proto": "ROKID_BLE", "command": "OFF" }
```

##### ble 状态

``` json
{ "state": "opened" }
{ "state": "closed" }
{ "state": "connected" }
{ "state": "hardshaked" }
{ "state": "disconected" }
```

##### ble 数据
 1. rokid ble 配网协议
bluetooth_service 目前对 rokid 配网 sdk 的协议进行了封装，所以里面包含 rokid 的配网私有协议。目前不支持 ble 裸流的处理，所以 ble 配网必须使用 rokid 的配网sdk。具体配网协议详情请见附件（rokid 配网协议.pdf）。

 2. 发送数据
 
``` json
 { "proto": "ROKID_BLE", "data": "xxxx" }
```
>发送raw数据（没有rokid msgid封装）
 
``` json
 { "proto": "ROKID_BLE", "rawdata": "xxxx" }
```

3.接收数据
 
``` json
 { "data": "xxxx" }
```

#### A2DP_SINK
##### 控制命令
 1. 打开A2dpsink

``` json
 { "proto": "A2DPSINK", "command": "ON", "name": "xxxxxx"}
```

>如果需要自动关闭非 a2dpsink 的 profile，可增加 unique 字段;（不会自动关闭hfp）

``` json
 { "proto": "A2DPSINK", "command": "ON", "name": "xxxxxx", "unique": 1}
```

>如果需要同时打开蓝牙电话，可以增加 "sec_pro": "HFP"

``` json
 { "proto": "A2DPSINK", "command": "ON", "name": "xxxxxx", "sec_pro": "HFP"}
```

>如果需要自定义打开时，是否根据历史记录自动重连，蓝牙是否可发现，可设置auto_connect discoverable属性, 缺省默认均为true

``` json
 { "proto": "A2DPSINK", "command": "ON", "name": "xxxxxx", "unique": true, "auto_connect": false, "discoverable": false}
```

 2.打开A2dpsink，连接后，做 sub 操作
 
``` json
 { "proto": "A2DPSINK", "command": "ON", "name": "xxxxxx", "subquent": "PLAY" }
```
``` json
 { "proto": "A2DPSINK", "command": "ON", "name": "xxxxxx", "subquent": "PAUSE" }
```
``` json
 { "proto": "A2DPSINK", "command": "ON", "name": "xxxxxx", "subquent": "STOP" }
```
 3.关闭a2dpsink
 
``` json
 { "proto": "A2DPSINK", "command": "OFF"}
```

>同时关闭蓝牙HFP

``` json
 { "proto": "A2DPSINK", "command": "OFF"，"sec_pro": "HFP"}
```

 4.播放音乐

``` json
{ "proto": "A2DPSINK", "command": "PLAY"}
```

 5.暂停音乐

``` json
{ "proto": "A2DPSINK", "command": "PAUSE"}
```

 6.停止音乐

``` json
{ "proto": "A2DPSINK", "command": "STOP"}
```
 7.下一首

``` json
{ "proto": "A2DPSINK", "command": "NEXT"}
```
 8.上一首

``` json
{ "proto": "A2DPSINK", "command": "PREV"}
```
 9.断开设备

``` json
{ "proto": "A2DPSINK", "command": "DISCONNECT"}
DISCONNECT: 会断开a2dp sink和hfp的连接
```

``` json
{ "proto": "A2DPSINK", "command": "DISCONNECT_PEER"}
DISCONNECT_PEER ：只会断开a2dp sink profile连接
```

 10.静音
 >主要应用场景，是激活唤醒时，需要立即暂停蓝牙音乐的输出，但pause命令实现延迟较高，mute则会立即生效。mute设备为若琪，并非手机

``` json
{ "proto": "A2DPSINK", "command": "MUTE"}
```
 11.取消静音

``` json
{ "proto": "A2DPSINK", "command": "UNMUTE"}
```

 12.暂停+静音

``` json
{ "proto": "A2DPSINK", "command": "PAUSE_MUTE"}
```
 13.播放+取消静音

``` json
{ "proto": "A2DPSINK", "command": "PLAY_UNMUTE"}
```
14.获取播放歌曲信息

``` json
{ "proto": "A2DPSINK", "command": "GETSONG_ATTRS"}
```

15.连接

``` json
{ "proto": "A2DPSINK", "command": "CONNECT", "address": "xx:xx:xx:xx:xx:xx", "name": "xxx" }
```
16.设置蓝牙可发现

``` json
{ "proto": "A2DPSINK", "command": "VISIBILITY","discoverable": true}
```
17.获取绑定设备列表

``` json
{ "proto": "A2DPSINK", "command": "PAIREDLIST" }
```

##### 状态信息
>action: stateupdate 
 a2dpstate： a2dp打开状态
 connect_state: 连接状态
 play_state：播放状态
 broadcast_state：蓝牙可发现状态
 linknum：打开蓝牙时，自动尝试连接设备个数，最大2。
 
 1. 打开A2dpsink后，open 正常

``` json
{ "action": "stateupdate", "a2dpstate": "opened", "connect_state": "invailed", "play_state": "invalid", "broadcast_state": "opened", "linknum": xx }
```
 2.打开A2dpsink后，open 异常

``` json
{ "action": "stateupdate",  "a2dpstate": "open failed", "connect_state": "invailed", "play_state": "invalid", "broadcast_state": "invalid"}
```
 3.设备已连接

``` json
{ "action": "stateupdate", "a2dpstate": "opened", "connect_state": "connected", "connect_name": "*******", "play_state": "invalid", "broadcast_state": "closed" }
```
 4.设备断开连接

``` json
{ "action": "stateupdate", "a2dpstate": "opened", "connect_state": "disconnected", "play_state": "invailed",  "broadcast_state": "opened"}
```
 5.设备播放音乐

``` json
{ "action": "stateupdate", "a2dpstate": "opened", "connect_state": "connected", "connect_name": "******", "play_state": "played", "broadcast_state": "closed"}
```
 6.暂停停止播放音乐

``` json
{ "action" : "stateupdate", "a2dpstate": "opened", "connect_state": "connected", "connect_name": "******", "play_state": "stoped", "boardcast_state": "closed" } 
```
7.播放歌曲信息

``` json
{ "action": "element_attrs", "title": "That Girl", "artist": "Olly Murs", "album": "24 HRS (Deluxe)", "track_num": "19", "track_nums": "25", "genre": "", "time": "176819" }
依次表示：titel:歌曲名；artist:艺术家；album:专辑；track_num:播放列表中第几首。track_nums：播放列表歌曲总数；genre：歌曲风格。time：歌曲总时间，单位ms
```
8.获取配对设备列表

``` json
{ "action": "pairedList", "results": { "deviceList": [ { "address": "f4:f5:db:cb:59:6c", "name": "小米手机ZZ" }, { "address": "78:4f:43:90:15:7d", "name": "sweet的MacBook Pro" }, { "address": "50:01:d9:38:2e:f2", "name": "Sweet" }, { "address": "50:01:d9:88:97:28", "name": "zhuxinyu" }, { "address": "84:a1:34:0a:61:41", "name": "sophos" } ] } }
```


##### 音量
蓝牙服务不涉及系统音量的更改，需要系统 core 程序对系统音量进行调整。部分蓝牙 source 设备直接通过修改自身的音频流进行控制，此时不需要 core 对应更改。另外一部分蓝牙设备会发送蓝牙音量，core 程序需要关注，进行匹配。

 1. BT audio stream init OK 此时可通过pulseaudio来初始化本地蓝牙音量

``` json
{ "action" : "volumechange" }
```
 2.BT audio volume change

``` json
{ "action" : "volumechange", "value" : 65 }
```

3.设置Absolutely volume同步给remote source设备
>需要手机支持，不支持则命令设置无效，手机不会响应

``` json
{ "proto": "A2DPSINK", "command": "VOLUME", "value": 100 }
```

#### HFP
**HFP建议和a2dp sink结合一起使用。
目前bluez协议栈暂不支持HFP。
硬件必须使用PCM（蓝牙SCO）接口通讯蓝牙语音数据。**
##### 控制命令
 1. 打开HFP

``` json
 { "proto": "HFP", "command": "ON", "name": "xxxxxx"}
```
>A2DPSINK unique: true 不会关闭HFP，但BLE/A2DP SOURCE，unique on 会关闭HFP

 2.关闭HFP

``` json
 { "proto": "HFP", "command": "OFF"}
```
 3.断开设备

``` json
{ "proto": "HFP", "command": "DISCONNECT"}
```
``` json
{ "proto": "HFP", "command": "DISCONNECT_PEER"}
```

 4.拨号

``` json
{ "proto": "HFP", "command": "DIALING", "NUMBER": "10086"}
```
 5.接听

``` json
{ "proto": "HFP", "command": "ANSWERCALL"}
```
 6.挂断

``` json
{ "proto": "HFP", "command": "HANGUP"}
```

##### 状态信息

 1. "action": "stateupdate"

``` json
{ "action": "stateupdate", "hfpstate": "opened", "connect_state": "connected", "connect_name": "******", "service": "active", "call": "inactive", "setup": "incoming", "held": "none","audio": "on" }
```
>hfpstate: HFP profile打开状态。可能值：opened, open failed, closing, closed, invalid

>connect_state: HFP连接状态。可能值：connecting, connected, connect failed, disconnecting, disconnected, invalid

>service: 手机网络状态。可能值：inactive, active
active表示手机可拨打电话或接听电话

>call: 通话状态。可能值：inactive, active
active表示手机已经处于通话接通状态。

>setup: 电话处理状态。可能值及意义：none：不需要任何处理（或处理已经完毕），incoming:手机来电状态。outgoing:手机正在拨号。alerting: 手机A拨打B的手机号，B的手机接收到被拨打。（即outgoing拨号时，呼叫的手机号码有效，且被呼叫号码手机响应了呼叫)。

>held: 通话保持状态。可能值：none: 没有被保持电话, hold_active：手机有一个active通话，并有一个保持电话, hold：手机通话被保持

>audio: HFP audio状态，表示当前是否有蓝牙电话音频流输入输出。可能值：on, off. 
手机端可以主动选择使用蓝牙，听筒，外放(免提)来接听、拨打电话，包括通话中也能切换

>上述例子表示：hfp打开成功，且已连接手机设备，手机网络正常，处于来电状态，没有接通，蓝牙音频流已开启，此时用户可以选择接通，挂断。

 2.来电铃声
 
``` json
 { "action": "ring", "audio": "off" }
```
>需要根据audio状态，确认是否需要播放本地来电铃声。
>audio: on 表示蓝牙音频已建立，手机已通过蓝牙发送了来电铃声
>audio: off 表示蓝牙音频并未建立，需要本地去播放来电铃声。
>此消息可能会上报多次
>如接通电话，请及时关闭本地播放的来电铃声

#### A2DP_SOURCE
##### 控制命令

 1. 打开A2dpsource

``` json
 { "proto": "A2DPSOURCE", "command": "ON", "name": "xxxxxx"}
```
>如果需要自动关闭非 a2dpsource 的 profile，需要增加 unique 字段;

``` json
 { "proto": "A2DPSOURCE", "command": "ON", "name": "xxxxxx", "unique": true}
```
2.关闭A2dpsource

``` json
 { "proto": "A2DPSOURCE", "command": "OFF"}
```
 3.连接设备

``` json
{ "proto": "A2DPSOURCE", "command": "CONNECT", "address": "xx:xx:xx:xx:xx:xx", "name": "xxxx" }
```
 4.扫描

``` json
{ "proto": "A2DPSOURCE", "command": "DISCOVERY" }
```
 5.断开设备

``` json
{ "proto": "A2DPSOURCE", "command": "DISCONNECT"}
```
``` json
{ "proto": "A2DPSOURCE", "command": "DISCONNECT_PEER"}
```

##### 状态命令
 1. 打开A2dpsource后，open 正常

``` json
{ "action": "stateupdate", "a2dpstate": "opened", "connect_state": "invailed", "broadcast_state": "opened", "linknum": xx }
```
 2.打开A2dpsource后，open 异常

``` json
{ "action": "stateupdate",  "a2dpstate": "open failed", "connect_state": "invailed", "broadcast_state": "invalid"}
```
 3.设备连接后

``` json
{ "action": "stateupdate", "a2dpstate": "opened", "connect_state": "connected", "connect_name": "*******", "broadcast_state": "closed" }
```
 4.断开连接后

``` json
{ "action": "stateupdate", "a2dpstate": "opened", "connect_state": "disconnected", "broadcast_state": "opened"}
```
 5.获取扫描结果

``` json
{ "action": "discovery", "results": { "deviceList": [ { "address": "b4:0b:44:73:88:3c", "name": "大船真帅" }, { "address": "6c:21:a2:2e:5e:1a", "name": "Rokid-Me-9999zz" }, { "address": "f4:60:e2:2e:da:09", "name": "小米手机" } ] } }
```

## 调试方法

> bluetooth 代码仓库位于*frameworks/native/services/bluetooth_service*。在仓库里面的 demo 目录下面有两个工具。一个是 bt_monitor(可以抓取蓝牙的所有通讯节点的数据) 和一个 bt_service_test(可以模拟第三方应用通过命令去操作蓝牙单项功能)。运行效果如下：

### bt_monitor

``` shell
/ # bt_monitor
command recv data:: { "name": "Rokid-Pebble-9999zz", "command": "ON", "proto": "ROKID_BLE" }
ble len ;:72
ble recv data:: { "state": "opened" }
ble len ;:21
command recv data:: { "command": "OFF", "proto": "ROKID_BLE" }
ble len ;:42
ble recv data:: { "state": "closed" }
ble len ;:21

```

### bt_service_test
``` shell
/ # bt_service_test
Bluetooth test CMD menu:
    1 ble
    2 a2dp_sink
    99 exit
Select cmd => 1
Bluetooth ble test CMD menu:
    1 ble on
    2 ble off
    3 send data
    4 send raw data
    99 exit ble
Select cmd => 1
Bluetooth ble test CMD menu:
    1 ble on
    2 ble off
    3 send data
    4 send raw data
    99 exit ble
Select cmd => 
ble len ;:21
2
Bluetooth ble test CMD menu:
    1 ble on
    2 ble off
    3 send data
    99 exit ble
Select cmd => 
ble len ;:21
```


