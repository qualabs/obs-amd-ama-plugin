cmake_minimum_required(VERSION 3.16...3.26)

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/common/bootstrap.cmake" NO_POLICY_SCOPE)

project(${_name} VERSION ${_version})

# set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
option(ENABLE_FRONTEND_API "Use obs-frontend-api for UI functionality" OFF)
option(ENABLE_QT "Use Qt functionality" OFF)

include(compilerconfig)
include(defaults)
include(helpers)
include_directories(/usr/include/obs/ /opt/amd/ama/ma35/include/xma /opt/amd/ama/ma35/include/xrm
                    /opt/amd/ama/ma35/include/xrm_interface)
link_directories(/opt/amd/ama/ma35/lib)
set(CMAKE_INSTALL_RPATH "/opt/amd/ama/ma35/lib")

add_library(${CMAKE_PROJECT_NAME} MODULE)
target_link_libraries(
  ${CMAKE_PROJECT_NAME}
  PUBLIC hugetlbfs-2.23
  PUBLIC xma
  PUBLIC xrm
  PUBLIC xrm_interface
  PUBLIC xrm_encoder_plugin)

find_package(libobs REQUIRED)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE OBS::libobs)

if(ENABLE_FRONTEND_API)
  find_package(obs-frontend-api REQUIRED)
  target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE OBS::obs-frontend-api)
endif()

if(ENABLE_QT)
  find_qt(COMPONENTS Widgets Core)
  target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE Qt::Core Qt::Widgets)
  target_compile_options(
    ${CMAKE_PROJECT_NAME} PRIVATE $<$<C_COMPILER_ID:Clang,AppleClang>:-Wno-quoted-include-in-framework-header
                                  -Wno-comma>)
  set_target_properties(
    ${CMAKE_PROJECT_NAME}
    PROPERTIES AUTOMOC ON
               AUTOUIC ON
               AUTORCC ON)
endif()

target_sources(${CMAKE_PROJECT_NAME} PRIVATE src/ama-plugin-main.c src/ama-filter.c src/ama-encoder.c src/ama-scaler.c
                                             src/ama-context.c)

set_target_properties_plugin(${CMAKE_PROJECT_NAME} PROPERTIES OUTPUT_NAME ${_name})
