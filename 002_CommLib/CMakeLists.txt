file(GLOB SRC_FILES *.hpp *.cpp)

add_library(${COMMLIB_LAS}  STATIC  ${SRC_FILES})
