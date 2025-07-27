cmake_minimum_required(VERSION 3.16)
project(proxy_maker)

include(homebrew.cmake)

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
set(OpenCV_DIR "/opt/homebrew/Cellar/opencv/4.12.0/lib/cmake/opencv4/")
find_package(OpenCV REQUIRED)

message(STATUS "OpenCV library status:")
message(STATUS "    config: ${OpenCV_DIR}")
message(STATUS "    version: ${OpenCV_VERSION}")
message(STATUS "    libraries: ${OpenCV_LIBS}")
message(STATUS "    include path: ${OpenCV_INCLUDE_DIRS}")

FetchContent_Declare(
        magic_enum
        GIT_REPOSITORY https://github.com/Neargye/magic_enum.git
)

FetchContent_MakeAvailable(magic_enum)

FetchContent_Declare( 
    ThreadPool
    GIT_REPOSITORY https://github.com/bshoshany/thread-pool.git
    DOWNLOAD_EXTRACT_TIMESTAMP FALSE
)

FetchContent_MakeAvailable(ThreadPool)

add_executable(proxy_maker)

target_sources(proxy_maker
        PRIVATE ${PROJECT_SOURCE_DIR}/main.cpp
        PRIVATE ${PROJECT_SOURCE_DIR}/utility.cpp
        PRIVATE ${PROJECT_SOURCE_DIR}/configuration.cpp
        PRIVATE ${PROJECT_SOURCE_DIR}/image_processing.cpp
        PRIVATE ${PROJECT_SOURCE_DIR}/pdf_generation.cpp
        PRIVATE ${PROJECT_SOURCE_DIR}/paper.cpp
        PRIVATE ${PROJECT_SOURCE_DIR}/card.cpp
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
        "${PROJECT_SOURCE_DIR}/build/_deps/pdfhummus-build/PDFWriter/"
        $<TARGET_FILE_DIR:proxy_maker>)

add_custom_command(TARGET proxy_maker POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PROJECT_SOURCE_DIR}/build/_deps/pdfhummus-build/FreeType/"
        $<TARGET_FILE_DIR:proxy_maker>)

add_custom_command(TARGET proxy_maker POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PROJECT_SOURCE_DIR}/build/_deps/pdfhummus-build/LibAesgm/"
        $<TARGET_FILE_DIR:proxy_maker>)

add_custom_command(TARGET proxy_maker POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PROJECT_SOURCE_DIR}/build/_deps/pdfhummus-build/LibJpeg/"
        $<TARGET_FILE_DIR:proxy_maker>)

add_custom_command(TARGET proxy_maker POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PROJECT_SOURCE_DIR}/build/_deps/pdfhummus-build/LibPng/"
        $<TARGET_FILE_DIR:proxy_maker>)

add_custom_command(TARGET proxy_maker POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PROJECT_SOURCE_DIR}/build/_deps/pdfhummus-build/LibTiff/"
        $<TARGET_FILE_DIR:proxy_maker>)

add_custom_command(TARGET proxy_maker POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PROJECT_SOURCE_DIR}/build/_deps/pdfhummus-build/Zlib/"
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