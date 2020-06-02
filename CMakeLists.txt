cmake_minimum_required(VERSION 2.8)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{VITASDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
  else()
    message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
  endif()
endif()

project(ScoreHax)
include("${VITASDK}/share/vita.cmake" REQUIRED)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,-q -Wall -O3 -std=gnu99")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fno-rtti -fno-exceptions")

include_directories(
)

link_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
)

if (NOT ${RELEASE})
  add_definitions(-DENABLE_LOGGING)
endif()

add_executable(ScoreHax
  ScoreHax.c
)

target_link_libraries(ScoreHax
  taihen_stub
  SceLibKernel_stub
  SceAppMgr_stub
  SceCommonDialog_stub
  SceSysmodule_stub
  SceSysmem_stub
  SceAppUtil_stub
  SceIme_stub
  SceCtrl_stub
  vita2d
  SceDisplay_stub
  SceGxm_stub
  SceLibc_stub
  SceIofilemgr_stub
  SceLibKernel_stub
)

set_target_properties(ScoreHax
  PROPERTIES LINK_FLAGS "-nostdlib"
)

vita_create_self(ScoreHax.suprx ScoreHax
  CONFIG ${CMAKE_SOURCE_DIR}/ScoreHax.yml
)