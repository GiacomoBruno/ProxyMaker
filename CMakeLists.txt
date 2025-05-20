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

FetchContent_Declare(
        opencv
        QUIET
        GIT_REPOSITORY https://github.com/opencv/opencv.git
        GIT_TAG 4.11.0
)
FetchContent_MakeAvailable(opencv)

FetchContent_Declare(
        magic_enum
        GIT_REPOSITORY https://github.com/Neargye/magic_enum.git
)

FetchContent_Populate(magic_enum)

FetchContent_Declare(
    ESRGAN
    URL https://github.com/xinntao/Real-ESRGAN-ncnn-vulkan/releases/download/v0.2.0/realesrgan-ncnn-vulkan-v0.2.0-windows.zip
    DOWNLOAD_EXTRACT_TIMESTAMP FALSE

)

FetchContent_Populate(ESRGAN)

FetchContent_Declare(
    ThreadPool
    GIT_REPOSITORY https://github.com/bshoshany/thread-pool.git
    DOWNLOAD_EXTRACT_TIMESTAMP FALSE
)

FetchContent_Populate(ThreadPool)


add_executable(proxy_maker)

target_sources(proxy_maker
        PRIVATE ${PROJECT_SOURCE_DIR}/main.cpp
        PRIVATE ${PROJECT_SOURCE_DIR}/utility.cpp
        PRIVATE ${PROJECT_SOURCE_DIR}/configuration.cpp
        PRIVATE ${PROJECT_SOURCE_DIR}/paper.cpp
        PRIVATE ${PROJECT_SOURCE_DIR}/image_processing.cpp
        PRIVATE ${PROJECT_SOURCE_DIR}/pdf_generation.cpp
)

target_include_directories(proxy_maker 
PRIVATE ${PROJECT_SOURCE_DIR}/build/_deps/pdfhummus-src
PRIVATE ${OPENCV_CONFIG_FILE_INCLUDE_DIR}
PRIVATE ${OPENCV_MODULE_opencv_core_LOCATION}/include
PRIVATE ${OPENCV_MODULE_opencv_imgcodecs_LOCATION}/include
PRIVATE ${OPENCV_MODULE_opencv_imgproc_LOCATION}/include
PRIVATE ${PROJECT_SOURCE_DIR}/build/_deps/magic_enum-src/include/magic_enum
PRIVATE ${PROJECT_SOURCE_DIR}/build/_deps/threadpool-src/include/
)

add_custom_command(TARGET proxy_maker POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PROJECT_SOURCE_DIR}/build/_deps/pdfhummus-build/PDFWriter/$<CONFIG>/"
        $<TARGET_FILE_DIR:proxy_maker>)

add_custom_command(TARGET proxy_maker POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PROJECT_SOURCE_DIR}/build/_deps/pdfhummus-build/FreeType/$<CONFIG>/"
        $<TARGET_FILE_DIR:proxy_maker>)

add_custom_command(TARGET proxy_maker POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PROJECT_SOURCE_DIR}/build/_deps/pdfhummus-build/LibAesgm/$<CONFIG>/"
        $<TARGET_FILE_DIR:proxy_maker>)

add_custom_command(TARGET proxy_maker POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PROJECT_SOURCE_DIR}/build/_deps/pdfhummus-build/LibJpeg/$<CONFIG>/"
        $<TARGET_FILE_DIR:proxy_maker>)

add_custom_command(TARGET proxy_maker POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PROJECT_SOURCE_DIR}/build/_deps/pdfhummus-build/LibPng/$<CONFIG>/"
        $<TARGET_FILE_DIR:proxy_maker>)

add_custom_command(TARGET proxy_maker POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PROJECT_SOURCE_DIR}/build/_deps/pdfhummus-build/LibTiff/$<CONFIG>/"
        $<TARGET_FILE_DIR:proxy_maker>)

add_custom_command(TARGET proxy_maker POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PROJECT_SOURCE_DIR}/build/_deps/pdfhummus-build/Zlib/$<CONFIG>/"
        $<TARGET_FILE_DIR:proxy_maker>)
        
add_custom_command(TARGET proxy_maker POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    "${PROJECT_SOURCE_DIR}/build/_deps/esrgan-src/realesrgan-ncnn-vulkan.exe"
    $<TARGET_FILE_DIR:proxy_maker>)

add_custom_command(TARGET proxy_maker POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    "${PROJECT_SOURCE_DIR}/build/_deps/esrgan-src/vcomp140.dll"
    $<TARGET_FILE_DIR:proxy_maker>)

add_custom_command(TARGET proxy_maker POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    "${PROJECT_SOURCE_DIR}/build/_deps/esrgan-src/vcomp140d.dll"
    $<TARGET_FILE_DIR:proxy_maker>)

add_custom_command(TARGET proxy_maker POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        $<TARGET_FILE_DIR:proxy_maker>
        "${PROJECT_SOURCE_DIR}/release/bin/")


        

target_link_libraries (proxy_maker 
PDFHummus::PDFWriter
opencv_imgcodecs
opencv_core
opencv_imgproc
)