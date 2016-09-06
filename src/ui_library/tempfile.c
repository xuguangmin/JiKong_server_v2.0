
#include <string.h>
#define EVENT_BUTTON_CLICK  0x1010
#define EVENT_IMAGE_CLICK  0X1020
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
	ImageControl_5=5,
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
	HMUI_4=20004,
	ImageControl_23=23,
	ImageControl_24=24,
	ImageControl_25=25,
	ImageControl_26=26,
	ImageControl_27=27,
	ImageControl_28=28,
	ImageControl_29=29,
	ImageControl_30=30,
	ImageControl_31=31,
	ImageControl_32=32,
	ImageControl_33=33,
	ImageControl_34=34,
	ImageControl_35=35,
	ImageControl_36=36,
	ImageControl_37=37,
	ImageControl_38=38,
	ImageControl_39=39,
	ImageControl_40=40,
	ImageControl_41=41,
	ImageControl_42=42,
	ImageControl_43=43,
	ImageControl_44=44,
	ImageControl_45=45,
	ImageControl_46=46,
	ImageControl_47=47,
	ImageControl_48=48,
	ImageControl_49=49,
	ImageControl_50=50,
	ImageControl_51=51,
	ImageControl_52=52,
	ImageControl_53=53,
	ImageControl_54=54,
	ImageControl_55=55,
	ImageControl_56=56,
	ImageControl_57=57,
	ImageControl_58=58,
	ImageControl_59=59,
	ImageControl_60=60,
	ImageControl_61=61,
	ImageControl_62=62,
	ImageControl_63=63,
	ImageControl_64=64,
	ImageControl_65=65,
	ImageControl_66=66,
	ImageControl_67=67,
	ImageControl_68=68,
	ImageControl_69=69,
	ImageControl_70=70,
	ImageControl_71=71,
	ImageControl_72=72,
	InfraredControl_73=73,
	ImageControl_78=78,
	ImageControl_79=79,

};


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
	devi_init_serial_port(1,9600,8,1,0);

	return 0;
}

int dealwithSerialportData(int port,unsigned char *data,int len)
{
	if(nLoad == 0)
	{
		lua_getglobal(L,"DealwithSerialportData");
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
		}
		
		printf("load success in dealwithSerialportData\n");
	}else
	{
		printf("load failed in dealwithSerialportData\n");
	}
	
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

		if(senderId == ImageControl_23)
		{
		devi_irda_write_data(1,1);

		}
		if(senderId == ImageControl_24)
		{
		devi_irda_write_data(1,2);

		}
		if(senderId == ImageControl_25)
		{
		devi_irda_write_data(1,3);

		}
		if(senderId == ImageControl_26)
		{
		devi_irda_write_data(1,4);

		}
		if(senderId == ImageControl_27)
		{
		devi_irda_write_data(1,5);

		}
		if(senderId == ImageControl_28)
		{
		devi_irda_write_data(1,6);

		}
		if(senderId == ImageControl_29)
		{
		devi_irda_write_data(1,7);

		}
		if(senderId == ImageControl_30)
		{
		devi_irda_write_data(1,8);

		}
		if(senderId == ImageControl_31)
		{
		devi_irda_write_data(1,9);

		}
		if(senderId == ImageControl_32)
		{
		devi_irda_write_data(1,10);

		}
		if(senderId == ImageControl_33)
		{
		devi_irda_write_data(1,11);

		}
		if(senderId == ImageControl_34)
		{
		devi_irda_write_data(1,12);

		}
		if(senderId == ImageControl_35)
		{
		devi_irda_write_data(1,13);

		}
		if(senderId == ImageControl_36)
		{
		devi_irda_write_data(1,14);

		}
		if(senderId == ImageControl_37)
		{
		devi_irda_write_data(1,15);

		}
		if(senderId == ImageControl_38)
		{
		devi_irda_write_data(1,16);

		}
		if(senderId == ImageControl_39)
		{
		devi_irda_write_data(1,17);

		}
		if(senderId == ImageControl_40)
		{
		devi_irda_write_data(1,18);

		}
		if(senderId == ImageControl_41)
		{
		devi_irda_write_data(1,19);

		}
		if(senderId == ImageControl_42)
		{
		devi_irda_write_data(1,20);

		}
		if(senderId == ImageControl_43)
		{
		devi_irda_write_data(1,21);

		}
		if(senderId == ImageControl_44)
		{
		devi_irda_write_data(1,22);

		}
		if(senderId == ImageControl_45)
		{
		devi_irda_write_data(1,23);

		}
		if(senderId == ImageControl_46)
		{
		devi_irda_write_data(1,24);

		}
		if(senderId == ImageControl_47)
		{
		devi_irda_write_data(1,25);

		}
		if(senderId == ImageControl_48)
		{
		devi_irda_write_data(1,26);

		}
		if(senderId == ImageControl_49)
		{
		devi_irda_write_data(1,27);

		}
		if(senderId == ImageControl_50)
		{
		devi_irda_write_data(1,28);

		}
		if(senderId == ImageControl_51)
		{
		devi_irda_write_data(1,29);

		}
		if(senderId == ImageControl_52)
		{
		devi_irda_write_data(1,30);

		}
		if(senderId == ImageControl_53)
		{
		devi_irda_write_data(1,31);

		}
		if(senderId == ImageControl_54)
		{
		devi_irda_write_data(1,32);

		}
		if(senderId == ImageControl_55)
		{
		devi_irda_write_data(1,33);

		}
		if(senderId == ImageControl_56)
		{
		devi_irda_write_data(1,34);

		}
		if(senderId == ImageControl_57)
		{
		devi_irda_write_data(1,35);

		}
		if(senderId == ImageControl_58)
		{
		devi_irda_write_data(1,36);

		}
		if(senderId == ImageControl_59)
		{
		devi_irda_write_data(1,37);

		}
		if(senderId == ImageControl_60)
		{
		devi_irda_write_data(1,38);

		}
		if(senderId == ImageControl_61)
		{
		devi_irda_write_data(1,39);

		}
		if(senderId == ImageControl_62)
		{
		devi_irda_write_data(1,40);

		}
		if(senderId == ImageControl_63)
		{
		devi_irda_write_data(1,41);

		}
		if(senderId == ImageControl_64)
		{
		devi_irda_write_data(1,42);

		}
		if(senderId == ImageControl_65)
		{
		devi_irda_write_data(1,43);

		}
		if(senderId == ImageControl_66)
		{
		devi_irda_write_data(1,44);

		}
		if(senderId == ImageControl_67)
		{
		devi_irda_write_data(1,45);

		}
		if(senderId == ImageControl_68)
		{
		devi_irda_write_data(1,46);

		}
		if(senderId == ImageControl_69)
		{
		devi_irda_write_data(1,47);

		}
		if(senderId == ImageControl_70)
		{
		devi_irda_write_data(1,48);

		}
		if(senderId == ImageControl_71)
		{

		}
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
	devi_release_serial_port(1);

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
int myprint(int port,unsigned char *data,int len)
{
	printf("myprint \n");
}
