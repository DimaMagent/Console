#include "VFS.hpp"
#include <vector>
#include <sstream>
#include <iostream>
#include <fstream>

std::string VFSNode::getFullPath() const {
    auto parentPtr = parent.lock();
    if (!parentPtr) return "/";
    if (parentPtr->name == "/") return "/" + name;
    return parentPtr->getFullPath() + "/" + name;
}

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

std::shared_ptr<VFSNode> VFS::resolvePath(const std::string& path, const std::shared_ptr<VFSNode>& cwd) const
{
    if (path.empty()) return cwd;

    std::shared_ptr<VFSNode> current = (path[0] == '/') ? root : cwd;

    std::stringstream ss(path);
    std::string token;
    while (std::getline(ss, token, '/')) {
        if (token.empty() || token == ".") continue;
        if (token == "..") {
            auto parentPtr = current->parent.lock();
            if (parentPtr) {
                current = parentPtr;
            }
            continue;
        }

        auto it = current->children.find(token);
        if (it != current->children.end()) {
            current = it->second;
        }
        else {
            return nullptr;
        }
    }
    return current;
}
