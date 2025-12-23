#include "PathUtils.h"
#include <iostream>
#include <fstream>

std::string getResourcePath(const std::string& relativePath) {
    // 简化版本：直接返回相对路径
    // CMake 会将资源文件复制到可执行文件目录，所以相对路径应该可以工作
    return relativePath;
}

