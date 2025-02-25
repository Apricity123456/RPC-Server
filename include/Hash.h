#include <iostream>
#include <string>
struct FileHash {
    std::string m_filepath;
    std::string m_sha1;
    FileHash(std::string filepath);
};