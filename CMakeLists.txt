cmake_minimum_required(VERSION 3.16)
project(cardBoardEngine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(CARDBOARD_BUILD_FULL_GAME
  "Build the full Visual Studio game target (Game, Scene, ImGui) as 'tgp'"
  ON)

set(CARDBOARD_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")
set(GP_LIB "${CARDBOARD_ROOT}/lib")

if(EXISTS "${CARDBOARD_ROOT}/game/shaders")
  set(CARDBOARD_SHADER_DIR "${CARDBOARD_ROOT}/game/shaders")
elseif(EXISTS "${CARDBOARD_ROOT}/assets/shaders")
  set(CARDBOARD_SHADER_DIR "${CARDBOARD_ROOT}/assets/shaders")
else()
  message(FATAL_ERROR "No shader directory found (expected game/shaders or assets/shaders).")
endif()

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
)

add_executable(cardboard ${FRAMEWORK_SOURCES} ${ENGINE_SOURCES})

target_include_directories(cardboard PRIVATE
  "${CARDBOARD_ROOT}/framework"
  "${CARDBOARD_ROOT}/src"
  "${CARDBOARD_ROOT}/src/Map"
  "${CARDBOARD_ROOT}/src/Player"
)

if(WIN32)
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
else()
  if(APPLE)
    execute_process(
      COMMAND brew --prefix
      OUTPUT_VARIABLE HOMEBREW_PREFIX
      OUTPUT_STRIP_TRAILING_WHITESPACE
      ERROR_QUIET
    )
    if(HOMEBREW_PREFIX)
      list(APPEND CMAKE_PREFIX_PATH "${HOMEBREW_PREFIX}")
      list(APPEND CMAKE_PREFIX_PATH "${HOMEBREW_PREFIX}/opt/sdl2")
      list(APPEND CMAKE_PREFIX_PATH "${HOMEBREW_PREFIX}/opt/sdl2_image")
      list(APPEND CMAKE_PREFIX_PATH "${HOMEBREW_PREFIX}/opt/glew")
    endif()
  endif()

  find_package(OpenGL REQUIRED)
  find_package(SDL2 CONFIG REQUIRED)

  set(_sdl2_image_linked FALSE)
  find_package(SDL2_image CONFIG QUIET)
  if(TARGET SDL2_image::SDL2_image)
    set(_sdl2_image_linked TRUE)
  endif()

  if(NOT _sdl2_image_linked)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SDL2_IMAGE_PC REQUIRED IMPORTED_TARGET SDL2_image)
  endif()

  find_package(GLEW REQUIRED)

  target_link_libraries(cardboard PRIVATE SDL2::SDL2)

  if(TARGET SDL2_image::SDL2_image)
    target_link_libraries(cardboard PRIVATE SDL2_image::SDL2_image)
  else()
    target_link_libraries(cardboard PRIVATE PkgConfig::SDL2_IMAGE)
  endif()

  if(TARGET GLEW::GLEW)
    target_link_libraries(cardboard PRIVATE GLEW::GLEW)
  else()
    target_include_directories(cardboard PRIVATE ${GLEW_INCLUDE_DIRS})
    target_link_libraries(cardboard PRIVATE ${GLEW_LIBRARIES})
  endif()

  target_link_libraries(cardboard PRIVATE OpenGL::GL)
endif()

if(MSVC)
  target_compile_options(cardboard PRIVATE /W3)
endif()

function(cardboard_stage_runtime_assets target_name)
  add_custom_command(
    TARGET ${target_name} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CARDBOARD_SHADER_DIR}"
            "$<TARGET_FILE_DIR:${target_name}>/shaders"
    COMMENT "Copying shaders for ${target_name}"
  )
  add_custom_command(
    TARGET ${target_name} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CARDBOARD_ROOT}/assets/textures"
            "$<TARGET_FILE_DIR:${target_name}>/textures"
    COMMENT "Copying textures for ${target_name}"
  )
endfunction()

cardboard_stage_runtime_assets(cardboard)

if(CARDBOARD_BUILD_FULL_GAME)
  set(IMGUI_SOURCES
    lib/imgui/imgui.cpp
    lib/imgui/imgui_draw.cpp
    lib/imgui/imgui_tables.cpp
    lib/imgui/imgui_widgets.cpp
    lib/imgui/imgui_impl_sdl2.cpp
    lib/imgui/imgui_impl_opengl3.cpp
  )

  set(TGP_SOURCES
    main.cpp
    Game.cpp
    AnimatedSprite.cpp
    Collision2D.cpp
    Collision2D_1.cpp
    InputSystem.cpp
    Inventory.cpp
    Item.cpp
    Enemy.cpp
    EnemyManager.cpp
    NavGrid.cpp
    logmanager.cpp
    Map.cpp
    Map_1.cpp
    Minimap.cpp
    Matrix4.cpp
    MouseCursor.cpp
    Player.cpp
    Player_1.cpp
    Renderer.cpp
    Scene.cpp
    SceneCardBoard.cpp
    Shader.cpp
    SoundSystem.cpp
    Sprite.cpp
    Texture.cpp
    TextureManager.cpp
    UI.cpp
    Vector2.cpp
    VertexArray.cpp
    XboxController.cpp
    ${IMGUI_SOURCES}
  )

  add_executable(tgp ${TGP_SOURCES})

  target_include_directories(tgp PRIVATE
    "${CARDBOARD_ROOT}"
    "${CARDBOARD_ROOT}/lib/imgui"
    "${CARDBOARD_ROOT}/lib/FMOD/include"
  )

  if(WIN32)
    target_include_directories(tgp BEFORE PRIVATE
      "${GP_LIB}/SDL2-2.0.22/include"
      "${GP_LIB}/SDL2_image-2.6.1/include"
      "${GP_LIB}/glew-2.1.0/include"
    )
    target_link_directories(tgp PRIVATE
      "${GP_LIB}/SDL2-2.0.22/lib/x86"
      "${GP_LIB}/SDL2_image-2.6.1/lib/x86"
      "${GP_LIB}/glew-2.1.0/lib/x86"
      "${GP_LIB}/FMOD/lib/x86"
    )
    target_link_libraries(tgp PRIVATE
      SDL2 SDL2main SDL2_image glew32 opengl32 fmod_vc
    )
  else()
    if(APPLE)
      execute_process(
        COMMAND brew --prefix
        OUTPUT_VARIABLE HOMEBREW_PREFIX
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
      )
      if(HOMEBREW_PREFIX)
        list(APPEND CMAKE_PREFIX_PATH "${HOMEBREW_PREFIX}")
        list(APPEND CMAKE_PREFIX_PATH "${HOMEBREW_PREFIX}/opt/sdl2")
        list(APPEND CMAKE_PREFIX_PATH "${HOMEBREW_PREFIX}/opt/sdl2_image")
        list(APPEND CMAKE_PREFIX_PATH "${HOMEBREW_PREFIX}/opt/glew")
      endif()
    endif()

    find_package(OpenGL REQUIRED)
    find_package(SDL2 CONFIG REQUIRED)

    set(_sdl2_image_linked FALSE)
    find_package(SDL2_image CONFIG QUIET)
    if(TARGET SDL2_image::SDL2_image)
      set(_sdl2_image_linked TRUE)
    endif()
    if(NOT _sdl2_image_linked)
      find_package(PkgConfig REQUIRED)
      pkg_check_modules(SDL2_IMAGE_PC REQUIRED IMPORTED_TARGET SDL2_image)
    endif()

    find_package(GLEW REQUIRED)

    target_link_libraries(tgp PRIVATE SDL2::SDL2)
    if(TARGET SDL2_image::SDL2_image)
      target_link_libraries(tgp PRIVATE SDL2_image::SDL2_image)
    else()
      target_link_libraries(tgp PRIVATE PkgConfig::SDL2_IMAGE)
    endif()
    if(TARGET GLEW::GLEW)
      target_link_libraries(tgp PRIVATE GLEW::GLEW)
    else()
      target_include_directories(tgp PRIVATE ${GLEW_INCLUDE_DIRS})
      target_link_libraries(tgp PRIVATE ${GLEW_LIBRARIES})
    endif()
    target_link_libraries(tgp PRIVATE OpenGL::GL)

    find_library(FMOD_LIBRARY NAMES fmod PATHS
      /opt/homebrew/lib /usr/local/lib)
    if(FMOD_LIBRARY)
      target_link_libraries(tgp PRIVATE ${FMOD_LIBRARY})
    else()
      target_compile_definitions(tgp PRIVATE CARDBOARD_NO_FMOD=1)
      message(STATUS "FMOD not found — building tgp without audio (install FMOD Studio SDK for sound).")
    endif()
  endif()

  if(MSVC)
    target_compile_options(tgp PRIVATE /W3)
  endif()

  cardboard_stage_runtime_assets(tgp)
endif()
