/******************************************************************************

                  ��Ȩ���� (C), 2001-2020, ���������ſƼ��ɷ����޹�˾

 ******************************************************************************
  �ļ����� : tempfile.c
  ����    :
  �������� :

  �汾    : 1.0
  �������� :�ṩ�˶�̬��Ľӿ�


  ˵����

      ��ģ������lua�ű����ű������̶ֹ�Ϊ��controlprogram.lua����Ҫ��
      �ýű���ʵ����������������

		function __jikong_initialize()
			--
		end

		function __jikong_uninitialize()
			--
		end

		function __jikong_control_action(ctrl_id, action, data)
			--
		end

		function __jikong_response_data(port_type, port_no, data)
			--
		end

******************************************************************************/
/*
 * /home/flx/developer/workspace_c/ccc_server/so4ui
 */

#include <string.h>
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "devi_wrap4lua.h"


#define LUA_CUSTOM_LUA_SCRIPT             "controlprogram.lua"
#define LUA_CUSTOM_FUNC_NAME_INIT         "__jikong_initialize"
#define LUA_CUSTOM_FUNC_NAME_UN_INIT      "__jikong_uninitialize"
#define LUA_CUSTOM_FUNC_NAME_ACTION       "__jikong_control_action"
#define LUA_CUSTOM_FUNC_NAME_RESPONSE     "__jikong_response_data"


static int nLoad = -1;
static lua_State *L;


static int jikong_script_is_load_success()
{
	if(nLoad != 0)
	{
		printf("load lua script failed :%s\n", LUA_CUSTOM_LUA_SCRIPT);
		return 0;
	}
	return 1;
}
/*
 * ����lua�ű��еĳ�ʼ������
 */
static int call_lua_func_jikong_initialize()
{
	int result = 0;
	if(!jikong_script_is_load_success())
		return -1;

	lua_getglobal(L, LUA_CUSTOM_FUNC_NAME_INIT);
	if(lua_isfunction(L, -1))
	{
		if(lua_pcall(L, 0, 1, 0) == 0)
		{
			printf("lua call suceess \n");
		}
		else
		{
			printf("%s:%d, call failed: %s\n", __FILE__, __LINE__, LUA_CUSTOM_FUNC_NAME_INIT);
			printf("%s\n", lua_tostring(L,-1));
			result = -1;
		}
		lua_pop(L,1);
	}
	else
	{
		printf("%s is not a function\n", LUA_CUSTOM_FUNC_NAME_INIT);
	}

	return result;
}
/*
 * ����lua�ű��еķ���ʼ������
 */
static int call_lua_func_jikong_uninitialize()
{
	int result = 0;
	if(!jikong_script_is_load_success())
		return -1;

	lua_getglobal(L, LUA_CUSTOM_FUNC_NAME_UN_INIT);
	if(lua_isfunction(L, -1))
	{
		if(lua_pcall(L, 0, 1, 0) == 0)
		{
			printf("lua call suceess \n");
		}
		else
		{
			printf("%s:%d, call failed: %s\n", __FILE__, __LINE__, LUA_CUSTOM_FUNC_NAME_UN_INIT);
			printf("%s\n", lua_tostring(L,-1));
			result = -1;
		}
		lua_pop(L,1);
	}
	else
	{
		printf("%s is not a function\n", LUA_CUSTOM_FUNC_NAME_UN_INIT);
	}

	return result;
}
/*
 * ����lua�ű��еĿؼ�����������
 */
static int call_lua_func_jikong_control_action(int senderId, int event, const char *data, int len)
{
	int result = 0;
	if(!jikong_script_is_load_success())
		return -1;

	lua_getglobal(L, LUA_CUSTOM_FUNC_NAME_ACTION);
	if(lua_isfunction(L, -1))
	{
		lua_pushnumber(L, senderId);
		lua_pushnumber(L, event);
		lua_pushlstring(L, data, len);
		if(lua_pcall(L, 3, 1, 0) == 0)
		{
			printf("lua call suceess \n");
		}
		else
		{
			printf("call failed: %s, %s\n",LUA_CUSTOM_FUNC_NAME_ACTION, lua_tostring(L,-1));
			result = -1;
		}
		lua_pop(L,1);
	}
	else
	{
		printf("%s is not a function\n", LUA_CUSTOM_FUNC_NAME_ACTION);
	}

	return result;
}
/*
 * ����lua�ű��еĴ��� ���� �ĺ���
 */
static int call_lua_func_jikong_response_data(int port_type, int port_no, const char *buffer, int data_len)
{
	int result = 0;
	if(!jikong_script_is_load_success())
		return -1;

	lua_getglobal(L, LUA_CUSTOM_FUNC_NAME_RESPONSE);
	if(lua_isfunction(L, -1))
	{
		lua_pushnumber(L, port_type);
		lua_pushnumber(L, port_no);
		lua_pushlstring(L, buffer, data_len);
		if(lua_pcall(L, 3, 1, 0) == 0)
		{
			printf("lua call suceess \n");
		}
		else
		{
			printf("call failed: %s, %s\n", LUA_CUSTOM_FUNC_NAME_RESPONSE, lua_tostring(L,-1));
			result = -1;
		}
		lua_pop(L,1);
	}
	else
	{
		printf("%s is not a function\n", LUA_CUSTOM_FUNC_NAME_RESPONSE);
	}

	return result;
}

/*
 * �������Դ��ڵķ���
 * ֻ��Ϊ�˼���ԭ���Ľӿ�
 */
int dealwithSerialportData(int port, const unsigned char *data,int len)
{
	return call_lua_func_jikong_response_data(1, port, (char *)data, len);
}
/* ������ */
int ProcessResponseData(int port_type, int port_no, const unsigned char *buffer, int data_len)
{
	return call_lua_func_jikong_control_action(port_type, port_no, (char *)buffer, data_len);
}
/*
 * ԭ����char *data
 */
int ProcessEvent(int senderId, int event, const char *data, int len)
{
	return call_lua_func_jikong_control_action(senderId, event, data, len);
}

int ReleaseControls()
{
	call_lua_func_jikong_uninitialize();
    lua_close(L);
    return 0;
}

int InitializeControls()
{
    L = lua_open();
	luaL_openlibs(L);
	lua_register_jikong_lib(L);
	
	if(luaL_loadfile(L, LUA_CUSTOM_LUA_SCRIPT) || lua_pcall(L, 0, 0, 0))
	{
		printf("load lua file failed :%s. %s\n", LUA_CUSTOM_LUA_SCRIPT, lua_tostring(L, -1));
		lua_pop(L, 1);
        return -1;
	}
    else
    {
		nLoad = 0;
        printf("loadfile success\n");
    }
    lua_pop(L,1);

    if(call_lua_func_jikong_initialize() != 0)
    	return -1;

	return 0;
}




#if 0
int dealwithSerialportData(int port,unsigned char *data,int len)
{
	if(nLoad == 0)
	{
		lua_getglobal(L,"DealwithSerialportData");
		if(lua_isfunction(L,-1))
		{
			lua_pushnumber(L,  port);
			lua_pushlstring(L, data, len);
			lua_pushnumber(L,  len);
			if(lua_pcall(L,3,1,0) == 0)
			{	
				printf("lua call suceess \n");
			}
			else
			{
				fprintf(stderr, "lua dealwithSerialportData failed :portId:%d,err:%s\n",port,lua_tostring(L,-1)); 
			}
			lua_pop(L,1);
		}
		else
		{
			printf("DealwithSerialportdata is not a function\n");
		}
		
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
				lua_pushlstring(L,data,len);
				lua_pushnumber(L,len);

				if(lua_pcall(L,4,1,0) == 0)
				{
					printf("lua call suceess \n");
				}else{
					fprintf(stderr, "lua call failed :senderId:%d, event:%d,err:%s\n",senderId,event,lua_tostring(L,-1));
				}
				lua_pop(L, 1);
			}
		}else
		{
			printf("not call it\n");
		}
	}

	return 0;
}
#endif
