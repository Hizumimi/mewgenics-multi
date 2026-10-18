// mm_trade_ui -- Mewgenics Multi : le bouton « Échanger » et sa fenetre. Voir mm_trade_ui.h.

#include "mm_trade_ui.h"
#include "mm_gift.h"

#include "imgui.h"

#include <algorithm>
#include <cfloat>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <set>
#include <string>
#include <vector>

namespace mgmp {
namespace {

constexpr int64_t kMaxAmount = 1000000;   // kGiftMaxMoney, kPurseGiveMax
constexpr size_t  kMaxPicked = 16;        // kGiftMaxItems

struct Ui {
    TradeView v;
    bool      have      = false;
    bool      open      = false;   // la fenetre
    bool      was_show  = false;
    int       target    = -1;      // le numero du joueur choisi
    int       coins     = 0;
    int       food      = 0;
    std::set<uint64_t> picked;     // les objets choisis (leur id)
    char      filter[64] = {};
};
Ui u;

void refresh() {
    TradeView nv;
    if (gift_view(nv, u.v.serial)) {
        u.v    = std::move(nv);
        u.have = true;
    }
}

void set_open(bool open) {
    if (u.open == open) return;
    u.open = open;
    gift_window_open(open);
}

// Une recherche sans casse (lettres ASCII ; les accents tels quels).
bool contains_ci(const std::string& hay, const char* needle) {
    const size_t n = strlen(needle);
    if (!n) return true;
    auto low = [](unsigned char c) { return (c >= 'A' && c <= 'Z') ? (unsigned char)(c + 32) : c; };
    for (size_t i = 0; i + n <= hay.size(); ++i) {
        size_t k = 0;
        while (k < n && low((unsigned char)hay[i + k]) == low((unsigned char)needle[k])) ++k;
        if (k == n) return true;
    }
    return false;
}

void draw_button(float fs) {
    const ImGuiViewport* vp = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(vp->WorkPos.x + vp->WorkSize.x - fs * 0.6f, vp->WorkPos.y + vp->WorkSize.y * 0.12f),
                            ImGuiCond_FirstUseEver, ImVec2(1.0f, 0.0f));
    ImGui::SetNextWindowBgAlpha(0.80f);
    const ImGuiWindowFlags fl = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                                ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar |
                                ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing |
                                ImGuiWindowFlags_NoCollapse;
    if (ImGui::Begin("##mm_trade_button", nullptr, fl)) {
        if (ImGui::Button(u.open ? u8"Fermer les échanges" : u8"Échanger")) set_open(!u.open);
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip(u8"Donner des pièces, de la nourriture ou des objets à un autre joueur");
    }
    ImGui::End();
}

void status_line() {
    if (u.v.status.empty()) return;
    ImGui::Separator();
    ImGui::TextWrapped("%s", u.v.status.c_str());
}

void draw_window(float fs) {
    const ImGuiViewport* vp = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(vp->WorkPos.x + vp->WorkSize.x * 0.5f, vp->WorkPos.y + vp->WorkSize.y * 0.5f),
                            ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(fs * 27.0f, fs * 31.0f), ImGuiCond_Appearing);
    bool keep = true;
    if (!ImGui::Begin(u8"Échanger###mm_trade_window", &keep, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse)) {
        ImGui::End();
        if (!keep) set_open(false);
        return;
    }
    const TradeView& v = u.v;
    if (v.mode == TradeMode::None) {
        ImGui::TextWrapped("%s", v.why_none.c_str());
        status_line();
        ImGui::End();
        if (!keep) set_open(false);
        return;
    }

    // A qui.
    const TradePeer* chosen = nullptr;
    for (const TradePeer& p : v.peers)
        if (p.id == u.target) chosen = &p;
    if (!chosen) {
        for (const TradePeer& p : v.peers)
            if (!chosen && p.can) chosen = &p;
        if (!chosen && !v.peers.empty()) chosen = &v.peers.front();
        u.target = chosen ? chosen->id : -1;
    }
    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted(u8"Donner à :");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(-FLT_MIN);
    const std::string shown = chosen ? chosen->name + (chosen->can ? "" : u8" (ne peut pas recevoir)") : u8"(personne)";
    if (ImGui::BeginCombo("##mm_trade_to", shown.c_str())) {
        for (const TradePeer& p : v.peers) {
            const std::string l = p.name + (p.can ? "" : u8" (ne peut pas recevoir)") + "##p" + std::to_string(p.id);
            if (ImGui::Selectable(l.c_str(), p.id == u.target)) u.target = p.id;
        }
        ImGui::EndCombo();
    }
    if (v.peers.empty()) ImGui::TextWrapped(u8"Personne d'autre ne peut recevoir ici.");
    else if (chosen && !chosen->can) ImGui::TextColored(ImVec4(1.0f, 0.82f, 0.35f, 1.0f), "%s", chosen->why.c_str());
    ImGui::Separator();

    // Combien.
    ImGui::TextWrapped(v.mode == TradeMode::House ? u8"Dans ta maison : %lld pièces et %lld nourritures."
                                                  : u8"Ta bourse : %lld pièces et %lld nourritures.",
                       (long long)v.coins, (long long)v.food);
    const int max_c = (int)std::max<int64_t>(0, std::min<int64_t>(v.coins, kMaxAmount));
    const int max_f = (int)std::max<int64_t>(0, std::min<int64_t>(v.food, kMaxAmount));
    ImGui::SetNextItemWidth(fs * 8.0f);
    ImGui::InputInt(u8"pièces à donner", &u.coins, 1, 10);
    u.coins = std::max(0, std::min(u.coins, max_c));
    ImGui::SetNextItemWidth(fs * 8.0f);
    ImGui::InputInt(u8"nourritures à donner", &u.food, 1, 10);
    u.food = std::max(0, std::min(u.food, max_f));

    // Quels objets.
    if (v.mode == TradeMode::House) {
        if (v.items_ready) {
            // Les objets choisis qui ne sont plus la (rangement relu, objet bouge).
            for (auto it = u.picked.begin(); it != u.picked.end();) {
                bool here = false;
                for (const TradeItem& t : v.items) here = here || t.id == *it;
                it = here ? std::next(it) : u.picked.erase(it);
            }
        }
        ImGui::Separator();
        ImGui::Text(u8"Objets de ton rangement : %zu choisi%s (16 au plus)", u.picked.size(),
                    u.picked.size() > 1 ? "s" : "");
        ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::InputTextWithHint("##mm_trade_filter", u8"Chercher un objet…", u.filter, sizeof(u.filter));
        ImGui::BeginChild("##mm_trade_items", ImVec2(0.0f, -fs * 4.6f), ImGuiChildFlags_Borders);
        if (!v.items_ready) ImGui::TextDisabled(u8"Lecture du rangement…");
        else if (v.items.empty()) ImGui::TextDisabled(u8"Ton rangement est vide.");
        for (const TradeItem& t : v.items) {
            if (!contains_ci(t.label, u.filter)) continue;
            bool on = u.picked.count(t.id) != 0;
            const std::string id = t.label + "##i" + std::to_string(t.id);
            if (ImGui::Checkbox(id.c_str(), &on)) {
                if (!on) u.picked.erase(t.id);
                else if (u.picked.size() < kMaxPicked) u.picked.insert(t.id);
            }
        }
        ImGui::EndChild();
    } else {
        ImGui::Separator();
        ImGui::TextWrapped(u8"En aventure, les objets sont dans le sac de l'équipe, commun à tous : "
                           u8"seules les pièces et la nourriture de ta bourse se donnent.");
    }

    // Donner.
    const bool something = u.coins > 0 || u.food > 0 || !u.picked.empty();
    const bool ok = chosen && chosen->can && something && !v.pending;
    ImGui::BeginDisabled(!ok);
    if (ImGui::Button(u8"Donner", ImVec2(fs * 8.0f, 0.0f)) && chosen) {
        gift_request(v.mode, chosen->id, u.coins, u.food, std::vector<uint64_t>(u.picked.begin(), u.picked.end()));
        u.coins = 0;
        u.food  = 0;
        u.picked.clear();
    }
    ImGui::EndDisabled();
    if (v.pending) {
        ImGui::SameLine();
        ImGui::TextDisabled(u8"En attente de la réponse…");
    }
    status_line();
    ImGui::End();
    if (!keep) set_open(false);
}

void draw_toasts(float fs) {
    std::vector<const TradeToast*> list;
    for (const TradeToast& t : u.v.toasts)
        if (!(u.open && t.result)) list.push_back(&t);    // la fenetre ouverte le dit deja
    if (list.empty()) return;
    const ImGuiViewport* vp = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(vp->WorkPos.x + vp->WorkSize.x * 0.5f, vp->WorkPos.y + vp->WorkSize.y * 0.05f),
                            ImGuiCond_Always, ImVec2(0.5f, 0.0f));
    ImGui::SetNextWindowBgAlpha(0.85f);
    const ImGuiWindowFlags fl = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs |
                                ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing |
                                ImGuiWindowFlags_NoNav | ImGuiWindowFlags_AlwaysAutoResize |
                                ImGuiWindowFlags_NoMove;
    if (ImGui::Begin("##mm_trade_toasts", nullptr, fl)) {
        ImGui::PushTextWrapPos(fs * 30.0f);
        for (const TradeToast* t : list) ImGui::TextUnformatted(t->text.c_str());
        ImGui::PopTextWrapPos();
    }
    ImGui::End();
}

} // namespace

bool trade_ui_wanted() {
    refresh();
    if (!u.have) return false;
    // Le bouton disparait (sortie de la maison, un combat) : la fenetre se ferme avec lui.
    if (u.was_show && !u.v.show) set_open(false);
    u.was_show = u.v.show;
    return u.v.show || u.open || !u.v.toasts.empty();
}

void trade_ui_draw(ImFont* font) {
    const ImGuiIO& io = ImGui::GetIO();
    const float fs = std::max(15.0f, std::min(34.0f, io.DisplaySize.y * (19.0f / 1080.0f)));
    ImGui::PushFont(font, fs);
    if (u.v.show || u.open) draw_button(fs);
    if (u.open) draw_window(fs);
    draw_toasts(fs);
    ImGui::PopFont();
}

} // namespace mgmp
