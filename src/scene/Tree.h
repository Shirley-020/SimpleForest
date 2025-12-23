#ifndef TREE_H
#define TREE_H

#include <glm/glm.hpp>
#include <vector>

struct Tree {
    glm::vec3 position;
    float scale; // 随机缩放因子
};

std::vector<Tree> generateRandomTrees(
    int treeCount, float xMin = -12.0f, float xMax = 12.0f,
    float zMin = -20.0f, float zMax = 20.0f,
    float houseXRange = 4.0f, float houseZRange = 4.0f,
    float minDistance = 2.0f);

#endif // TREE_H

