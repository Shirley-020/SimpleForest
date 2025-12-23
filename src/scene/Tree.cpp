#include "Tree.h"
#include <glm/gtc/type_ptr.hpp>
#include <cmath>
#include <cstdlib>

std::vector<Tree> generateRandomTrees(
    int treeCount, float xMin, float xMax,
    float zMin, float zMax,
    float houseXRange, float houseZRange,
    float minDistance) {

    std::vector<Tree> trees;

    while (trees.size() < treeCount) {
        float randX = xMin + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (xMax - xMin)));
        float randZ = zMin + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (zMax - zMin)));

        // 避开房子区域（增加安全距离，确保树不会太靠近房子）
        // 使用更大的缓冲区范围，让树与房子保持更远的距离
        if (std::abs(randX) < houseXRange && std::abs(randZ) < houseZRange) {
            continue;
        }

        // 检查与其他树木的距离
        bool isTooClose = false;
        for (const auto& tree : trees) {
            float distance = glm::distance(glm::vec2(randX, randZ), glm::vec2(tree.position.x, tree.position.z));
            if (distance < minDistance) {
                isTooClose = true;
                break;
            }
        }

        if (!isTooClose) {
            // 生成随机缩放因子 
            float randomScale = 1.5f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX)) * 0.5f;
            trees.push_back({ glm::vec3(randX, 0.0f, randZ), randomScale });
        }
    }
    return trees;
}

