/******************************************************************************

                  ��Ȩ���� (C), 2001-2020, ���������ſƼ��ɷ����޹�˾

 ******************************************************************************
  �ļ����� : luaevent.c
  ����    :
  �������� :

  �汾    : 1.0
  �������� : �Լ��ط�������ṩ�Ľӿڽ�����luaʽ�ķ�װ����lua�ű����Ե�����Щ�ӿڡ�
           Ҳ���Ƕ�lua��һ����ǿ���ṩ��һЩ���ڼ��ص�lua��̽ӿ�


  ˵����
      ��lua��ע�ắ�������Ƿ������µ�ԭ��
      ����һ����ʾ����ֵ����������

      typedef int (*lua_CFunction)(lua_State *L)


  ��̽ӿڣ�

		serial.open(serial_no, baud_rate, data_bits, parity, stop_bit)
		serial.close(serial_no)
		serial.send(serial_no, data)

		infrared.send(infrared_no, rec_no)

		relay.open(relay_no)
		relay.close(relay_no)

		telnet.open(telnet_no, ipaddr, port)
		telnet.send(telnet_no, data)
		telnet.close(telnet_no)

		wol.send(mac_addr, password)
		http.send(url)
		onvif.send(http_uri, http_body)

		ui.show(ctrl_id)
		ui.hide(ctrl_id)
		ui.enable(ctrl_id)
		ui.disable(ctrl_id)

		slider.setvalue(ctrl_id, value)
		progress.setvalue(ctrl_id, value)
		radiobox.check(ctrl_id, true|false)
		checkbox.check(ctrl_id, true|false)

	��̽ӿ�˵��


		���ܣ����ô��ڲ���
		������
			serial_no :���ڱ�ţ�ȡֵ1 ~ 8
			baud_rate :������
			data_bits :����λ��ȡֵ5, 6, 7, 8
			parity    :У�飬ȡֵN, O, E��N none, O odd, E even
			stop_bit  :ֹͣλ��ȡֵ 1, 2
		serial.open(serial_no, baud_rate, data_bits, parity, stop_bit)

		���ܣ��رմ���
		������
			serial_no :���ڱ�ţ�ȡֵ1 ~ 8
		serial.close(serial_no)

		���ܣ��򴮿ڷ�������
		������
			serial_no :���ڱ�ţ�ȡֵ1 ~ 8
			data      :��������
		serial.send(serial_no, data)
******************************************************************************/
#include "lua.h"
#include "lauxlib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "device_interface/device_interface.h"

/* UI */
static int setUiControlDisable(lua_State *L)
{
	int ctrl_id = luaL_checkint(L, 1);
	int result = (0 == uie_ctrl_disable(ctrl_id)) ? 1:0;

	lua_pushboolean(L, result);
	return 1;   /* ���ؽ���ĸ��� */
}
static int setUiControlEnable(lua_State *L)
{
	int ctrl_id = luaL_checkint(L, 1);
	int result = (0 == uie_ctrl_enable(ctrl_id)) ? 1:0;

	lua_pushboolean(L, result);
	return 1;   /* ���ؽ���ĸ��� */
}

static int setUiControlUnvisible(lua_State *L)
{
	int ctrl_id = luaL_checkint(L, 1);
	int result = (0 == uie_ctrl_unvisual(ctrl_id)) ? 1:0;

	lua_pushboolean(L, result);
	return 1;   /* ���ؽ���ĸ��� */
}
static int setUiControlVisible(lua_State *L)
{
	int ctrl_id = luaL_checkint(L, 1);
	int result = (0 == uie_ctrl_visual(ctrl_id)) ? 1:0;

	lua_pushboolean(L, result);
	return 1;   /* ���ؽ���ĸ��� */
}
static const struct luaL_Reg lua_jikong_lib_ui[] = {
	{"show",    setUiControlVisible},
	{"hide",    setUiControlUnvisible},
	{"enable",  setUiControlEnable},
	{"disable", setUiControlDisable},
	{NULL,      NULL}
};

/* ����������� */
static int setUiSliderValue(lua_State *L)
{
	int ctrl_id  = luaL_checkint(L, 1);
	int progress = luaL_checkint(L, 2);
	int result = (0 == uie_ctrl_set_value_int(ctrl_id, progress)) ? 1:0;

	lua_pushboolean(L, result);
	return 1;   /* ���ؽ���ĸ��� */
}
static const struct luaL_Reg lua_jikong_lib_slider[] = {
	{"setvalue",    setUiSliderValue},
	{NULL,          NULL}
};

/* �����������ֵ */
static int setUiProgressValue(lua_State *L)
{
	int ctrl_id  = luaL_checkint(L, 1);
	int progress = luaL_checkint(L, 2);
	int result = (0 == uie_ctrl_set_value_int(ctrl_id, progress)) ? 1:0;

	lua_pushboolean(L, result);
	return 1;   /* ���ؽ���ĸ��� */
}
static const struct luaL_Reg lua_jikong_lib_progress[] = {
	{"setvalue",    setUiProgressValue},
	{NULL,          NULL}
};

/* checkbox */
static int setUiCheckboxCheckValue(lua_State *L)
{
	int result = 0;
	int ctrl_id  = luaL_checkint(L, 1);
	int b_check  = lua_toboolean(L, 2);

	if(0 == b_check)
	{
		result = (0 == uie_ctrl_set_not_checked(ctrl_id)) ? 1:0;
	}
	else
	{
		result = (0 == uie_ctrl_set_checked(ctrl_id)) ? 1:0;
	}

	lua_pushboolean(L, result);
	return 1;   /* ���ؽ���ĸ��� */
}
static const struct luaL_Reg lua_jikong_lib_checkbox[] = {
	{"check",   setUiCheckboxCheckValue},
	{NULL,      NULL}
};

/* radiobox */
static int setUiRadioboxCheckValue(lua_State *L)
{
	int result = 0;
	int ctrl_id  = luaL_checkint(L, 1);
	int b_check  = lua_toboolean(L, 2);

	if(0 == b_check)
	{
		result = (0 == uie_ctrl_set_not_checked(ctrl_id)) ? 1:0;
	}
	else
	{
		result = (0 == uie_ctrl_set_checked(ctrl_id)) ? 1:0;
	}

	lua_pushboolean(L, result);
	return 1;   /* ���ؽ���ĸ��� */
}
static const struct luaL_Reg lua_jikong_lib_radiobox[] = {
	{"check",   setUiRadioboxCheckValue},
	{NULL,      NULL}
};


/* telnet */
static int telnetPortWrite(lua_State *L)
{
	int result = 0;
	int telnet_no, data_len;
	const char *data;

	telnet_no = luaL_checkint(L, 1);
	data      = luaL_checkstring(L, 2);
	data_len  = lua_strlen(L, 2);
	if(data_len > 0)
	{
		result = (0 == devi_telnet_write_data(telnet_no, (unsigned char *)data, data_len)) ? 1:0;
	}

	lua_pushboolean(L, result);
	return 1;   /* ���ؽ���ĸ��� */
}
static int telnetPortClose(lua_State *L)
{
	int telnet_no       = luaL_checkint(L, 1);

	int result = (0 == devi_release_telnet(telnet_no)) ? 1:0;
	lua_pushboolean(L, result);
	return 1;
}
static int telnetPortOpen(lua_State *L)
{
	int result = 0;
	int telnet_no       = luaL_checkint(L, 1);
	const char *ip_addr = luaL_checkstring(L, 2);
	int ip_port         = luaL_checkint(L, 3);

	if(lua_strlen(L, 2) <= 0)
	{
		lua_pushboolean(L, 0);
		return 1;
	}

	result = (0 == devi_init_telnet(telnet_no, ip_addr, ip_port)) ? 1:0;
	lua_pushboolean(L, result);
	return 1;
}
static const struct luaL_Reg lua_jikong_lib_telnet[] = {
	{"open",  telnetPortOpen},
	{"close", telnetPortClose},
	{"send",  telnetPortWrite},
	{NULL,    NULL}
};


/* http */
static int httpSendData(lua_State *L)
{
	int result = 0;
	const char *data = luaL_checkstring(L, 1);

	if(lua_strlen(L, 1) > 0)
	{
		result = (0 == devi_network_http(data)) ? 1:0;
	}

	lua_pushboolean(L, result);
	return 1;   /* ���ؽ���ĸ��� */
}
static const struct luaL_Reg lua_jikong_lib_http[] = {
	{"send",  httpSendData},
	{NULL,    NULL}
};

/* onvif */
static int onvifSendData(lua_State *L)
{
	int result = 0;
	const char *uri  = luaL_checkstring(L, 1);
	const char *body = luaL_checkstring(L, 2);

	if(lua_strlen(L, 1) > 0 && lua_strlen(L, 2) > 0)
	{
		result = (0 == devi_network_onvif(uri, body)) ? 1:0;
	}

	lua_pushboolean(L, result);
	return 1;   /* ���ؽ���ĸ��� */
}
static const struct luaL_Reg lua_jikong_lib_onvif[] = {
	{"send",  onvifSendData},
	{NULL,    NULL}
};


/* WOL */
static int deviWolSendData(lua_State *L)
{
	int data_len, result = 0;
	const char *macAddress = luaL_checkstring(L, 1);

	data_len  = lua_strlen(L, 1);
	if(data_len > 0)
	{
		const char *password = NULL;

		data_len = 0;
		if(lua_type(L, 2) != LUA_TNIL)      /* �������Ϊ�� */
		{
			password = luaL_checkstring(L, 2);
			data_len = lua_strlen(L, 2);
		}

		result = (0 == devi_wol_write_data(macAddress, (unsigned char *)password, data_len)) ? 1:0;
	}

	lua_pushboolean(L, result);
	return 1;   /* ���ؽ���ĸ��� */
}
static const struct luaL_Reg lua_jikong_lib_wol[] = {
	{"send",  deviWolSendData},
	{NULL,    NULL}
};


/* �̵� */
static int relay_write_open(lua_State *L)
{
	int relay_no = luaL_checkint(L, 1);
	int result   = (0 == devi_relay_write_data(relay_no, 1)) ? 1:0;

	lua_pushboolean(L, result);
	return 1;   /* ���ؽ���ĸ��� */
}
static int relay_write_close(lua_State *L)
{
	int relay_no = luaL_checkint(L, 1);
	int result   = (0 == devi_relay_write_data(relay_no, 0)) ? 1:0;

	lua_pushboolean(L, result);
	return 1;   /* ���ؽ���ĸ��� */
}
static const struct luaL_Reg lua_jikong_lib_relay[] = {
	{"open",  relay_write_open},
	{"close", relay_write_close},
	{NULL,    NULL}
};


/* ���� */
static int infraredWrite(lua_State *L)
{
	int infrared_no = luaL_checkint(L, 1);
	int rec_no      = luaL_checkint(L, 2);

	int result = (0 == devi_irda_write_data(infrared_no, rec_no)) ? 1:0;

	lua_pushboolean(L, result);
	return 1;   /* ���ؽ���ĸ��� */
}
static const struct luaL_Reg lua_jikong_lib_infrared[] = {
	{"send",  infraredWrite},
	{NULL,    NULL}
};


/* ���� */
static int serialPortWrite(lua_State *L)
{
	int data_len, result = 0;
	int serial_no    = luaL_checkint(L, 1);
	const char *data = luaL_checkstring(L, 2);

	data_len  = lua_strlen(L, 2);
	if(data_len > 0)
	{
		result = (0 == devi_serial_port_write_data(serial_no, (unsigned char *)data, data_len)) ? 1:0;
	}

	lua_pushboolean(L, result);
	return 1;   /* ���ؽ���ĸ��� */
}
static int serialPortClose(lua_State *L)
{
	int serial_no  = luaL_checkint(L, 1);
	int result = (0 == devi_release_serial_port(serial_no)) ? 1:0;

	lua_pushboolean(L, result);
	return 1;
}
/* serial open
 * 0--none, 1--odd, 2--even
 *
 * ˵����
 *    ���ڱ�š�����λ��ֹͣλ�Ȳ������������飬
 *    ���������������Ϸ��ԡ�
 *    Ŀ���ǿ��ǵ�������չʱ���������Ҫ����
 */
static int serialPortOpen(lua_State *L)
{
	int result, parity = 0;
	int serial_no          = luaL_checkint(L, 1);
	int baud_rate          = luaL_checkint(L, 2);
	int data_bits          = luaL_checkint(L, 3);
	const char *str_parity = luaL_checkstring(L, 4);
	int stop_bit           = luaL_checkint(L, 5);

	if(lua_strlen(L, 4) <= 0)
	{
		lua_pushboolean(L, 0);
		return 1;
	}

	if     (!strcasecmp(str_parity, "N"))     parity = 0;
	else if(!strcasecmp(str_parity, "none"))  parity = 0;
	else if(!strcasecmp(str_parity, "O"))     parity = 1;
	else if(!strcasecmp(str_parity, "odd"))   parity = 1;
	else if(!strcasecmp(str_parity, "E"))     parity = 2;
	else if(!strcasecmp(str_parity, "even"))  parity = 2;
	else {
		lua_pushboolean(L, 0);
		return 1;
	}

	//printf("%s serial %d %d %d-%s-%d\n", __FUNCTION__, serial_no, baud_rate, data_bits, str_parity, stop_bit);
	result = (0 == devi_init_serial_port(serial_no, baud_rate, data_bits, stop_bit, parity)) ? 1:0;
	lua_pushboolean(L, result);
	return 1;
}
static const struct luaL_Reg lua_jikong_lib_serial[] = {
	{"open",  serialPortOpen},
	{"close", serialPortClose},
	{"send",  serialPortWrite},
	{NULL,    NULL}
};


/* ���� */
static int only_test_bool(lua_State *L)
{
	int result = luaL_checkint(L, 1);

	lua_pushboolean(L, result);
	return 1;   /* ���ؽ���ĸ��� */
}
static const struct luaL_Reg lua_jikong_lib_test[] = {
	{"send",  only_test_bool},
	{NULL,    NULL}
};


/*
 * ע���
 * �ÿ��Ǽ����ṩ�Ŀɱ�lua�ű�ʹ�õĿ��ƽӿ�
 */
int lua_register_jikong_lib(lua_State *L)
{
	luaL_openlib(L, "test",      lua_jikong_lib_test,     0);

	luaL_openlib(L, "serial",    lua_jikong_lib_serial,   0);
	luaL_openlib(L, "infrared",  lua_jikong_lib_infrared, 0);
	luaL_openlib(L, "relay",     lua_jikong_lib_relay,    0);
	luaL_openlib(L, "telnet",    lua_jikong_lib_telnet,   0);

	luaL_openlib(L, "wol",       lua_jikong_lib_wol,      0);
	luaL_openlib(L, "http",      lua_jikong_lib_http,     0);
	luaL_openlib(L, "onvif",     lua_jikong_lib_onvif,    0);
	luaL_openlib(L, "ui",        lua_jikong_lib_ui,       0);
	luaL_openlib(L, "slider",    lua_jikong_lib_slider,   0);
	luaL_openlib(L, "progress",  lua_jikong_lib_progress, 0);
	luaL_openlib(L, "checkbox",  lua_jikong_lib_checkbox, 0);
	luaL_openlib(L, "radiobox",  lua_jikong_lib_radiobox, 0);






	//lua_pushcfunction(L, serialPortWrite);
	//lua_setglobal(L, "serialPortWrite");
	return 1;
}


#if 0
int transferChars(const char * data,char *buff)
{
	int nRet = 0;
	if(data == 0)
        return nRet;
    int nLen = strlen(data);
    char dataBuff[nLen + 1];
    memcpy(dataBuff,data,nLen);
    dataBuff[nLen] = '\0';

    char *result = NULL;
    char delims[] = ",";
    result = strtok(dataBuff,delims);
    nLen = 0;
    while(result != NULL)
    {
        nLen = strlen(result);
        if(nLen >= 2 && result[0] == '0' && (result[1] == 'x' || result[1] == 'X'))
            buff[nRet] = strtol(result,NULL,16);
        else
            buff[nRet] = strtol(result,NULL,10);

        result = strtok(NULL,delims);
        nRet++;
    }

    return nRet;
}
#endif
