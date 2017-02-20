
#include <string.h>
#define EVENT_BUTTON_CLICK  0x1010
#define EVENT_IMAGE_CLICK  0X1020

#define EVENT_BUTTON_PRESSED  0x1011
#define EVENT_IMAGE_PRESSED  0X1021

#define EVENT_BUTTON_RELEASED  0x1012
#define EVENT_IMAGE_RELEASED  0X1022

#define EVENT_COMBOBOX_SEL_INDEX_CHANGED 0X1090
#define EVENT_LISTBOX_SEL_INDEX_CHANGED 0X10D0
#define EVENT_TIMER_TICK 0X10C0
#define SetValue(X,V) X=V
#define AddValue(X,V) X+=V
#define ReduceValue(X,V) X-=V
int getIntFromBytes(char *data,int nlen);

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
int nLoad = -1;

lua_State * L;

enum list
{
	HMUI_1=20001,
	ImageControl_1=1,
	HMUI_2=20002,
	ImageControl_2=2,
	ImageControl_3=3,
	ImageControl_4=4,
	ImageControl_90=90,
	ImageControl_91=91,
	ImageControl_92=92,
	ImageControl_93=93,
	ImageControl_94=94,
	ImageControl_95=95,
	ImageControl_96=96,
	ImageControl_97=97,
	ImageControl_98=98,
	ImageControl_99=99,
	ImageControl_100=100,
	ImageControl_101=101,
	ImageControl_102=102,
	ImageControl_103=103,
	ImageControl_104=104,
	ImageControl_105=105,
	SerialPortControl_114=114,
	ImageControl_106=106,
	ImageControl_107=107,
	ImageControl_108=108,
	ImageControl_109=109,
	ImageControl_110=110,
	ImageControl_111=111,
	ImageControl_112=112,
	ImageControl_113=113,
	HMUI_3=20003,
	ImageControl_6=6,
	ImageControl_7=7,
	ImageControl_8=8,
	ImageControl_9=9,
	ImageControl_10=10,
	ImageControl_11=11,
	ImageControl_12=12,
	ImageControl_13=13,
	ImageControl_14=14,
	ImageControl_15=15,
	ImageControl_16=16,
	ImageControl_17=17,
	ImageControl_18=18,
	ImageControl_19=19,
	ImageControl_20=20,
	ImageControl_21=21,
	SerialPortControl_74=74,
	ImageControl_75=75,
	ImageControl_76=76,
	ImageControl_77=77,
	ImageControl_5=5,
	ImageControl_27=27,
	ImageControl_28=28,
	ImageControl_37=37,
	ImageControl_38=38,
	ImageControl_39=39,
	ImageControl_40=40,
	HMUI_4=20004,
	ImageControl_30=30,
	ImageControl_31=31,
	ImageControl_32=32,
	ImageControl_33=33,
	ImageControl_34=34,
	ImageControl_35=35,
	ImageControl_36=36,
	ImageControl_68=68,
	ImageControl_69=69,
	ImageControl_70=70,
	ImageControl_71=71,
	ImageControl_72=72,
	ImageControl_78=78,
	ImageControl_79=79,
	ImageControl_89=89,
	ImageControl_83=83,
	ImageControl_88=88,
	SerialPortControl_22=22,
	ImageControl_23=23,
	ImageControl_24=24,
	ImageControl_25=25,
	ImageControl_26=26,

};

int dealwithSerialportData(int port,unsigned char *data,int len)
{
	if(nLoad == 0)
	{
		/*lua_getglobal(L,"DealwithSerialportData");
		if(lua_isfunction(L,-1))
		{
			lua_pushnumber(L,port);
			lua_pushstring(L,data);
			lua_pushnumber(L,len);
			if(lua_pcall(L,3,1,0) == 0)
			{	
				printf("lua call suceess \n");
				lua_pop(L,1);
			}else{
				printf("lua call failed \n");
			}
		}else{
			printf("DealwithSerialportdata is not a function\n");
		}*/
		
		printf("load success in dealwithSerialportData\n");
	}else
	{
		printf("load failed in dealwithSerialportData\n");
	}
	
	return 0;
}
int InitializeControls()
{
    L=lua_open();
	luaL_openlibs(L);
	if(luaL_loadfile(L,"controlprogram.lua") || lua_pcall(L,0,0,0))
	{
        printf("loadfile failed\n");
        return -1;
	}
    else
    {
		nLoad = 0;
        printf("loadfile success\n");
    }
    lua_pop(L,1);
	devi_init_serial_port(2,1200,8,1,0);
	devi_init_serial_port(1,9600,8,1,0);
	devi_init_serial_port(3,9600,8,1,0);

	return 0;
}


int ProcessEvent(int senderId, int event, char *data, int len)
{
	int iRet = 0;
	if(nLoad == 0)
	{
		lua_getglobal(L,"ControlProgram");
		if(lua_istable(L,-1))
		{
			lua_getfield(L,-1,"ProcessEvent");
			if(lua_isfunction(L,-1))
			{
				lua_pushnumber(L,senderId);
				lua_pushnumber(L,event);
				lua_pushstring(L,data);
				//lua_pushnumber(len);
				if(lua_pcall(L,3,1,0) == 0)
				{	
					printf("lua call suceess \n");
					lua_pop(L,1);
				}else{
					printf("lua call failed \n");
				}
			}
		}else
		{
			printf("not call it\n");
		}
	}
    switch (event)
    {

    case EVENT_BUTTON_CLICK:
    {

    }
    break;

    case EVENT_IMAGE_CLICK:
    {

		if(senderId == ImageControl_27)
		{
uie_ctrl_unvisual(ImageControl_37);uie_ctrl_unvisual(ImageControl_38);uie_ctrl_unvisual(ImageControl_39);uie_ctrl_unvisual(ImageControl_40);
		}
    }
    break;

    case EVENT_BUTTON_PRESSED:
    {

    }
    break;

    case EVENT_IMAGE_PRESSED:
    {

    }
    break;

    case EVENT_BUTTON_RELEASED:
    {

    }
    break;

    case EVENT_IMAGE_RELEASED:
    {

    }
    break;

    case EVENT_LISTBOX_SEL_INDEX_CHANGED:
    {
        int SelectedIndex = 0;
        SelectedIndex = getIntFromBytes(data + 15,4);

    }
    break;

    case EVENT_COMBOBOX_SEL_INDEX_CHANGED:
    {
        int SelectedIndex = 0;
        SelectedIndex = getIntFromBytes(data + 15,4);

    }
    break;

    case EVENT_TIMER_TICK:
    {

    }
    break;

     default:
			break;
	}

	return 0;
}

int ReleaseControls()
{
    lua_close(L);
	devi_release_serial_port(2);
	devi_release_serial_port(1);
	devi_release_serial_port(3);

    return 0;
}
int getIntFromBytes(char *data,int nlen)
{
    int iRetValue=0;
    if(data!=NULL&&nlen==4)
    {
        int h0=0;
        h0=data[0]<<24&0xFF000000;
        int h1=0;
        h1=(data[1]<<16)&0x00FF0000;
        int l0=0;
        l0=(data[2]<<8)&0x0000FF00;
        int l1=0;
        l1=data[3]&0x000000FF;
        iRetValue=h0+h1+l0+l1;
    }
    return iRetValue;
}
