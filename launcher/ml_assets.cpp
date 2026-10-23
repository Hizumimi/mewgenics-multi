// ml_assets.cpp -- voir ml_assets.h.
#include "ml_assets.h"

#include "ml_gpak.h"

#include <cstring>

namespace ml {
namespace {

struct WantedImage { uint16_t id; uint32_t w, h; Image Assets::*dst; const char* what; };
const WantedImage kImages[] = {
    {1816, 1984, 1134, &Assets::house,      u8"la maison sur la colline"},
    {3805, 2000, 2000, &Assets::sun,        u8"le soleil"},
    {1,     920,  923, &Assets::paper,      u8"la feuille de papier"},
    {3629, 1922, 1122, &Assets::paper_wide, u8"la feuille de papier large"},
};

struct WantedFont { const char* prefix; Font Assets::*dst; };
const WantedFont kFonts[] = {
    {"Peralta",           &Assets::title},
    {"TikaFont Adjusted", &Assets::text},
    {"EdmundMcMillen",    &Assets::hand},
};

bool open_swf(Gpak& gpak, const char* name, Swf& swf, Assets& a) {
    std::vector<uint8_t> file;
    std::string err;
    if (!gpak.read(name, file)) { a.problems.push_back(gpak.error()); return false; }
    if (!swf_open(std::move(file), swf, err)) { a.problems.push_back(std::string(name) + " : " + err); return false; }
    return true;
}

void load_images(Gpak& gpak, Assets& a) {
    Swf swf;
    if (!open_swf(gpak, "swfs/ui.swf", swf, a)) return;
    bool seen[sizeof(kImages) / sizeof(kImages[0])] = {};
    const bool whole = swf_each_tag(swf, [&](const SwfTag& tag) {
        uint16_t id = 0;
        if (tag.code != 36 || !swf_char_id(tag, id)) return true;
        for (size_t i = 0; i < sizeof(kImages) / sizeof(kImages[0]); ++i) {
            const WantedImage& w = kImages[i];
            if (w.id != id || seen[i]) continue;
            seen[i] = true;
            Image img;
            std::string err;
            if (!swf_decode_bitmap(tag, img, err))
                a.problems.push_back(std::string(u8"ui.swf : ") + w.what + " : " + err);
            else if (img.w != w.w || img.h != w.h)
                a.problems.push_back(std::string(u8"ui.swf : ") + w.what + u8" n'a pas la taille attendue (" +
                                     std::to_string(img.w) + "x" + std::to_string(img.h) + ")");
            else
                a.*w.dst = std::move(img);
        }
        for (bool s : seen) if (!s) return true;
        return false;   // les quatre sont la : inutile de lire la suite
    });
    if (!whole) a.problems.push_back(u8"ui.swf : fichier tronqué");
    for (size_t i = 0; i < sizeof(kImages) / sizeof(kImages[0]); ++i)
        if (!seen[i]) a.problems.push_back(std::string(u8"ui.swf : ") + kImages[i].what + u8" est introuvable");
}

void load_fonts(Gpak& gpak, Assets& a) {
    Swf swf;
    if (!open_swf(gpak, "swfs/fonts.swf", swf, a)) return;
    const bool whole = swf_each_tag(swf, [&](const SwfTag& tag) {
        std::string name;
        if (!swf_font_name(tag, name)) return true;
        for (const WantedFont& w : kFonts) {
            Font& dst = a.*w.dst;
            if (!dst.glyphs.empty() || name.compare(0, strlen(w.prefix), w.prefix) != 0) continue;
            std::string err;
            if (!swf_decode_font(tag, dst, err)) {
                a.problems.push_back(u8"fonts.swf : " + name + " : " + err);
                dst = Font{};
            }
        }
        return true;
    });
    if (!whole) a.problems.push_back(u8"fonts.swf : fichier tronqué");
    for (const WantedFont& w : kFonts)
        if ((a.*w.dst).glyphs.empty())
            a.problems.push_back(std::string(u8"fonts.swf : la police ") + w.prefix + u8" est introuvable ou vide");
}

} // namespace

std::filesystem::path default_gpak_path() {
    return std::filesystem::u8path(
        u8"C:\\Program Files (x86)\\Steam\\steamapps\\common\\Mewgenics\\resources.gpak");
}

Assets load_assets(const std::filesystem::path& gpak_path) {
    Assets a;
    Gpak gpak;
    if (!gpak.open(gpak_path)) {
        a.problems.push_back(u8"resources.gpak : " + gpak.error());
        return a;
    }
    load_images(gpak, a);
    load_fonts(gpak, a);
    return a;
}

std::future<Assets> load_assets_async(std::filesystem::path gpak) {
    return std::async(std::launch::async, [p = std::move(gpak)] { return load_assets(p); });
}

} // namespace ml
