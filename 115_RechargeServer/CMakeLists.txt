file(GLOB SRC_FILES *.hpp *.h *.cpp *.cxx *.c)

add_executable(${RECHARGE_SVR_EXE} ${SRC_FILES})

set(CMAKE_INCLUDE_CURRENT_DIR ON)
target_link_libraries(${RECHARGE_SVR_EXE} ${ALL_LAS})

install(TARGETS ${RECHARGE_SVR_EXE} DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})
