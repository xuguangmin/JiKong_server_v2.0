cmake_minimum_required(VERSION 2.8)
PROJECT(ccc_server)


# ------------ custom variables ------------
# custom compiler   arm-none-linux-gnueabi-gcc
SET(CUSTOM_C_COMPILER           arm-none-linux-gnueabi-gcc)

# custom project root directory
SET(CUSTOM_PROJECT_DIR          ${CMAKE_SOURCE_DIR})
# custom src directory
SET(CUSTOM_SRC_DIR              ${CUSTOM_PROJECT_DIR}/src)

# custom bin output directory
SET(CUSTOM_OUTPUT_DIR_BIN       ${CUSTOM_PROJECT_DIR}/bin)
# custom lib output directory
SET(CUSTOM_OUTPUT_DIR_LIB       ${CUSTOM_PROJECT_DIR}/lib)

# custom include directory
SET(CUSTOM_INCLUDE_DIR_MAIN     ${CUSTOM_SRC_DIR}/include)
SET(CUSTOM_INCLUDE_DIR_SQLITE   ${CUSTOM_SRC_DIR}/sqlite)
SET(CUSTOM_INCLUDE_DIR_ZLIB     ${CUSTOM_SRC_DIR}/zlib/include)
# custom link directory
SET(CUSTOM_LINK_DIR_MAIN        ${CUSTOM_OUTPUT_DIR_LIB})
SET(CUSTOM_LINK_DIR_SQLITE      ${CUSTOM_SRC_DIR}/sqlite/lib)
SET(CUSTOM_LINK_DIR_ZLIB        ${CUSTOM_SRC_DIR}/zlib/lib)



# ------------ debug ------------
ADD_DEFINITIONS(-Wall)
#SET(CMAKE_C_FLAGS -static)
#ADD_DEFINITIONS(-static)
#ADD_DEFINITIONS(-g)
#ADD_DEFINITIONS(-DDEBUG)


# ------------ set system variable ------------
# set bin output directory
SET(EXECUTABLE_OUTPUT_PATH  ${CUSTOM_OUTPUT_DIR_BIN})
# set lib output directory
SET(LIBRARY_OUTPUT_PATH     ${CUSTOM_OUTPUT_DIR_LIB})

# set compiler
SET(CMAKE_C_COMPILER        ${CUSTOM_C_COMPILER})

# set include directory
INCLUDE_DIRECTORIES(${CUSTOM_INCLUDE_DIR_MAIN})
INCLUDE_DIRECTORIES(${CUSTOM_INCLUDE_DIR_SQLITE})
INCLUDE_DIRECTORIES(${CUSTOM_INCLUDE_DIR_ZLIB})
# set link directory
LINK_DIRECTORIES(${CUSTOM_LINK_DIR_MAIN})
LINK_DIRECTORIES(${CUSTOM_LINK_DIR_SQLITE})
LINK_DIRECTORIES(${CUSTOM_LINK_DIR_ZLIB})


# ------------ target ------------
ADD_SUBDIRECTORY(src)

