#ifndef PATH_UTILS_H
#define PATH_UTILS_H

#include <string>

// 获取资源文件路径（简化版本，直接返回相对路径）
// CMake 会将 objects 和 shaders 目录复制到可执行文件目录
std::string getResourcePath(const std::string& relativePath);

#endif // PATH_UTILS_H
