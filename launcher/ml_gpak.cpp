// ml_gpak.cpp -- voir ml_gpak.h pour le format.
#include "ml_gpak.h"

namespace ml {
namespace {

constexpr uint32_t kMaxEntries = 1u << 20;   // 19900 dans ce build
constexpr uint16_t kMaxNameLen = 1024;

template <class T> bool get(std::ifstream& f, T& v) {
    return (bool)f.read(reinterpret_cast<char*>(&v), sizeof(v));
}

} // namespace

bool Gpak::open(const std::filesystem::path& path) {
    index_.clear();
    f_.close();
    f_.clear();
    f_.open(path, std::ios::binary);
    if (!f_) { error_ = u8"impossible d'ouvrir " + path.u8string(); return false; }

    f_.seekg(0, std::ios::end);
    const uint64_t file_size = (uint64_t)(std::streamoff)f_.tellg();
    f_.seekg(0);

    uint32_t count = 0;
    if (!get(f_, count) || count == 0 || count > kMaxEntries) {
        error_ = u8"l'archive ne commence pas par un nombre d'entrées plausible";
        return false;
    }

    std::string name;
    uint64_t offset = 0;
    for (uint32_t i = 0; i < count; ++i) {
        uint16_t len = 0;
        uint32_t size = 0;
        if (!get(f_, len) || len > kMaxNameLen) { error_ = u8"index illisible"; index_.clear(); return false; }
        name.resize(len);
        if (!f_.read(&name[0], len) || !get(f_, size)) { error_ = u8"index tronqué"; index_.clear(); return false; }
        index_[name] = Entry{offset, size};
        offset += size;
    }

    // Les donnees commencent apres l'index entier (GPak+0x304 dans le jeu).
    const uint64_t base = (uint64_t)(std::streamoff)f_.tellg();
    if (base + offset > file_size) { error_ = u8"archive tronquée"; index_.clear(); return false; }
    for (auto& e : index_) e.second.offset += base;
    error_.clear();
    return true;
}

bool Gpak::read(const std::string& name, std::vector<uint8_t>& out) {
    out.clear();
    auto it = index_.find(name);
    if (it == index_.end()) { error_ = name + u8" absent de l'archive"; return false; }
    const Entry e = it->second;
    if (e.size > kMaxEntry) { error_ = name + u8" : taille invraisemblable"; return false; }

    out.resize(e.size);
    f_.clear();
    f_.seekg((std::streamoff)e.offset);
    if (!f_.read(reinterpret_cast<char*>(out.data()), e.size)) {
        out.clear();
        error_ = name + u8" : lecture courte";
        return false;
    }
    return true;
}

} // namespace ml
