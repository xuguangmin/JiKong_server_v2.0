#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <pthread.h>
#include "util_log.h"


typedef int (*INITIALIZE_PARSER)();
typedef int (*RELEASE_PARSER)();
typedef int (*PARSER_CONTROL_EVENT)(int senderId, int event, char *data, int data_len);
typedef int (*PARSER_SERIAL_RECV)(int serial_no, unsigned char *data, int data_len);
typedef int (*PARSER_CONNECT_RECV)(int controlId, unsigned char *data, int data_len);

static INITIALIZE_PARSER    pf_initialize_parser   = NULL;
static RELEASE_PARSER       pf_release_parser      = NULL;
static PARSER_CONTROL_EVENT pf_parse_control_event = NULL;
static PARSER_SERIAL_RECV   pf_parse_serial_recv   = NULL;
static PARSER_CONNECT_RECV  pf_parse_connect_recv  = NULL;


static void *g_hLibrary = NULL;
static char *g_dl_filename = NULL;
static pthread_mutex_t lib_mutex = PTHREAD_MUTEX_INITIALIZER;


static void ui_library_lock()
{
	pthread_mutex_lock(&lib_mutex);
}
static void ui_library_unlock()
{
	pthread_mutex_unlock(&lib_mutex);
}

static int save_dl_filename(const char *ui_event_so_filename)
{
	int len = strlen(ui_event_so_filename);
	if(!ui_event_so_filename || len <= 0)
	{
		CCC_LOG_OUT("save_dl_filename() invalid filename\n");
		return 0;
	}
	if(g_dl_filename) free(g_dl_filename);
	g_dl_filename = (char *)malloc(len + 1 + 10);
	if(!g_dl_filename)
	{
		CCC_LOG_OUT("save_dl_filename() malloc error\n");
		return 0;
	}

	sprintf(g_dl_filename, "./%s", ui_event_so_filename);
	return 1;
}

static int check_loadlib_error()
{
	char *error;
	if ((error = dlerror()) != NULL)
	{
		CCC_LOG_OUT("%s check_loadlib_error : %s\n", g_dl_filename, error);
		return 0;
	}
	return 1;
}

static int load_ui_library(const char *ui_event_so_filename)
{
	g_hLibrary = NULL;
	if(!save_dl_filename(ui_event_so_filename))
		return 0;

	g_hLibrary = dlopen(g_dl_filename, RTLD_LAZY);
	if(NULL == g_hLibrary)
	{
		CCC_LOG_OUT("%s dlopen error.\n %s \n", g_dl_filename, dlerror());
		return 0;
	}

	pf_parse_serial_recv = dlsym(g_hLibrary, "dealwithSerialportData");
	if (NULL == pf_parse_serial_recv){if (!check_loadlib_error()) return 0;}

	pf_initialize_parser = dlsym(g_hLibrary, "InitializeControls");
	if (NULL == pf_initialize_parser){if (!check_loadlib_error()) return 0;}

	pf_release_parser = dlsym(g_hLibrary, "ReleaseControls");
	if (NULL == pf_release_parser){if (!check_loadlib_error()) return 0;}

	pf_parse_control_event = dlsym(g_hLibrary, "ProcessEvent");
	if (NULL == pf_parse_control_event){if (!check_loadlib_error()) return 0;}

	// pf_parse_connect_recv = dlsym(g_hLibrary, "dealwithNetworkData");
	//if (NULL == pf_parse_connect_recv){if (!check_loadlib_error()) return 0;}

	CCC_LOG_OUT("load library %s success\n", g_dl_filename);
	return 1;
}

static int interval_initialize_parser()
{
	if(!pf_initialize_parser)
	{
		CCC_LOG_OUT("pf_initialize_parser = NULL\n");
		return 0;
	}

	if(pf_initialize_parser() != 0)
	{
		CCC_LOG_OUT("initialize_parser error\n");
		return 0;
	}
	return 1;
}

static int interval_release_parser()
{
	if(!pf_release_parser)
	{
		CCC_LOG_OUT("pf_release_parser = NULL\n");
		return 0;
	}

	if(pf_release_parser() != 0)
	{
		CCC_LOG_OUT("release_parser error\n");
		return 0;
	}
	return 1;
}
static int unload_ui_library()
{
	pf_parse_control_event = NULL;
	pf_parse_serial_recv   = NULL;
	pf_release_parser      = NULL;
	pf_initialize_parser   = NULL;

	if(g_hLibrary)
	{
		while(dlclose(g_hLibrary));           /* 直到dlclose的计数为0 */
		g_hLibrary = NULL;

		CCC_LOG_OUT("unload %s success\n", g_dl_filename);
		if(g_dl_filename)
		{
			free(g_dl_filename);
			g_dl_filename = NULL;
		}
	}
	return 1;
}

static int interval_parse_control_event(int senderId, int event, char *data, int dataLen)
{
	if(!pf_parse_control_event)
	{
		CCC_LOG_OUT("pf_parse_control_event = NULL\n");
		return 0;
	}

	if(pf_parse_control_event(senderId, event, data, dataLen) != 0)
	{
		CCC_LOG_OUT("parse_control_event error\n");
		return 0;
	}

	//CCC_LOG_OUT("pf_parse_control_event senderId %d, event %d\n", senderId, event);
	return 1;
}

static int interval_parse_serial_recv(int serial_no, unsigned char *data, int dataLen)
{
	if(!pf_parse_serial_recv)
	{
		CCC_LOG_OUT("pf_parse_serial_recv = NULL\n");
		return 0;
	}

	if(pf_parse_serial_recv(serial_no, data, dataLen) != 0)
	{
		CCC_LOG_OUT("parse_serial_recv error\n");
		return 0;
	}
	return 1;
}

static int interval_parse_connect_recv(int controlId, unsigned char *data, int dataLen)
{
	if(!pf_parse_connect_recv)
	{
		CCC_LOG_OUT("pf_parse_connect_recv = NULL\n");
		return 0;
	}

	if(pf_parse_connect_recv(controlId, data, dataLen) != 0)
	{
		CCC_LOG_OUT("parse_connect_recv error\n");
		return 0;
	}
	return 1;
}

int parse_connect_recv(int controlId, unsigned char *data, int dataLen)
{
	int result;
	ui_library_lock();
	result = interval_parse_connect_recv(controlId, data, dataLen);
	ui_library_unlock();
	return result;
}

int parse_serial_recv(int serial_no, unsigned char *data, int dataLen)
{
	int result;
	ui_library_lock();
	result = interval_parse_serial_recv(serial_no, data, dataLen);
	ui_library_unlock();
	return result;
}

int parse_control_event(int senderId, int event, char *data, int dataLen)
{
	int result;
	ui_library_lock();
	result = interval_parse_control_event(senderId, event, data, dataLen);
	ui_library_unlock();
	return result;
}

int initialize_ui_library(const char *ui_event_so_filename)
{
	ui_library_lock();
	if(!load_ui_library(ui_event_so_filename))
	{
		ui_library_unlock();
		CCC_LOG_OUT("load_ui_library() %s error\n", ui_event_so_filename);
		return 0;
	}

	if(!interval_initialize_parser())
	{
		ui_library_unlock();
		return 0;
	}

	ui_library_unlock();
	return 1;
}

int release_ui_library()
{
	ui_library_lock();
	if(!interval_release_parser())
	{
		ui_library_unlock();
		return 0;
	}

	if(!unload_ui_library())
	{
		ui_library_unlock();
		return 0;
	}

	ui_library_unlock();
	return 1;
}


