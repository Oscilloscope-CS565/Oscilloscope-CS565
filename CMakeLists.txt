cmake_minimum_required(VERSION 3.16)
project(OscilloscopeCS565Qt LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

set(IO_LIBRARY_SOURCES
    ioLibrary/ioBuffer.cpp
    ioLibrary/ioRead.cpp
    ioLibrary/ioWrite.cpp
    ioLibrary/ioFtdiDevice.cpp
    ioLibrary/ioCircularBuffer.cpp
    ioLibrary/ioThreadedReader.cpp
    ioLibrary/ioThreadedWriter.cpp
    ioLibrary/ioScaleShiftPipeline.cpp
)

set(UI_SOURCES
    qt_main.cpp
    ui/ioOscilloscopeModel.cpp
    ui/ioWaveformWidget.cpp
    ui/ioCompactOscilloscopeView.cpp
    ui/ioWorkspaceOscilloscopeView.cpp
    ui/ioMainWindow.cpp
)

set(UI_HEADERS
    ui/ioOscilloscopeModel.h
    ui/ioWaveformWidget.h
    ui/ioCompactOscilloscopeView.h
    ui/ioWorkspaceOscilloscopeView.h
    ui/ioMainWindow.h
)

add_executable(oscilloscope_qt ${IO_LIBRARY_SOURCES} ${UI_SOURCES} ${UI_HEADERS})

target_include_directories(oscilloscope_qt PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/ioLibrary
    ${CMAKE_CURRENT_SOURCE_DIR}/ui
)

target_link_libraries(oscilloscope_qt PRIVATE Qt6::Widgets)

if(APPLE)
    target_link_libraries(oscilloscope_qt PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/libftd2xx.a
        "-framework CoreFoundation"
        "-framework IOKit"
    )
elseif(WIN32)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        target_link_libraries(oscilloscope_qt PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/amd64/ftd2xx.lib
            setupapi
        )
    else()
        target_link_libraries(oscilloscope_qt PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/i386/ftd2xx.lib
            setupapi
        )
    endif()
else()
    target_link_libraries(oscilloscope_qt PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/libftd2xx.a
    )
endif()
