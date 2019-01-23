file(GLOB SRC_FILES *.hpp *.h *.cpp *.cxx *.c libghttp/*c libghttp/*h)

add_executable(${EXCHANGE_SVR_EXE} ${SRC_FILES})

set(CMAKE_INCLUDE_CURRENT_DIR ON)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libghttp)
target_link_libraries(${EXCHANGE_SVR_EXE} ${ALL_LAS} ${MYSQLCLIENT_LIB} ${PUGIXML_LIB})

install(TARGETS ${EXCHANGE_SVR_EXE} DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})
