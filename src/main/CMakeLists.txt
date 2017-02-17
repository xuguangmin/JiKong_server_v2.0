CONFIGURE_FILE(ccc_config.h     ${CUSTOM_INCLUDE_DIR_MAIN}/ccc_config.h)

ADD_EXECUTABLE(ccc_server ccc_main.c indicator_lamp.c)
# sequence must be :
#                   libtps_tcp_server.a; in libtcp_server.a
#                   libfile_deal.a; libconfigure.a; libsqlite3.a;
#                   libcurl.a; -lrt;
TARGET_LINK_LIBRARIES(ccc_server  libconsole.a; libccc_cli.a; libjunction.a; libprotocol_adapter.a; libprotocol.a; libtps_tcp_server.a; libtcp_server.a; libdata_pool.a; libpacket_pool.a; libthread.a; libfile_deal.a; libui_library_wrapper.a; libdevice_controller.a; libconfigure.a;  libutils.a; libsqlite3.a; libghttp.a; -ldl; -lpthread)

ADD_EXECUTABLE(ccc_server_monitor ccc_monitor.c)

# ------------ install ------------
install(TARGETS ccc_server ccc_server_monitor RUNTIME DESTINATION release)
