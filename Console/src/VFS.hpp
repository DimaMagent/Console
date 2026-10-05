#pragma once
#pragma once
#include <memory>
#include <string>
#include <map>

struct VFSNode : std::enable_shared_from_this<VFSNode> {
    std::string name;
    bool isDirectory = false;
    std::string content; // Двоичные/текстовые данные
    std::weak_ptr<VFSNode> parent;
    std::map<std::string, std::shared_ptr<VFSNode>> children;

    VFSNode(std::string name, bool isDir) : name(std::move(name)), isDirectory(isDir) {}

    std::string getFullPath() const;
};

class VFS {
public:
    std::shared_ptr<VFSNode> root;

    VFS();

    void printTree(const std::shared_ptr<VFSNode>& node, int depth = 0) const;
    std::shared_ptr<VFSNode> resolvePath(const std::string& path, const std::shared_ptr<VFSNode>& cwd) const;
};