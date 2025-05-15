cmake_minimum_required(VERSION 3.16)
project(proxy_maker)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

set(BUILD_SHARED_LIBS ON)

FetchContent_Declare(
  PDFHummus
  URL https://github.com/galkahana/PDF-Writer/archive/refs/tags/v4.6.2.tar.gz
  URL_HASH SHA256=0a36815ccc9d207028567f90039785c824b211169ba5da68de84d0c15455ab62
  DOWNLOAD_EXTRACT_TIMESTAMP FALSE
  FIND_PACKAGE_ARGS
)
FetchContent_MakeAvailable(PDFHummus)

include(FetchContent)
FetchContent_Declare(
        opencv
        GIT_REPOSITORY https://github.com/opencv/opencv.git
        GIT_TAG 4.11.0
)
FetchContent_MakeAvailable(opencv)


add_executable(proxy_maker)

set_property(TARGET proxy_maker PROPERTY
             MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")

target_sources(proxy_maker
        PRIVATE ${PROJECT_SOURCE_DIR}/main.cpp
        PRIVATE ${PROJECT_SOURCE_DIR}/configuration.cpp
)

target_include_directories(proxy_maker 
PRIVATE ${PROJECT_SOURCE_DIR}/build/_deps/pdfhummus-src
PRIVATE ${OPENCV_CONFIG_FILE_INCLUDE_DIR}
PRIVATE ${OPENCV_MODULE_opencv_core_LOCATION}/include
PRIVATE ${OPENCV_MODULE_opencv_imgcodecs_LOCATION}/include
)

#target_link_directories(proxy_maker 
#PRIVATE ${PROJECT_SOURCE_DIR}/opencv/build/lib/$<CONFIG>/
#)

target_link_libraries (proxy_maker 
PDFHummus::PDFWriter
opencv_imgcodecs
opencv_core
opencv_imgproc
)