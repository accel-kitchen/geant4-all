#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
# CMakeLists.txt: ミュオグラフィ検出器シミュレーション (geant4-underground)
#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

set( NAME_MAIN_PROGRAM Application_Main )
project( ${NAME_MAIN_PROGRAM} )

# 可視化 (Qt/OpenGL) は既定で無効。WSL2 では Qt がバッチ終了後に固まるため、
# 必要なときだけ -DWITH_GEANT4_UIVIS=ON を付けてビルドし、--vis で起動する。
option(WITH_GEANT4_UIVIS "Build with Geant4 UI and Vis drivers" OFF)
if(WITH_GEANT4_UIVIS)
  find_package(Geant4 REQUIRED ui_all vis_all)
  add_definitions(-DMUO_WITH_UIVIS)
else()
  find_package(Geant4 REQUIRED)
endif()

include(${Geant4_USE_FILE})
include_directories(${PROJECT_SOURCE_DIR}/include)

file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cc)
file(GLOB headers ${PROJECT_SOURCE_DIR}/include/*.hh)

add_executable(${NAME_MAIN_PROGRAM} ${NAME_MAIN_PROGRAM}.cc ${sources} ${headers})
target_link_libraries(${NAME_MAIN_PROGRAM} ${Geant4_LIBRARIES})

set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR}/..)
install(TARGETS ${NAME_MAIN_PROGRAM} DESTINATION bin)
