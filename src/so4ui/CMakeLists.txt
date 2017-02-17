AUX_SOURCE_DIRECTORY(. SO4UI_SRC_LIST)
ADD_LIBRARY(hmui_event SHARED  ${SO4UI_SRC_LIST})

TARGET_LINK_LIBRARIES(hmui_event  libdevice_interface.a; liblua.a; -lm)

# ------------ install ------------
install(TARGETS hmui_event LIBRARY DESTINATION release)