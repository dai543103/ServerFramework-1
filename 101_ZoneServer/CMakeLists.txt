file(GLOB_RECURSE SRC_FILES *.c *.cpp *.h *.hpp)

set(CURRENT_INC ${CMAKE_CURRENT_SOURCE_DIR})

if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    add_executable(${ZONE_SVR_EXE} ${SRC_FILES})
	include_directories(${CURRENT_INC})
    target_link_libraries(${ZONE_SVR_EXE} ${ALL_LAS} ${PUGIXML_LIB})
else()
    add_executable(${ZONE_SVR_EXE} ${SRC_FILES})
	include_directories(${CURRENT_INC})
    target_link_libraries(${ZONE_SVR_EXE} ${ALL_LAS} ${PUGIXML_LIB})
endif()

install(TARGETS ${ZONE_SVR_EXE}  DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/Make)
