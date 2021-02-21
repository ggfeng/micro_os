#include "data.h"
#include "usbd_audio.h"
#include "stm32flash.h"
I2C_HandleTypeDef hi2c1;
Date_File	data_file;
cmd_tbl	At_cmd;
#define CMD_Max	20
static unsigned short int list=0;
unsigned char StringA_B(char *dataA,char *dataB,unsigned char size);
unsigned char Choice_string(char *datain,char *dataout,unsigned char num);
unsigned char cmd_splicing(char *data,unsigned short int num );
unsigned char cmdA_AND_b(char *dataA,char *dataB,unsigned char size);
unsigned char XOR(unsigned char *dat,unsigned char size);

void delay(unsigned short int t)
{
	unsigned char i;
	for(i=0;i<121;i++)
		for(;t>0;t--)
		{
			
		}
}
int  Camera(int flag,char *  argv[])
{
	if(StringA_B("OPEN",argv[0],4))
	{
		CAMERA_H;
		HID_Sent("Camera is open!\n",16);
	}else if(StringA_B("CLOSE",argv[0],5))
	{
		CAMERA_L;
		HID_Sent("Camera is close!\n",17);
	}else
	{
		HID_Sent("Camera do not have this status!\n",31);
		//error 
	}
	return 1;
}


int ReadLight(int flag,char *  argv[])
{	
	unsigned short int light=0;
	unsigned char num;
	char data[20]="AT+Light=\n";
	if(Periphery_State.Lsensor==EXIST)
	{
		light=hid_data.light_data[1];
		light|=hid_data.light_data[0]<<8;		
		num=cmd_splicing(&data[0],light);
		HID_Sent(&data[0],num);
	}else
	{
		HID_Sent("This one doesn't have Light sensor \n",64);
	}
	
	return 0;
}
int ReadPS(int flag,char *  argv[])
{
	unsigned char num;
	
	unsigned short int Ps=0;
	char data[20]="AT+Ps=\n";
	
	if(Periphery_State.Lsensor==EXIST)
	{
		Ps=psensor_readPS();
		num=cmd_splicing(&data[0],Ps);
		HID_Sent(&data[0],num);
	}else
	{
		HID_Sent("This one doesn't have P sensor \n",64);
	}
	
	return 0;
}

int ReadALS(int flag,char *  argv[])
{
	unsigned char Als_data;
	unsigned char num;
	char data[20]="AT+Als=\n";
	if(Periphery_State.Psensor==EXIST)
	{
		Als_data=psensor_readALS();
		num=cmd_splicing(&data[0],Als_data);
		HID_Sent(&data[0],num);
	}else
	{
		HID_Sent("This one doesn't have P sensor \n",64);
	}

	return 0;
}

int SetAls(int flag,char *  argv[])
{
	unsigned char Als_data;
	unsigned char num;
	unsigned char i;
	unsigned short int data;
	
	if(Periphery_State.Psensor==EXIST)
	{
		data=*argv[0]-'0';
		for(i=1;i<10;i++)
		{
			if(*(argv[0]+i)=='\n')
			{
				i=10;
			}else
			{
				data=data*10;
				data+=*(argv[0]+i)-'0';
			}
		}
		
		
		psensor_Set_Als(data);
		HID_Sent("OK\n",3);
	}else
	{
		HID_Sent("This one doesn't have P sensor \n",64);
	}
	
	return 0;
}
int SetPs(int flag,char *  argv[])
{
	unsigned char Als_data;
	unsigned char num;
	unsigned char i;
	unsigned short int data;
	if(Periphery_State.Psensor==EXIST)
	{
		data=*argv[0]-'0';
		for(i=1;i<10;i++)
		{
			if(*(argv[0]+i)=='\n')
			{
				i=10;
			}else
			{
				data=data*10;
				data+=*(argv[0]+i)-'0';
			}
		}
		psensor_Set_Ps(data);
		HID_Sent("OK\n",3);
	}else
	{
		HID_Sent("This one doesn't have Light sensor \n",64);
	}
	
	return 0;
}
int Touch_Status(int flag,char *  argv[])
{
	unsigned short int touch=Periphery_State.previous_touch;
	unsigned char num;
	char data[20]="AT+Touch=\n";
	if(Periphery_State.Touch==EXIST)
	{
		num=cmd_splicing(&data[0],touch);
		HID_Sent(&data[0],num);
	}else
	{
				HID_Sent("This one doesn't have Touch sensor \n",64);
	}
	return 0;
}

int ReadSn(int flag,char *  argv[])
{
	char data[64]="AT+Sn=\n";
	unsigned char num=0;
	if(data_file.SN_size>32)
	{
		HID_Sent("This one doesn't have Sn,please sent it! \n",64);
	}else
	{
		num=cmdA_AND_b(&data[0],&data_file.SN[0],data_file.SN_size);
		HID_Sent(&data[0],num+1);
	}

	return 0;
}
int SetSn(int flag,char * argv[])
{
	unsigned char i=0;
	for(i=0;i<32;i++)
	{
		if(*(argv[0]+i)=='\n')
		{
			data_file.SN_size=i;
		
			i=32;
		}else
		{
			data_file.SN[i]=*(argv[0]+i);
		}
	}
	
	
	UP_FileData();
	HID_Sent("SN Save OK\n",11);
	return 0;
}

int Status(int flag,char * argv[])
{
	if(StringA_B(argv[0],"Sleep",5))
	{
		Periphery_State.status=Sleep;
		HID_Sent("Sleep OK\n",3);
	}else if(StringA_B(argv[0],"Week",4))
	{
		Periphery_State.status=Week;
		HID_Sent("Week OK\n",3);
	}else
	{
			HID_Sent("This one do not have this status!\n",6);
	}
	
	
	return 0;
}
int AtDown(int flag,char * argv[])
{
			if(StringA_B(argv[0],"A",1))
			{
				if(data_file.run_list==MCUA)
				{
					HID_Sent("RUN is A,Can not down A.\n",25);
					return 0;
				}
				list=0;
				Periphery_State.status=Down;
				data_file.down_list=MCUA;
				EraseByte(MCUA);
				HID_Sent("OK\n",3);
			}else if(StringA_B(argv[0],"B",1))
			{
				if(data_file.run_list==MCUB)
				{
					HID_Sent("RUN is B,Can not down B.\n",25);
					return 0;
				}
				list=0;
				Periphery_State.status=Down;
				data_file.down_list=MCUB;	
				EraseByte(MCUB);
				HID_Sent("OK\n",3);
			}else if(StringA_B(argv[0],"C",1))
			{
				if(data_file.run_list==MCUC)
				{
					HID_Sent("RUN is C,Can not down C.\n",25);
					return 0;
				}
				list=0;
				Periphery_State.status=Down;
				data_file.down_list=MCUC;
				EraseByte(MCUC);
				HID_Sent("OK\n",3);
			}else if(StringA_B(argv[0],"OK",2))
			{
				Periphery_State.status=Week;
				HID_Sent("OK\n",3);
			}else
			{
				HID_Sent("DO not have thit run.\n",25);
			}
		return 0;
}	

int Firmware(int flag,char * argv[])
{
	if(StringA_B(argv[0],"A",1))
	{
		if(data_file.run_list==MCUA)
		{
			HID_Sent("Error\n",3);
			return 0;
		}
		data_file.run_list=MCUA;
		UP_FileData();
		HID_Sent("OK\n",3);
	}else if(StringA_B(argv[0],"B",1))
	{
		if(data_file.run_list==MCUB)
		{
			HID_Sent("Error\n",3);
			return 0;
		}
		data_file.run_list=MCUB;
		UP_FileData();
		HID_Sent("OK\n",3);
	}else if(StringA_B(argv[0],"C",1))
	{
		if(data_file.run_list==MCUC)
		{
			HID_Sent("Error\n",3);
			return 0;
		}
		data_file.run_list=MCUC;
		UP_FileData();
		HID_Sent("OK\n",3);
	}else 
	{
		HID_Sent("This one doesn't have that subarea!\n",6);
	}
	return 0;
}	
int ReadAcc(int flag,char * argv[])
{

	HID_Sent("OK\n",3);
	return 0;
}
int ReadGyro(int flag,char * argv[])
{
	unsigned char *cmd[3];

	HID_Sent("OK\n",3);
	return 0;
}
int SetAcc(int flag,char * argv[])
{
	char *cmd[3];
	cmd[0]=argv[0];
	cmd[1]=argv[1];
	cmd[2]=argv[2];
	HID_Sent("OK\n",3);
	return 0;
}
int SetGyro(int flag,char *argv[])
{
	HID_Sent("OK\n",3);
	return 0;
}

int Restart(int flag,char * argv[])
{ unsigned char i=0;
	HID_Sent("OK\n",3);
	for(i=0;i<50;i++)
	delay(50000);

	__set_FAULTMASK(1);
	NVIC_SystemReset();
	return 0;
}

int ReadRun(int flag,char * argv[])
{
	char data[64]="AT+Run=A\n";
	if(data_file.run_list==MCUA)
	{
		HID_Sent(&data[0],9);
	}else if(data_file.run_list==MCUB)
	{
		data[7]='B';
		HID_Sent(&data[0],9);
	}else if(data_file.run_list==MCUC)
	{
			data[7]='C';
			HID_Sent(&data[0],9);
	}else
	{
		HID_Sent("ERROR to RUN\n",6);	
	}
	
	return 0;
}
int ReadPcbaSn(int flag,char * argv[])
{
	char data[64]="AT+PcbaSn=\n";
	unsigned char num=0;
	unsigned char i=0;
	if(data_file.pcbaSn[0]>='0'&data_file.pcbaSn[0]<='9')
	{
		
		
		num=cmdA_AND_b(&data[0],&data_file.pcbaSn[0],0);
		HID_Sent(&data[0],num+1);
	
	}else
	{
		HID_Sent("ERROR to PcbaSn\n",6);	
	}
	return 0;
}

int SetPcbaSn(int flag,char * argv[])
{
	unsigned char i;
	unsigned char size=0;
	for(i=0;i<32;i++)
	{
		if(*(argv[0]+i)=='\n')
		{
			
		
			i=32;
		}else
		{
			size=i+1;
			data_file.pcbaSn[i]=*(argv[0]+i);
		}
	}

	UP_FileData();
	HID_Sent("PcbaSn Save OK\n",11);
	return 0;
}

int ReadTypeId(int flag,char * argv[])
{
	char data[64]="AT+TypeId=\n";
	unsigned char num=0;
	unsigned char i=0;
	if(data_file.TpyeId[0]<'0'|data_file.TpyeId[0]>'Z')
	{
				HID_Sent("ERROR to TypeId\n",6);	
	}else
	{
				num=cmdA_AND_b(&data[0],&data_file.TpyeId[0],0);
		HID_Sent(&data[0],num+1);

	}
	return 0;

}
int SetTypeId(int flag,char * argv[])
{
	unsigned char i;
	unsigned char size=0;
	for(i=0;i<32;i++)
	{
		if(*(argv[0]+i)=='\n')
		{
			
		
			i=32;
		}else
		{
			size=i+1;
			data_file.TpyeId[i]=*(argv[0]+i);
		}
	}

	UP_FileData();
	HID_Sent("PcbaSn Save OK\n",11);
}
int SofeVersions(int flag,char * argv[])
{
	char data[64]="AT+SofeVersions=\n";
	unsigned char num=0;
	unsigned char i=0;
	cmdA_AND_b(&data[0],&data_file.soft[0],5);
	HID_Sent(&data[0],64);
return 0;
}
unsigned char cmdA_AND_b(char *dataA,char *dataB,unsigned char size)
{
	unsigned char i=0;
	unsigned char flag=0;
	if(size==0)
	{
		for(i=0;i<32;i++)
		{
			if(*(dataB+i)=='\n')
			{	
				
				i=32;
			}else
			{
				size=i+1;
			}
		}
	
	}
	
	
	for(i=0;i<CMD_Max;i++)
	{
			if(*(dataA+i)=='\n')
			{
				flag=i;
				i=CMD_Max;
			}
	}
	for(i=0;i<size;i++)
	{
		*(dataA+flag+i)=*(dataB+i);
	}
	*(dataA+flag+i)='\n';
	return flag+i;
}
	

unsigned char cmd_splicing(char *data,unsigned short int num )
{
	unsigned char i=0;
	unsigned char flag=0;
	for(i=0;i<CMD_Max;i++)
	{
			if(*(data+i)=='\n')
			{
				flag=i;
				i=CMD_Max;
			}
	}
	if(num>=10000)
	{
		*(data+flag)=num/10000+'0';
		*(data+flag+1)=num/1000%10+'0';
		*(data+flag+2)=num/100%10+'0';
		*(data+flag+3)=num/10%10+'0';
		*(data+flag+4)=num%10+'0';
		*(data+flag+5)='\n';
		
		return flag+6;
	}else if(num>=1000)
	{
		*(data+flag+0)=num/1000%10+'0';
		*(data+flag+1)=num/100%10+'0';
		*(data+flag+2)=num/10%10+'0';
		*(data+flag+3)=num%10+'0';
		*(data+flag+4)='\n';
		
		return flag+5;
	}else if(num>=100)
	{		
		*(data+flag+0)=num/100%10+'0';
		*(data+flag+1)=num/10%10+'0';
		*(data+flag+2)=num%10+'0';
		*(data+flag+3)='\n';
		
		return flag+4;
	}else if(num>=10)
	{
		*(data+flag+0)=num/10%10+'0';
		*(data+flag+1)=num%10+'0';
		*(data+flag+2)='\n';
		
		return flag+3;
	}else
	{
		*(data+flag+0)=num%10+'0';
		*(data+flag+1)='\n';
		
		return flag+2;
	}
}

void cmd_init(void)
{
	At_cmd.cmd[0].name="AT+Camera\n";
	At_cmd.cmd[0].maxargs=1;
	At_cmd.cmd[0].size=8;
	At_cmd.cmd[0].cmd = &Camera;
	
	At_cmd.cmd[1].name="AT+Light?\n";
	At_cmd.cmd[1].maxargs=0;
	At_cmd.cmd[1].size=9;
	At_cmd.cmd[1].cmd = &ReadLight;
	
	At_cmd.cmd[2].name="AT+Als?\n";
	At_cmd.cmd[2].maxargs=0;
	At_cmd.cmd[2].size=7;
	At_cmd.cmd[2].cmd=&ReadALS;
	
	At_cmd.cmd[3].name="AT+Ps?\n";
	At_cmd.cmd[3].maxargs=0;
	At_cmd.cmd[3].size=6;
	At_cmd.cmd[3].cmd=&ReadPS;	
	
	At_cmd.cmd[4].name="AT+Als\n";
	At_cmd.cmd[4].maxargs=1;
	At_cmd.cmd[4].size=6;
	At_cmd.cmd[4].cmd=&SetAls;
	
	At_cmd.cmd[5].name="AT+Ps\n";
	At_cmd.cmd[5].maxargs=1;
	At_cmd.cmd[5].size=5;
	At_cmd.cmd[5].cmd=&SetPs;
	
	At_cmd.cmd[6].name="AT+Touch?\n";	
	At_cmd.cmd[6].maxargs=0;
	At_cmd.cmd[6].size=8;
	At_cmd.cmd[6].cmd=&Touch_Status;
	
	At_cmd.cmd[7].name="AT+Sn?\n";
	At_cmd.cmd[7].maxargs=0;
	At_cmd.cmd[7].size=6;
	At_cmd.cmd[7].cmd=&ReadSn;
	
	At_cmd.cmd[8].name="AT+Sn=\n";	
	At_cmd.cmd[8].maxargs=1;
	At_cmd.cmd[8].size=6;
	At_cmd.cmd[8].cmd=&SetSn;
	
	At_cmd.cmd[9].name="AT+Status\n";
	At_cmd.cmd[9].maxargs=1;
	At_cmd.cmd[9].size=9;
	At_cmd.cmd[9].cmd=&Status;
	
	At_cmd.cmd[10].name="AT+Run?\n";
	At_cmd.cmd[10].maxargs=0;
	At_cmd.cmd[10].size=7;
	At_cmd.cmd[10].cmd=&ReadRun;
	
	
	At_cmd.cmd[11].name="AT+Down\n";
	At_cmd.cmd[11].maxargs=1;
	At_cmd.cmd[11].size=7;
	At_cmd.cmd[11].cmd = &AtDown;
	
	At_cmd.cmd[12].name="AT+Firmware\n";
	At_cmd.cmd[12].maxargs=1;
	At_cmd.cmd[12].size=11;
	At_cmd.cmd[12].cmd = &Firmware;
	
	At_cmd.cmd[13].name="AT+Reboot\n";	
	At_cmd.cmd[13].maxargs=0;
	At_cmd.cmd[13].size=9;
	At_cmd.cmd[13].cmd=&Restart;
	
	At_cmd.cmd[14].name="AT+PcbaSn?\n";	
	At_cmd.cmd[14].maxargs=0;
	At_cmd.cmd[14].size=10;
	At_cmd.cmd[14].cmd=&ReadPcbaSn;	

	At_cmd.cmd[15].name="AT+PcbaSn=\n";	
	At_cmd.cmd[15].maxargs=1;
	At_cmd.cmd[15].size=10;
	At_cmd.cmd[15].cmd=&SetPcbaSn;	

	At_cmd.cmd[16].name="AT+TypeId?\n";	
	At_cmd.cmd[16].maxargs=0;
	At_cmd.cmd[16].size=10;
	At_cmd.cmd[16].cmd=&ReadTypeId;	
	
	At_cmd.cmd[17].name="AT+TypeId=\n";	
	At_cmd.cmd[17].maxargs=1;
	At_cmd.cmd[17].size=10;
	At_cmd.cmd[17].cmd=&SetTypeId;	

	At_cmd.cmd[18].name="AT+SofeVersions?\n";	
	At_cmd.cmd[18].maxargs=0;
	At_cmd.cmd[18].size=15;
	At_cmd.cmd[18].cmd=&SofeVersions;	
	
	At_cmd.cmd[19].name="AT+Acc?\n";	
	At_cmd.cmd[19].maxargs=0;
	At_cmd.cmd[19].size=7;
	At_cmd.cmd[19].cmd=&ReadAcc;	
	
	At_cmd.cmd[20].name="AT+Acc=\n";	
	At_cmd.cmd[20].maxargs=3;
	At_cmd.cmd[20].size=7;
	At_cmd.cmd[20].cmd=&SetAcc;	
	
	At_cmd.cmd[21].name="AT+Gyro?\n";	
	At_cmd.cmd[21].maxargs=0;
	At_cmd.cmd[21].size=8;
	At_cmd.cmd[21].cmd=&ReadGyro;	
	
	At_cmd.cmd[22].name="AT+Gyro=\n";	
	At_cmd.cmd[22].maxargs=3;
	At_cmd.cmd[22].size=8;
	At_cmd.cmd[22].cmd=&SetGyro;		
	
	
	At_cmd.com_num=23;
}

unsigned char StringA_B(char *dataA,char *dataB,unsigned char size)
{
	unsigned char i=0;
	for(i=0;i<size;i++)
	{
		if((*(dataA+i))!=(*(dataB+i)))
			return 0;
	}
	return 1;
}
unsigned char Choice_string(char *datain,char *dataout,unsigned char num)
{
	unsigned char i=0;
	unsigned char j=0;
	unsigned char section=0;
	for(i=0;i<250;i++)
	{
		if(num==section)
		{
			*(dataout+j)=*(datain+i);
			j++;
		}
		if(*(datain+i)=='\n')
		{
			*(dataout+j)='\n';
			return 1;
		}
		
		if(*(datain+i)==' '|*(datain+i)=='=')
		{
			*(dataout+j)='\n';
			section++;
		}
		

	}
	return 0;
}


unsigned char XOR(unsigned char *dat,unsigned char size)
{
	unsigned char xor;
	unsigned char i=0;
	xor=*dat;
	for(i=1;i<size;i++)
	{
		xor=xor^(*(dat+i));
	}
	return  xor;
}

void find_cmd (char *cmd)
{
	cmd_tbl_t *cmdtp;
	unsigned char i=0;
	unsigned char j=0;
	char	data[64];
	unsigned char	data1[64];
	char cmd_dat[MAXCOM_Parameter][64];
	unsigned short int dat16;
	char * argv[10];
	unsigned short int num=0;
	
//	Periphery_State.status=Sleep;
	if(Periphery_State.status!=Down)
	{
		for(i=0;i<At_cmd.com_num;i++)
		{
			if(StringA_B(cmd,(At_cmd.cmd[i].name),At_cmd.cmd[i].size))
			{
		
				if(At_cmd.cmd[i].maxargs==0)
				{
					At_cmd.cmd[i].cmd(0,argv);
				}else
				{
					for(j=0;j<At_cmd.cmd[i].maxargs;j++)
						{
							Choice_string(cmd,&cmd_dat[j][0],j+1);
							argv[j]=&cmd_dat[j][0];
						}
					At_cmd.cmd[i].cmd(At_cmd.cmd[i].maxargs,argv);
				}
					i=At_cmd.com_num;
			}
		}
		if(i==At_cmd.com_num)
			HID_Sent("Com is error!\n",14);
		
	}else 
	{//down
		if(StringA_B(cmd,"AT+Down OK\n",10))
		{
			Periphery_State.status=Week;
			HID_Sent("AT+Down OK\n",11);
			
		}else if(StringA_B(cmd,"AT+Down\n",7))
		{
			Periphery_State.status=Week;
			HID_Sent("OK\n",3);
			
		}else
		{
			for(i=0;i<61;i++)
			{
				data1[i]=*(cmd+2+i);
			}
				num=*(cmd+0)<<8;
			  num=num+*(cmd+1);
				if(num==list)
				{
				
				
				if(XOR(&data1[0],60)==data1[60])
				{		
						list++;
						writeflash(data_file.down_list,&data1[0],60);	
						data_file.down_list=data_file.down_list+60;
						dat16=(*cmd)<<8;
						dat16|=*(cmd+1)<<0;
						if(dat16>=10000)
						{
							data[0]=dat16/10000+'0';
							data[1]=dat16/1000%10+'0';
							data[2]=dat16/100%10+'0';
							data[3]=dat16/10%10+'0';
							data[4]=dat16%10+'0';
							data[5]=' ';
							data[6]='O';
							data[7]='K';
							data[8]='\n';
							HID_Sent(&data[0],9);	
						}else if(dat16>=1000)
						{
	
							data[0]=dat16/1000%10+'0';
							data[1]=dat16/100%10+'0';
							data[2]=dat16/10%10+'0';
							data[3]=dat16%10+'0';
							data[4]=' ';
							data[5]='O';
							data[6]='K';
							data[7]='\n';
							HID_Sent(&data[0],8);	
			
						}else if(dat16>=100)
						{

							data[0]=dat16/100%10+'0';
							data[1]=dat16/10%10+'0';
							data[2]=dat16%10+'0';
							data[3]=' ';
							data[4]='O';
							data[5]='K';
							data[6]='\n';
							HID_Sent(&data[0],7);			
			
						}else if(dat16>=10)
						{

							data[0]=dat16/10%10+'0';
							data[1]=dat16%10+'0';
							data[2]=' ';
							data[3]='O';
							data[4]='K';
							data[5]='\n';
							HID_Sent(&data[0],6);				
					}else
				  {
						//	data[0]=dat16/10%10+'0';
							data[0]=dat16%10+'0';
							data[1]=' ';
							data[2]='O';
							data[3]='K';
							data[4]='\n';
							HID_Sent(&data[0],6);				
						}
				}else
				{
					HID_Sent("Error\n",6);
				}
			}else 
			{	
					HID_Sent("Error\n",6);//list error
			}

	}
		
	}
}


