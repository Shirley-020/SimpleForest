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
# 4. 主程序（删除无效变量，精简配置）
# -----------------------------------------------------
add_executable(${PROJECT_NAME}
    src/main.cpp
)

# 头文件路径：仅保留必要项，删除${GLM_INCLUDE_DIR}
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/include  # 手动添加的GLM头文件路径（核心）
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)

# 链接依赖（保持不变）
target_link_libraries(${PROJECT_NAME} PRIVATE
    glfw
    glad_lib
    imgui_lib
)

find_package(OpenGL REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE OpenGL::GL)

target_compile_definitions(${PROJECT_NAME} PRIVATE IMGUI_IMPL_OPENGL_LOADER_GLAD)

# 复制shaders（固定管线可注释，可编程管线保留）
add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/shaders $<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders
)