#include "VFSXmlParser.hpp"
#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <fstream>

std::string base64Decode(const std::string& in) {
    std::string out;
    std::vector<int> T(256, -1);
    const std::string b64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    for (int i = 0; i < 64; i++) T[b64[i]] = i;

    int val = 0, valb = -8;
    for (unsigned char c : in) {
        if (T[c] == -1) continue;
        val = (val << 6) + T[c];
        valb += 6;
        if (valb >= 0) {
            out.push_back(char((val >> valb) & 0xFF));
            valb -= 8;
        }
    }
    return out;
}


bool VFSXmlParser::loadVFS(const std::string& xmlPath, VFS& vfs) {
    std::ifstream file(xmlPath);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open VFS XML file: " << xmlPath << "\n";
        return false;
    }

    std::string xmlContent((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    size_t pos = 0;

    size_t vfsStart = xmlContent.find("<vfs>", pos);
    if (vfsStart == std::string::npos) {
        std::cerr << "Error: Invalid XML format. Root <vfs> tag not found.\n";
        return false;
    }

    pos = vfsStart + 5;
    return parseChildren(xmlContent, pos, vfs.root);
}

std::string VFSXmlParser::extractAttribute(const std::string& tag, const std::string& attrName) {
    std::string key = attrName + "=\"";
    size_t start = tag.find(key);
    if (start == std::string::npos) return "";
    start += key.length();
    size_t end = tag.find("\"", start);
    if (end == std::string::npos) return "";
    return tag.substr(start, end - start);
}

bool VFSXmlParser::parseChildren(const std::string& xml, size_t& pos, const std::shared_ptr<VFSNode>& parent) {
    while (pos < xml.length()) {
        size_t tagOpen = xml.find('<', pos);
        if (tagOpen == std::string::npos) break;

        size_t tagClose = xml.find('>', tagOpen);
        if (tagClose == std::string::npos) break;

        std::string tagContent = xml.substr(tagOpen + 1, tagClose - tagOpen - 1);
        pos = tagClose + 1;

        if (!tagContent.empty() && tagContent[0] == '/') {
            return true;
        }

        // Обработка директории: <dir name="...">
        if (tagContent.rfind("dir", 0) == 0) {
            std::string dirName = extractAttribute(tagContent, "name");
            if (dirName.empty()) dirName = "unnamed_dir";

            auto dirNode = std::make_shared<VFSNode>(dirName, true);
            dirNode->parent = parent;
            parent->children[dirName] = dirNode;

            parseChildren(xml, pos, dirNode);
        }
        // Обработка файла: <file name="..." encoding="...">content</file>
        else if (tagContent.rfind("file", 0) == 0) {
            std::string fileName = extractAttribute(tagContent, "name");
            std::string encoding = extractAttribute(tagContent, "encoding");
            if (fileName.empty()) fileName = "unnamed_file";

            size_t fileEnd = xml.find("</file>", pos);
            if (fileEnd == std::string::npos) break;

            std::string rawData = xml.substr(pos, fileEnd - pos);
            pos = fileEnd + 7; // Длина "</file>"

            // Чистка лишних пробелов/переносов строк
            size_t first = rawData.find_first_not_of(" \t\r\n");
            size_t last = rawData.find_last_not_of(" \t\r\n");
            std::string cleanData = (first == std::string::npos) ? "" : rawData.substr(first, (last - first + 1));

            std::string finalContent = (encoding == "base64") ? base64Decode(cleanData) : cleanData;

            auto fileNode = std::make_shared<VFSNode>(fileName, false);
            fileNode->content = finalContent;
            fileNode->parent = parent;
            parent->children[fileName] = fileNode;
        }
    }
    return true;
}