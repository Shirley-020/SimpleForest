cmake_minimum_required(VERSION 3.15)
# 新增C语言支持（适配glad.c）
project(ForestHouseDemo LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# 给C文件指定标准（glad.c用C11）
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include(FetchContent)

# -----------------------------------------------------
# 1. GLFW（保持不变）
# -----------------------------------------------------
FetchContent_Declare(
    glfw
    URL https://github.com/glfw/glfw/archive/refs/tags/3.3.8.zip
)
# 禁用GLFW不必要的构建（优化编译）
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

# -----------------------------------------------------
# 2. ImGui（添加GLFW依赖关联）
# -----------------------------------------------------
FetchContent_Declare(
    imgui
    URL https://github.com/ocornut/imgui/archive/refs/tags/v1.90.4.zip
)
FetchContent_MakeAvailable(imgui)

set(IMGUI_SRC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

add_library(imgui_lib ${IMGUI_SRC})
target_include_directories(imgui_lib PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
    ${glfw_SOURCE_DIR}/include  # 给ImGui添加GLFW头文件路径
)
target_link_libraries(imgui_lib PUBLIC glfw)  # ImGui链接GLFW，避免未定义引用

# -----------------------------------------------------
# 3. glad（保持不变，已适配C语言）
# -----------------------------------------------------
add_library(glad_lib
    src/glad.c
)
target_include_directories(glad_lib PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

# -----------------------------------------------------
# 3.5. Assimp（模型加载库）- 使用项目内本地文件
# -----------------------------------------------------
# Assimp 文件路径配置（项目内本地路径）
set(ASSIMP_ROOT_DIR ${CMAKE_SOURCE_DIR}/include/assimp)
set(ASSIMP_INCLUDE_DIR ${ASSIMP_ROOT_DIR}/include)
set(ASSIMP_LIB_DIR ${ASSIMP_ROOT_DIR}/lib)
set(ASSIMP_BIN_DIR ${ASSIMP_ROOT_DIR}/bin)
set(ASSIMP_LIB_NAME assimp-vc143-mt)
set(ASSIMP_LIBRARY ${ASSIMP_LIB_DIR}/${ASSIMP_LIB_NAME}.lib)
set(ASSIMP_DLL ${ASSIMP_BIN_DIR}/${ASSIMP_LIB_NAME}.dll)

# 检查 Assimp 文件是否存在
set(assimp_FOUND FALSE)
set(ASSIMP_MISSING_FILES "")

# 检查头文件
if(EXISTS "${ASSIMP_INCLUDE_DIR}/assimp/Importer.hpp" AND 
   EXISTS "${ASSIMP_INCLUDE_DIR}/assimp/scene.h")
    message(STATUS "Assimp header files found: ${ASSIMP_INCLUDE_DIR}")
else()
    list(APPEND ASSIMP_MISSING_FILES "Header files in ${ASSIMP_INCLUDE_DIR}")
    message(WARNING "Assimp header files not found in: ${ASSIMP_INCLUDE_DIR}")
endif()

# 检查库文件
if(EXISTS "${ASSIMP_LIBRARY}")
    message(STATUS "Assimp library found: ${ASSIMP_LIBRARY}")
else()
    list(APPEND ASSIMP_MISSING_FILES "Library file: ${ASSIMP_LIBRARY}")
    message(WARNING "Assimp library not found: ${ASSIMP_LIBRARY}")
endif()

# 检查 DLL 文件（Windows）
if(WIN32)
    if(EXISTS "${ASSIMP_DLL}")
        message(STATUS "Assimp DLL found: ${ASSIMP_DLL}")
    else()
        list(APPEND ASSIMP_MISSING_FILES "DLL file: ${ASSIMP_DLL}")
        message(WARNING "Assimp DLL not found: ${ASSIMP_DLL}")
    endif()
endif()

# 如果所有必需文件都存在，标记为找到
if(EXISTS "${ASSIMP_INCLUDE_DIR}/assimp/Importer.hpp" AND 
   EXISTS "${ASSIMP_INCLUDE_DIR}/assimp/scene.h" AND
   EXISTS "${ASSIMP_LIBRARY}")
    set(assimp_FOUND TRUE)
    message(STATUS "=== Assimp configuration (local files) ===")
    message(STATUS "  Root directory: ${ASSIMP_ROOT_DIR}")
    message(STATUS "  Include directory: ${ASSIMP_INCLUDE_DIR}")
    message(STATUS "  Library: ${ASSIMP_LIBRARY}")
    if(WIN32)
        message(STATUS "  DLL: ${ASSIMP_DLL}")
    endif()
    message(STATUS "=== Assimp found successfully! ===")
else()
    message(ERROR "=== Assimp configuration failed! ===")
    message(ERROR "Missing files:")
    foreach(missing_file ${ASSIMP_MISSING_FILES})
        message(ERROR "  - ${missing_file}")
    endforeach()
    message(ERROR "Please ensure all Assimp files are in the correct locations:")
    message(ERROR "  - Headers: ${ASSIMP_INCLUDE_DIR}/assimp/")
    message(ERROR "  - Library: ${ASSIMP_LIBRARY}")
    if(WIN32)
        message(ERROR "  - DLL: ${ASSIMP_DLL}")
    endif()
    message(ERROR "Model loading will be disabled.")
endif()

# -----------------------------------------------------
# 4. 主程序（模块化重构后的文件）
# -----------------------------------------------------
add_executable(${PROJECT_NAME}
    src/main.cpp
    
    # Core modules
    src/core/Shader.cpp
    src/core/Camera.cpp
    src/core/Texture.cpp
    src/core/Model.cpp
    src/core/PathUtils.cpp
    
    # Geometry modules
    src/geometry/Mesh.cpp
    src/geometry/PrimitiveFactory.cpp
    
    # Scene modules
    src/scene/Materials.cpp
    src/scene/Tree.cpp
    src/scene/HouseRenderer.cpp
    
    # Input module
    src/input/Input.cpp
)

# 头文件路径：包含所有模块目录
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/include  # GLM、stb等第三方库
    ${CMAKE_SOURCE_DIR}/include/assimp/include  # Assimp 头文件路径
    ${CMAKE_SOURCE_DIR}/src      # 模块化头文件路径
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)

# 链接依赖（保持不变）
target_link_libraries(${PROJECT_NAME} PRIVATE
    glfw
    glad_lib
    imgui_lib
)

# 链接 Assimp（如果找到）
if(assimp_FOUND)
    # 链接 Assimp 库
    target_link_libraries(${PROJECT_NAME} PRIVATE ${ASSIMP_LIBRARY})
    message(STATUS "Assimp library linked: ${ASSIMP_LIBRARY}")
    
    # 添加编译定义
    target_compile_definitions(${PROJECT_NAME} PRIVATE ASSIMP_AVAILABLE)
    message(STATUS "=== Assimp linked successfully. Model loading enabled. ===")
    
    # Windows: 自动复制 DLL 文件到程序输出目录
    if(WIN32 AND EXISTS "${ASSIMP_DLL}")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${ASSIMP_DLL}"
                "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
            COMMENT "Copying Assimp DLL to output directory"
        )
        message(STATUS "Assimp DLL will be copied to: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
    elseif(WIN32)
        message(WARNING "Assimp DLL not found, runtime may fail. Expected: ${ASSIMP_DLL}")
    endif()
else()
    target_compile_definitions(${PROJECT_NAME} PRIVATE ASSIMP_UNAVAILABLE)
    message(STATUS "=== Assimp not linked. Model loading disabled. Procedural trees will be used. ===")
endif()

find_package(OpenGL REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE OpenGL::GL)

target_compile_definitions(${PROJECT_NAME} PRIVATE IMGUI_IMPL_OPENGL_LOADER_GLAD)

# 复制shaders和objects目录
add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/shaders $<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders
    COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:${PROJECT_NAME}>/objects
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/objects $<TARGET_FILE_DIR:${PROJECT_NAME}>/objects
)