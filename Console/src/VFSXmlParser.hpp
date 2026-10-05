#pragma once
#include "VFS.hpp"

std::string base64Decode(const std::string& in);

class VFSXmlParser {
public:
    static bool loadVFS(const std::string& xmlPath, VFS& vfs);

private:
    static std::string extractAttribute(const std::string& tag, const std::string& attrName);

    static bool parseChildren(const std::string& xml, size_t& pos, const std::shared_ptr<VFSNode>& parent);
};