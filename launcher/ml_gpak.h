// ml_gpak.h -- Mewgenics Multi (launcher) : lire des entrees de resources.gpak.
//
// Meme format que src/core/mgmp_gpak.h (GPak::LoadIndex @ 0x140A434D0) :
//
//   u32 count
//   count x { u16 namelen, char name[namelen], u32 size }   // offsets cumules
//   ...donnees, a partir de la fin de l'index
//
// La difference avec la DLL : le launcher lit plusieurs entrees (ui.swf puis
// fonts.swf), donc l'index est garde en memoire une fois lu, et le fichier est
// ouvert par un chemin donne (le launcher tourne avant le jeu, pas dedans).
//
// L'archive fait 4,9 Go : les offsets sont sur 64 bits (std::streamoff l'est
// avec MSVC comme avec g++). Une entree n'est jamais lue que seule, jamais
// l'archive entiere. Aucune dependance a Windows : teste sous Linux aussi.
#pragma once

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace ml {

class Gpak {
public:
    // Ouvre l'archive et lit tout l'index (~20000 noms, ~700 Ko). Faux si le
    // fichier manque ou si l'index ne tient pas debout ; error() dit pourquoi.
    bool open(const std::filesystem::path& path);

    // Lit une entree entiere ("swfs/ui.swf"). Faux si l'archive n'est pas
    // ouverte, si le nom manque, si l'entree depasse kMaxEntry ou si la
    // lecture est courte ; `out` est alors vide.
    bool read(const std::string& name, std::vector<uint8_t>& out);

    bool has(const std::string& name) const { return index_.count(name) != 0; }
    size_t size() const { return index_.size(); }
    const std::string& error() const { return error_; }

    static constexpr uint32_t kMaxEntry = 256u * 1024u * 1024u;   // ui.swf : 45 Mo

private:
    struct Entry { uint64_t offset; uint32_t size; };
    std::ifstream f_;
    std::unordered_map<std::string, Entry> index_;
    std::string error_;
};

} // namespace ml
