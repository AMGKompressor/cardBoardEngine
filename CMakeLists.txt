cmake_minimum_required(VERSION 3.16)
project(cardBoardEngine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CARDBOARD_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")
set(GP_LIB "${CARDBOARD_ROOT}/lib")

set(FRAMEWORK_SOURCES
  framework/logmanager.cpp
  framework/matrix4.cpp
  framework/renderer.cpp
  framework/shader.cpp
  framework/sprite.cpp
  framework/texture.cpp
  framework/texturemanager.cpp
  framework/vertexarray.cpp
)

set(ENGINE_SOURCES
  src/main.cpp
  src/CardBoardApp.cpp
  src/Collision2D.cpp
  src/Map/Map.cpp
  src/Player/Player.cpp
  src/Enemy/Enemy.cpp
  src/Enemy/EnemySpawner.cpp
)

add_executable(cardboard ${FRAMEWORK_SOURCES} ${ENGINE_SOURCES})

target_include_directories(cardboard PRIVATE
  "${CARDBOARD_ROOT}/framework"
  "${CARDBOARD_ROOT}/src"
  "${CARDBOARD_ROOT}/src/Map"
  "${CARDBOARD_ROOT}/src/Player"
  "${CARDBOARD_ROOT}/src/Enemy"
)

set(GP_INCLUDES
  "${GP_LIB}/SDL2-2.0.22/include"
  "${GP_LIB}/SDL2_image-2.6.1/include"
  "${GP_LIB}/glew-2.1.0/include"
)
target_include_directories(cardboard BEFORE PRIVATE ${GP_INCLUDES})

target_link_directories(cardboard PRIVATE
  "${GP_LIB}/SDL2-2.0.22/lib/x86"
  "${GP_LIB}/SDL2_image-2.6.1/lib/x86"
  "${GP_LIB}/glew-2.1.0/lib/x86"
)

target_link_libraries(cardboard PRIVATE
  SDL2
  SDL2main
  SDL2_image
  glew32
  opengl32
)

if(MSVC)
  target_compile_options(cardboard PRIVATE /W3)
endif()

add_custom_command(
  TARGET cardboard POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          "${CARDBOARD_ROOT}/assets/shaders"
          "$<TARGET_FILE_DIR:cardboard>/shaders"
  COMMENT "Copying cardBoard shaders"
)
add_custom_command(
  TARGET cardboard POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          "${CARDBOARD_ROOT}/assets/textures"
          "$<TARGET_FILE_DIR:cardboard>/textures"
  COMMENT "Copying cardBoard textures"
)
