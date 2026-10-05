#include "VFS.hpp"
#include <vector>
#include <sstream>
#include <iostream>
#include <fstream>

VFS::VFS() {
    root = std::make_shared<VFSNode>("/", true);
}

void VFS::printTree(const std::shared_ptr<VFSNode>& node, int depth) const {
    if (!node) return;
    std::string indent(depth * 2, ' ');
    std::cout << indent << (node->isDirectory ? "[DIR] " : "[FILE] ") << node->name << "\n";
    for (const auto& [name, child] : node->children) {
        printTree(child, depth + 1);
    }
}