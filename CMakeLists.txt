cmake_minimum_required(VERSION 3.16)
project(proxy_maker)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

FetchContent_Declare(
  PDFHummus
  URL https://github.com/galkahana/PDF-Writer/archive/refs/tags/v4.6.2.tar.gz
  URL_HASH SHA256=0a36815ccc9d207028567f90039785c824b211169ba5da68de84d0c15455ab62
  DOWNLOAD_EXTRACT_TIMESTAMP FALSE
  FIND_PACKAGE_ARGS
)
FetchContent_MakeAvailable(PDFHummus)

add_executable(proxy_maker)

target_sources(proxy_maker
        PRIVATE ${PROJECT_SOURCE_DIR}/main.cpp
)

set(${OPENCV_MODULES} ${PROJECT_SOURCE_DIR}/build/_deps/opencv-src/modules)

target_include_directories(proxy_maker 
PRIVATE ${PROJECT_SOURCE_DIR}/build/_deps/pdfhummus-src
)

target_link_libraries (proxy_maker PDFHummus::PDFWriter)