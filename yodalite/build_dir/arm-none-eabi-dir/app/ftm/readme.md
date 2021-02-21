                    factory tool 使用说明

系统启动后进入命令行 运行 ftm 进入工厂模式：
AT+Sn?               read serial  number
AT+Sn=sn             write serial number 
AT+Seed?             read Seed 
AT+Seed=seed         write Seed
AT+Id?               read devicetypeid
AT+Id=devicetypeid   write devicetypeid
exit                 退出工厂模式

sample:

yodalite#ftm
Usage:
      AT+Sn?
      AT+Sn=sn
      AT+Seed?
      AT+Seed=seed
      AT+Id?
      AT+Id=devicetypeid
      exit

ftm#AT+Sn=0602041822000162
OK
ftm#AT+Seed=SN0602041822SN0602041822710162
OK
ftm#AT+Id=060F941561F24278B8ED71733D7B9507
OK
ftm#AT+Sn?
AT+Sn=0602041822000162
ftm#AT+Seed?
AT+Seed=SN0602041822SN0602041822710162
ftm#AT+Id?
AT+Id=060F941561F24278B8ED71733D7B9507
ftm#exit
yodalite#


