// ml_config.cpp -- voir ml_config.h.
#include "ml_config.h"

#include "json.hpp"

namespace ml {
namespace {

using Json = nlohmann::ordered_json;

// Sans exceptions et commentaires toleres, comme le loader et la DLL : un
// fichier qu'ils acceptent, le launcher l'accepte aussi. Pour reecrire, sans
// commentaires (dump() les effacerait).
bool parse(const std::string& text, Json* out, bool comments = true) {
    if (text.find_first_not_of(" \t\r\n") == std::string::npos) {
        *out = Json::object();
        return true;
    }
    *out = Json::parse(text, nullptr, false, comments);
    return !out->is_discarded() && out->is_object();
}

} // namespace

bool settings_read(const std::string& text, Settings* out) {
    *out = Settings();
    Json j;
    if (!parse(text, &j)) return false;

    auto g = j.find("game");
    if (g != j.end() && g->is_string() && !g->get<std::string>().empty())
        out->game = g->get<std::string>();

    auto n = j.find("net");
    if (n != j.end() && n->is_object()) {
        auto s = n->find("steam");
        if (s != n->end() && s->is_boolean()) out->steam = s->get<bool>();
    }
    return true;
}

bool settings_write(const std::string& text, const Settings& s, std::string* out) {
    Json j;
    if (!parse(text, &j, false)) return false;

    j["game"] = s.game;

    auto n = j.find("net");
    bool has = n != j.end() && n->is_object() && n->contains("steam");
    if (has || !s.steam) {
        if (n == j.end() || !n->is_object()) j["net"] = Json::object();
        j["net"]["steam"] = s.steam;
    }

    // ensure_ascii faux : les accents restent lisibles dans le Bloc-notes.
    const std::string t = j.dump(2, ' ', false, Json::error_handler_t::replace) + "\n";
    // Le fichier livre est en CRLF (paquet/) : on garde ses fins de ligne. dump()
    // n'ecrit jamais de \r brut (echappe dans les chaines).
    const bool crlf = text.find("\r\n") != std::string::npos;
    out->clear();
    for (char c : t) {
        if (c == '\n' && crlf) *out += '\r';
        *out += c;
    }
    return true;
}

bool appid_ok(const std::string& content) {
    const char* ws = " \t\r\n";
    size_t a = content.find_first_not_of(ws);
    if (a == std::string::npos) return false;
    size_t b = content.find_last_not_of(ws);
    return content.compare(a, b - a + 1, kAppId) == 0;
}

} // namespace ml
