find_package(SoapySDR CONFIG)

if (NOT SoapySDR_FOUND)
    message(WARNING "SoapySDR development files not found - skipping support")
    return()
endif ()

add_executable(${PROJECT_NAME} main.cpp DeviceManagerRtl.cpp DeviceStreamRtl.cpp DataQueue.cpp DataHandler.cpp)

set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

target_include_directories(${PROJECT_NAME} PUBLIC ${SOAPY_SDR_INCLUDE_DIR})

target_link_libraries(${PROJECT_NAME} SoapySDR)
