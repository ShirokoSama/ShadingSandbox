cmake_minimum_required(VERSION 3.1)
project(sandbox)

set(CMAKE_CXX_STANDARD 11)

message(${CMAKE_GENERATOR})
message(${CMAKE_CXX_COMPILER_ID})
message(${CMAKE_SIZEOF_VOID_P})

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    # mingw-w64
    if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "8")
        set(LIB_DIRECTORY ext/lib/mingw-w64)
        # mingw
    elseif("${CMAKE_SIZEOF_VOID_P}" STREQUAL "4")
        set(LIB_DIRECTORY ext/lib/mingw)
    endif()
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    # msvc 64bit
    if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "8")
        set(LIB_DIRECTORY ext/lib/msvc-x64)
        # msvc 32bit
    elseif("${CMAKE_SIZEOF_VOID_P}" STREQUAL "4")
        set(LIB_DIRECTORY ext/lib/msvc-x86)
    endif()
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/ext/include)
link_directories(${LIB_DIRECTORY})
file(GLOB RESOURCE_DIR resource *.*)

add_executable(sandbox
        ${RESOURCE_DIR}
        # header files
        tools/common.h
        tools/ResourceManager.h
        tools/Shader.h
        tools/stb_image.h
        tools/Texture2D.h
        tools/Camera.h
        tools/Mesh.h
        tools/MerlBrdfTexture.h
        tools/CubeMapTexture.h
        src/Scene.h
        src/Skybox.h
        src/BasePipeline.h
        src/SimplePipeline.h
        src/IblPipeline.h
        src/BasePrePass.h
        src/IblPrePass.h
        tools/Cube.h
        tools/Quad.h
        # source files
        tools/common.cpp
        tools/ResourceManager.cpp
        tools/Shader.cpp
        tools/stb_image.cpp
        tools/Texture2D.cpp
        tools/Mesh.cpp
        tools/MerlBrdfTexture.cpp
        tools/CubeMapTexture.cpp
        src/Scene.cpp
        src/Skybox.cpp
        src/SimplePipeline.cpp
        src/IblPipeline.cpp
        src/IblPrePass.cpp
        tools/Cube.cpp
        tools/Quad.cpp
        src/main.cpp
        glad.c
        # shader files
        shader/phong_fs.glsl
        shader/vertex.glsl
        shader/skybox_vs.glsl
        shader/skybox_fs.glsl
        shader/beckmann_fs.glsl
        shader/ggx_fs.glsl
        shader/cubemap_vs.glsl
        shader/equirectangular_to_cubemap_fs.glsl)
target_link_libraries(sandbox glfw3dll opengl32)

add_custom_command(TARGET sandbox POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy
        "${PROJECT_SOURCE_DIR}/config.json" $<TARGET_FILE_DIR:sandbox>)
add_custom_command(TARGET sandbox POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PROJECT_SOURCE_DIR}/shader" $<TARGET_FILE_DIR:sandbox>/shader)
add_custom_command(TARGET sandbox POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PROJECT_SOURCE_DIR}/resource" $<TARGET_FILE_DIR:sandbox>/resource)
add_custom_command(TARGET sandbox POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${PROJECT_SOURCE_DIR}/${LIB_DIRECTORY}/glfw3.dll" $<TARGET_FILE_DIR:sandbox>)