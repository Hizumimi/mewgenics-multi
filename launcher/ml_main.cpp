// ml_main.cpp -- Mewgenics Multi, le launcher : la fenetre "Mewgenics Multi.exe".
//
// Une fenetre sans bordure de 1152 x 648 points (16:9), dessinee en entier avec
// Direct2D 1.1 sur une swap chain D3D11 :
//   - le decor : un fond sombre et chaud, et le soleil rayonnant du jeu qui
//     tourne lentement derriere. C'est la seule chose animee en continu
//     (~30 images/s ; rien n'est dessine quand la fenetre est reduite) ;
//   - le premier plan : la maison sur la colline, la feuille de papier un peu
//     penchee et ce qu'elle porte (verifications, Jouer, reglages, journaux,
//     lancement). Il ne change qu'a un clic, un survol ou une nouvelle du
//     loader : il est dessine dans une image a part, refaite seulement quand
//     quelque chose change, puis posee telle quelle sur le decor.
//
// L'art vient de resources.gpak chez le joueur (ml_assets, sur un fil a part) :
// la fenetre s'ouvre tout de suite avec ses replis (papier creme dessine, Segoe
// UI par DirectWrite), puis le decor du jeu apparait en fondu. Le texte est
// dessine glyphe par glyphe en geometries Direct2D : les contours des polices du
// jeu, puis ceux des polices du jeu pour l'alphabet de la langue (russe, coreen,
// japonais, chinois : international_fonts.swf, lues seulement pour elle), puis
// ceux de Segoe UI et des polices CJK de Windows pour un caractere qu'elles n'ont
// pas (ou sans l'art). Une seule facon de dessiner le texte, avec ou sans le jeu.
//
// La langue (tr(), src/i18n/mm_lang.h) : "ui"."lang" de mgmp.json, sinon celle du
// jeu, de Windows, le francais (ml::ui_lang) ; le choix « Langue » des Reglages
// l'enregistre et l'applique tout de suite.
//
// Tout est place en points logiques (1152 x 648) ; une echelle (le DPI de
// l'ecran) les passe en pixels. Les zones cliquables sont notees pendant le
// dessin du premier plan ; la souris y est comparee en points du papier pour ce
// qui est dessus (il est penche).
//
//   "Mewgenics Multi.exe" --sans-art   n'essaie pas de lire resources.gpak
//                                        (pour voir le repli).
#include "ml_assets.h"
#include "ml_env.h"
#include "ml_run.h"
#include "ml_text.h"

#include <windows.h>
#include <windowsx.h>
#include <d2d1_1.h>
#include <d2d1_1helper.h>
#include <d2d1effects.h>
#include <d3d11.h>
#include <dwmapi.h>
#include <dwrite.h>
#include <dxgi1_2.h>
#include <shellapi.h>
#include <shobjidl.h>
#include <wrl/client.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstring>
#include <unordered_map>

using Microsoft::WRL::ComPtr;
using M = D2D1::Matrix3x2F;

namespace {

constexpr float kW = 1152, kH = 648;

// Le papier en points de la fenetre ; ce qui s'y pose est en points du papier.
constexpr float kPaperX = 40, kPaperY = 26, kPaperW = 640, kPaperH = 598, kPaperTilt = -1.2f;
constexpr float kLeft = 58, kRight = kPaperW - 58, kMid = kPaperW / 2;
constexpr float kSunX = kPaperX + kPaperW / 2, kSunY = kPaperY + kPaperH / 2, kSunR = 900;
constexpr float kHouseW = 690, kHouseH = kHouseW * 1134 / 1984, kHouseX = 548, kHouseY = kH - kHouseH + 22;

D2D1_COLOR_F rgb(uint32_t c, float a = 1) {
    return {((c >> 16) & 255) / 255.f, ((c >> 8) & 255) / 255.f, (c & 255) / 255.f, a};
}
D2D1_COLOR_F mix(D2D1_COLOR_F a, D2D1_COLOR_F b, float t) {
    return {a.r + (b.r - a.r) * t, a.g + (b.g - a.g) * t, a.b + (b.b - a.b) * t, a.a + (b.a - a.a) * t};
}

// Les traits bruns du jeu sur le papier, et les couleurs des marques.
const uint32_t kInk = 0x2b1d14, kInkSoft = 0x6e5646, kCream = 0xf1e3c4, kLight = 0xf4e4c4;
const uint32_t kGreen = 0x4f9a3b, kRed = 0xc4402c, kOrange = 0xe58b1f, kWarm = 0xd65d37;

enum Face { kTitle, kText, kHand, kFaces };   // Peralta, TikaFont, EdmundMcMillen
enum class Panel { Main, Settings, Logs, Launch };
enum Hot { kNone, kMin, kClose, kPlay, kRepair, kToSettings, kToLogs, kBack, kBrowse, kSteam,
           kSave, kOpenDir, kCopyLog, kRetry, kCancel, kLangPrev, kLangNext };

// Le choix « Langue » des reglages, dans l'ordre : "auto" puis les langues du jeu.
const char* const kLangChoices[] = {"auto", "fr", "en", "sp", "de", "it", "pt-br", "ru", "ko", "ja", "zh-cn"};
constexpr int kLangChoiceCount = sizeof(kLangChoices) / sizeof(kLangChoices[0]);
enum class Mark { Ok, Bad, Warn };

struct Zone { int id; D2D1_RECT_F r; bool paper, enabled; };
struct Shape { ComPtr<ID2D1PathGeometry> geo; float adv = 0; };   // un glyphe, en EM

bool is_file(const std::wstring& p) {
    DWORD a = GetFileAttributesW(p.c_str());
    return a != INVALID_FILE_ATTRIBUTES && !(a & FILE_ATTRIBUTE_DIRECTORY);
}

bool inside(const D2D1_RECT_F& r, D2D1_POINT_2F p) {
    return p.x >= r.left && p.x < r.right && p.y >= r.top && p.y < r.bottom;
}

// -1..1, toujours le meme pour (seed, k) : les bords tremblent sans bouger d'une image a l'autre.
float jitter(unsigned seed, int k) {
    unsigned h = seed * 2654435761u + (unsigned)k * 40503u;
    h ^= h >> 15;
    h *= 0x2c1b3c6du;
    h ^= h >> 12;
    return (h % 1000) / 500.f - 1.f;
}

// Multiplie chaque canal : un BGRA premultiplie le reste.
void tint(ml::Image& im, float r, float g, float b) {
    for (size_t i = 0; i + 3 < im.bgra.size(); i += 4) {
        im.bgra[i]     = (uint8_t)(im.bgra[i] * b);
        im.bgra[i + 1] = (uint8_t)(im.bgra[i + 1] * g);
        im.bgra[i + 2] = (uint8_t)(im.bgra[i + 2] * r);
    }
}

// Le soleil est gris clair sur gris plus clair : on garde l'ecart entre ses rayons
// comme transparence d'un or uni, sinon on ne verrait qu'un disque pale.
void gild(ml::Image& im) {
    for (size_t i = 0; i + 3 < im.bgra.size(); i += 4) {
        uint8_t* p = &im.bgra[i];
        if (!p[3]) continue;
        float v = p[2] / (float)p[3];
        float t = std::clamp((v - 0.86f) / 0.12f, 0.f, 1.f) * p[3] / 255.f;
        p[0] = (uint8_t)(0x6a * t);
        p[1] = (uint8_t)(0xc8 * t);
        p[2] = (uint8_t)(0xff * t);
        p[3] = (uint8_t)(255 * t);
    }
}

class App {
public:
    int run(HINSTANCE inst);

private:
    static LRESULT CALLBACK wndproc(HWND h, UINT m, WPARAM w, LPARAM l);
    LRESULT on_message(UINT m, WPARAM w, LPARAM l);

    // Direct2D
    bool create_device();
    void create_targets();
    void recreate();
    ComPtr<ID2D1Bitmap1> bitmap(const ml::Image& im);
    ComPtr<IDWriteFontFace> system_face(const wchar_t* family, DWRITE_FONT_WEIGHT weight);
    void fit_window(UINT dpi, const RECT* suggested);
    void render();
    void draw_back();
    void draw_front(const M& g);
    void draw_paper();
    void window_buttons();

    // art et texte
    std::filesystem::path gpak_path() const {
        return std::filesystem::path(ml::game_dir(settings_) + L"\\resources.gpak");
    }
    void start_assets();
    void poll_assets();
    void apply_lang();
    void poll_intl();
    void cycle_lang(int step);
    std::string lang_label();
    IDWriteFontFace* system_glyph(char16_t c, bool bold, UINT16* gi);
    const Shape& glyph(int face, char16_t c);
    template <class F> ComPtr<ID2D1PathGeometry> path(F&& add);
    float width(int face, float size, const std::u16string& s);
    float text16(int face, float size, float x, float y, const std::u16string& s, D2D1_COLOR_F col,
                 int align = 0, float halo = 0);
    float text(int face, float size, float x, float y, const std::string& s, D2D1_COLOR_F col,
               int align = 0, float halo = 0) {
        return text16(face, size, x, y, ml::utf8_to_u16(s), col, align, halo);
    }
    std::vector<std::u16string> wrap(int face, float size, float w, const std::string& s);
    std::string elide(int face, float size, float w, const std::string& s);
    float para(int face, float size, float x, float y, float w, const std::string& s, D2D1_COLOR_F col,
               float lh);

    // dessin a la main
    ComPtr<ID2D1PathGeometry> blob(D2D1_RECT_F r, float radius, float wobble, unsigned seed);
    void fill(ID2D1Geometry* g, D2D1_COLOR_F c);
    void stroke(ID2D1Geometry* g, D2D1_COLOR_F c, float w);
    void mark(Mark m, float x, float y, float s);
    void squiggle(float x0, float x1, float y, D2D1_COLOR_F c, float w);
    void spinner(float cx, float cy, float r);
    void zone(int id, D2D1_RECT_F r, bool enabled = true) { zones_.push_back({id, r, on_paper_, enabled}); }
    void button(int id, D2D1_RECT_F r, const std::string& label, bool primary, bool enabled = true);
    void big_button(int id, D2D1_RECT_F r, const std::string& label, bool enabled);
    void link(int id, float x, float y, const std::string& label, int align);
    void heading(const std::string& title);

    // panneaux
    void panel_main();
    void panel_settings();
    void panel_logs();
    void panel_launch();

    // actions
    void tick();
    void refresh_checks();
    bool blocked() const;
    void activate(int id);
    void enter();
    void back();
    void play();
    void browse();
    void save();
    const Zone* hit(D2D1_POINT_2F p) const;
    const Zone* hit_screen(POINT p) const;

    HWND  hwnd_ = nullptr;
    bool  closing_ = false;
    float scale_ = 1;        // pixels par point
    UINT  px_w_ = 0, px_h_ = 0;

    ComPtr<ID2D1Factory1>          d2d_;
    ComPtr<IDWriteFactory>         dw_;
    ComPtr<ID3D11Device>           d3d_;
    ComPtr<IDXGISwapChain1>        swap_;
    ComPtr<ID2D1DeviceContext>     dc_;
    ComPtr<ID2D1Bitmap1>           target_, front_;
    ComPtr<ID2D1SolidColorBrush>   brush_;
    ComPtr<ID2D1RadialGradientBrush> glow_, vignette_;
    ComPtr<ID2D1StrokeStyle>       round_;
    ComPtr<ID2D1Bitmap1>           house_bm_, sun_bm_, paper_bm_;
    ComPtr<IDWriteFontFace>        segoe_, segoe_bold_;
    bool front_dirty_ = true;
    M    base_ = M::Identity();     // transformation de ce qu'on dessine (fenetre ou papier)
    M    paper_m_ = M::Identity();  // papier -> fenetre, en points
    bool on_paper_ = false;

    // L'art du jeu. Les images restent en memoire : une carte graphique perdue
    // (pilote mis a jour) oblige a refaire les bitmaps.
    std::future<ml::Assets> pending_;
    bool no_art_ = false, art_ = false, art_missing_ = false;
    ULONGLONG fade_start_ = 0;
    float fade_ = 0;
    ml::Image house_, sun_, paper_;
    ml::Font fonts_[kFaces];
    std::unordered_map<char16_t, Shape> glyphs_[kFaces];

    // La langue (tr()) et les polices du jeu de son alphabet (international_fonts.swf,
    // lues sur un fil a part quand la langue en a besoin) : texte et titres, a la main.
    mm::Lang     lang_ = mm::Lang::fr;
    ml::LangFrom lang_from_ = ml::LangFrom::Default;
    ml::Font     intl_[2];
    std::string  intl_lang_, intl_want_;   // codes : charge (ou essaye), voulu
    std::future<ml::IntlFonts> intl_pending_;
    std::vector<ComPtr<IDWriteFontFace>> sys_;   // polices CJK du systeme, dans l'ordre de la langue
    bool sys_ready_ = false;

    ml::Paths    paths_;
    ml::Settings settings_, edit_;
    std::vector<ml::Check> checks_;
    ULONGLONG checked_at_ = 0;
    Panel panel_ = Panel::Main;
    std::string repair_note_, settings_note_, logs_note_;
    bool logs_ok_ = false;
    ml::Logs logs_;

    ml::Launch launch_;
    ml::Launch::Snapshot snap_;
    std::vector<std::string> steps_;   // les phrases du lancement, dans l'ordre
    ULONGLONG close_at_ = 0;

    std::vector<Zone> zones_;
    int hover_ = kNone, pressed_ = kNone;
    bool tracking_ = false;
    float play_anim_ = 0, sun_angle_ = 0;
    ULONGLONG last_tick_ = 0;
};

// --- fenetre ----------------------------------------------------------------

int App::run(HINSTANCE inst) {
    no_art_ = wcsstr(GetCommandLineW(), L"--sans-art") != nullptr;
    paths_ = ml::mod_paths();
    ml::config_load(paths_, &settings_);
    apply_lang();     // avant tout texte ; lance aussi la lecture des polices de son alphabet
    start_assets();   // tout de suite : la lecture avance pendant qu'on cree la fenetre

    D2D1_FACTORY_OPTIONS fo = {};
    if (FAILED(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory1), &fo,
                                 reinterpret_cast<void**>(d2d_.GetAddressOf()))) ||
        FAILED(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory),
                                   reinterpret_cast<IUnknown**>(dw_.GetAddressOf())))) {
        MessageBoxW(nullptr,
                    ml::utf8_to_wide(tr(u8"Direct2D est indisponible sur ce PC : Mewgenics Multi ne peut pas "
                                        u8"s'afficher.")).c_str(),
                    L"Mewgenics Multi", MB_ICONERROR);
        return 1;
    }
    d2d_->CreateStrokeStyle(D2D1::StrokeStyleProperties(D2D1_CAP_STYLE_ROUND, D2D1_CAP_STYLE_ROUND,
                                                        D2D1_CAP_STYLE_ROUND, D2D1_LINE_JOIN_ROUND),
                            nullptr, 0, &round_);
    segoe_      = system_face(L"Segoe UI", DWRITE_FONT_WEIGHT_NORMAL);
    segoe_bold_ = system_face(L"Segoe UI", DWRITE_FONT_WEIGHT_BOLD);

    WNDCLASSEXW wc = {sizeof(wc)};
    wc.lpfnWndProc   = wndproc;
    wc.hInstance     = inst;
    wc.hCursor       = LoadCursorW(nullptr, IDC_ARROW);
    wc.hIcon         = LoadIconW(nullptr, IDI_APPLICATION);
    wc.lpszClassName = L"MewgenicsMultiLauncher";
    RegisterClassExW(&wc);

    // Sur l'ecran de la souris ; fit_window le centre a la taille de son DPI.
    POINT cur = {};
    GetCursorPos(&cur);
    MONITORINFO mi = {sizeof(mi)};
    GetMonitorInfoW(MonitorFromPoint(cur, MONITOR_DEFAULTTOPRIMARY), &mi);
    hwnd_ = CreateWindowExW(0, wc.lpszClassName, L"Mewgenics Multi", WS_POPUP | WS_MINIMIZEBOX | WS_SYSMENU,
                            mi.rcWork.left, mi.rcWork.top, (int)kW, (int)kH, nullptr, nullptr, inst, this);
    if (!hwnd_) return 1;
    fit_window(GetDpiForWindow(hwnd_), nullptr);

    DWORD corner = 2;   // DWMWA_WINDOW_CORNER_PREFERENCE = DWMWCP_ROUND : coins de Windows 11
    DwmSetWindowAttribute(hwnd_, 33, &corner, sizeof(corner));
    HICON big = nullptr, little = nullptr;   // l'icone du jeu, s'il est la (small : macro de rpcndr.h)
    if (ExtractIconExW(ml::game_exe(settings_).c_str(), 0, &big, &little, 1) && big) {
        SendMessageW(hwnd_, WM_SETICON, ICON_BIG, (LPARAM)big);
        SendMessageW(hwnd_, WM_SETICON, ICON_SMALL, (LPARAM)(little ? little : big));
    }

    if (!create_device()) {
        MessageBoxW(hwnd_,
                    ml::utf8_to_wide(tr(u8"La carte graphique n'a pas pu être préparée : Mewgenics Multi ne "
                                        u8"peut pas s'afficher.")).c_str(),
                    L"Mewgenics Multi", MB_ICONERROR);
        return 1;
    }
    refresh_checks();
    // Un court instant pour l'art : s'il arrive, la fenetre s'ouvre deja aux
    // couleurs du jeu (sinon elle s'ouvre avec le repli, et l'art suit en fondu).
    if (pending_.valid()) pending_.wait_for(std::chrono::milliseconds(300));
    poll_assets();
    last_tick_ = GetTickCount64();
    render();
    ShowWindow(hwnd_, SW_SHOW);
    SetTimer(hwnd_, 1, 33, nullptr);

    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    if (big) DestroyIcon(big);
    if (little) DestroyIcon(little);
    return 0;
}

LRESULT CALLBACK App::wndproc(HWND h, UINT m, WPARAM w, LPARAM l) {
    if (m == WM_NCCREATE) {
        auto* app = (App*)((CREATESTRUCTW*)l)->lpCreateParams;
        app->hwnd_ = h;
        SetWindowLongPtrW(h, GWLP_USERDATA, (LONG_PTR)app);
    }
    auto* app = (App*)GetWindowLongPtrW(h, GWLP_USERDATA);
    return app ? app->on_message(m, w, l) : DefWindowProcW(h, m, w, l);
}

LRESULT App::on_message(UINT m, WPARAM w, LPARAM l) {
    switch (m) {
    case WM_TIMER:
        tick();
        return 0;
    case WM_PAINT: {
        PAINTSTRUCT ps;
        BeginPaint(hwnd_, &ps);
        EndPaint(hwnd_, &ps);
        render();
        return 0;
    }
    case WM_ERASEBKGND:
        return 1;
    case WM_NCHITTEST: {
        // Tout ce qui ne se clique pas sert a deplacer la fenetre.
        POINT p = {GET_X_LPARAM(l), GET_Y_LPARAM(l)};
        return hit_screen(p) ? HTCLIENT : HTCAPTION;
    }
    case WM_SETCURSOR:
        if (LOWORD(l) == HTCLIENT) {
            POINT p;
            GetCursorPos(&p);
            const Zone* z = hit_screen(p);
            SetCursor(LoadCursorW(nullptr, z && z->enabled ? IDC_HAND : IDC_ARROW));
            return TRUE;
        }
        break;
    case WM_MOUSEMOVE: {
        if (!tracking_) {
            TRACKMOUSEEVENT t = {sizeof(t), TME_LEAVE, hwnd_, 0};
            tracking_ = TrackMouseEvent(&t) != FALSE;
        }
        const Zone* z = hit({GET_X_LPARAM(l) / scale_, GET_Y_LPARAM(l) / scale_});
        int id = z && z->enabled ? z->id : kNone;
        if (id != hover_) { hover_ = id; front_dirty_ = true; }
        return 0;
    }
    case WM_MOUSELEAVE:
    case WM_NCMOUSEMOVE:
        if (m == WM_MOUSELEAVE) tracking_ = false;
        if (hover_ != kNone) { hover_ = kNone; front_dirty_ = true; }
        break;
    case WM_LBUTTONDOWN: {
        const Zone* z = hit({GET_X_LPARAM(l) / scale_, GET_Y_LPARAM(l) / scale_});
        if (z && z->enabled) {
            pressed_ = z->id;
            SetCapture(hwnd_);
            front_dirty_ = true;
            render();
        }
        return 0;
    }
    case WM_LBUTTONUP: {
        int was = pressed_;
        pressed_ = kNone;
        if (GetCapture() == hwnd_) ReleaseCapture();
        const Zone* z = hit({GET_X_LPARAM(l) / scale_, GET_Y_LPARAM(l) / scale_});
        front_dirty_ = true;
        if (was != kNone && z && z->id == was && z->enabled) activate(was);
        else render();
        return 0;
    }
    case WM_KEYDOWN:
        if (w == VK_RETURN) enter();
        else if (w == VK_ESCAPE) back();
        return 0;
    case WM_ACTIVATE:
        if (LOWORD(w) != WA_INACTIVE && panel_ == Panel::Main && dc_) refresh_checks();
        break;
    case WM_SIZE:
        if (w != SIZE_MINIMIZED && swap_ && LOWORD(l) && HIWORD(l) &&
            (LOWORD(l) != px_w_ || HIWORD(l) != px_h_)) {
            px_w_ = LOWORD(l);
            px_h_ = HIWORD(l);
            dc_->SetTarget(nullptr);
            target_.Reset();
            front_.Reset();
            if (FAILED(swap_->ResizeBuffers(0, px_w_, px_h_, DXGI_FORMAT_UNKNOWN, 0))) recreate();
            else create_targets();
        }
        // Reduite : plus de dessin, juste de quoi suivre le lancement.
        SetTimer(hwnd_, 1, w == SIZE_MINIMIZED ? 250 : 33, nullptr);
        return 0;
    case WM_DPICHANGED:
        fit_window(HIWORD(w), (const RECT*)l);
        front_dirty_ = true;
        return 0;
    case WM_DESTROY:
        closing_ = true;
        KillTimer(hwnd_, 1);
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hwnd_, m, w, l);
}

void App::fit_window(UINT dpi, const RECT* suggested) {
    HMONITOR mon = suggested ? MonitorFromRect(suggested, MONITOR_DEFAULTTONEAREST)
                             : MonitorFromWindow(hwnd_, MONITOR_DEFAULTTONEAREST);
    MONITORINFO mi = {sizeof(mi)};
    GetMonitorInfoW(mon, &mi);
    const RECT& wa = mi.rcWork;
    // Un petit ecran tres agrandi : la fenetre reste entiere a l'ecran.
    scale_ = std::min({dpi / 96.f, (wa.right - wa.left) * 0.96f / kW, (wa.bottom - wa.top) * 0.96f / kH});
    int w = (int)lroundf(kW * scale_), h = (int)lroundf(kH * scale_);
    int x = suggested ? suggested->left : (wa.left + wa.right - w) / 2;
    int y = suggested ? suggested->top : (wa.top + wa.bottom - h) / 2;
    SetWindowPos(hwnd_, nullptr, x, y, w, h, SWP_NOZORDER | SWP_NOACTIVATE);
}

// --- Direct2D ---------------------------------------------------------------

bool App::create_device() {
    const UINT flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
    HRESULT hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags, nullptr, 0,
                                   D3D11_SDK_VERSION, &d3d_, nullptr, nullptr);
    if (FAILED(hr))   // pas de carte utilisable (bureau a distance...) : le rendu logiciel
        hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, flags, nullptr, 0, D3D11_SDK_VERSION,
                               &d3d_, nullptr, nullptr);
    if (FAILED(hr)) return false;

    ComPtr<IDXGIDevice> dxgi;
    ComPtr<ID2D1Device> dev;
    ComPtr<IDXGIAdapter> adapter;
    ComPtr<IDXGIFactory2> factory;
    if (FAILED(d3d_.As(&dxgi)) || FAILED(d2d_->CreateDevice(dxgi.Get(), &dev)) ||
        FAILED(dev->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &dc_)) ||
        FAILED(dxgi->GetAdapter(&adapter)) || FAILED(adapter->GetParent(IID_PPV_ARGS(&factory))))
        return false;

    RECT rc;
    GetClientRect(hwnd_, &rc);
    px_w_ = (UINT)std::max<LONG>(1, rc.right);
    px_h_ = (UINT)std::max<LONG>(1, rc.bottom);
    DXGI_SWAP_CHAIN_DESC1 sd = {};
    sd.Width            = px_w_;
    sd.Height           = px_h_;
    sd.Format           = DXGI_FORMAT_B8G8R8A8_UNORM;
    sd.SampleDesc.Count = 1;
    sd.BufferUsage      = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.BufferCount      = 2;
    sd.SwapEffect       = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
    sd.AlphaMode        = DXGI_ALPHA_MODE_IGNORE;
    if (FAILED(factory->CreateSwapChainForHwnd(d3d_.Get(), hwnd_, &sd, nullptr, nullptr, &swap_)))
        return false;
    factory->MakeWindowAssociation(hwnd_, DXGI_MWA_NO_ALT_ENTER);

    dc_->CreateSolidColorBrush(rgb(kInk), &brush_);
    // Le fond : une lueur chaude derriere le papier, et les bords assombris.
    ComPtr<ID2D1GradientStopCollection> stops;
    D2D1_GRADIENT_STOP glow[] = {{0.f, rgb(0x5a3d29)}, {0.55f, rgb(0x2e2018)}, {1.f, rgb(0x150e0a)}};
    dc_->CreateGradientStopCollection(glow, 3, &stops);
    dc_->CreateRadialGradientBrush(D2D1::RadialGradientBrushProperties({kSunX, kSunY}, {}, 820, 820), stops.Get(),
                                   &glow_);
    D2D1_GRADIENT_STOP vig[] = {{0.f, rgb(0x150e0a, 0)}, {0.62f, rgb(0x150e0a, 0)}, {1.f, rgb(0x150e0a, 0.8f)}};
    dc_->CreateGradientStopCollection(vig, 3, &stops);
    dc_->CreateRadialGradientBrush(D2D1::RadialGradientBrushProperties({kW / 2, kH / 2}, {}, 740, 520),
                                   stops.Get(), &vignette_);
    if (!brush_ || !glow_ || !vignette_) return false;

    create_targets();
    house_bm_ = bitmap(house_);
    sun_bm_   = bitmap(sun_);
    paper_bm_ = bitmap(paper_);
    return target_ && front_;
}

void App::create_targets() {
    ComPtr<IDXGISurface> surface;
    if (FAILED(swap_->GetBuffer(0, IID_PPV_ARGS(&surface)))) return;
    D2D1_BITMAP_PROPERTIES1 tp = D2D1::BitmapProperties1(
        D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
        D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE));
    dc_->CreateBitmapFromDxgiSurface(surface.Get(), &tp, &target_);
    D2D1_BITMAP_PROPERTIES1 fp = D2D1::BitmapProperties1(
        D2D1_BITMAP_OPTIONS_TARGET, D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED));
    dc_->CreateBitmap(D2D1::SizeU(px_w_, px_h_), nullptr, 0, &fp, &front_);
    front_dirty_ = true;
}

ComPtr<ID2D1Bitmap1> App::bitmap(const ml::Image& im) {
    ComPtr<ID2D1Bitmap1> b;
    if (im.empty() || !dc_) return b;
    D2D1_BITMAP_PROPERTIES1 bp = D2D1::BitmapProperties1(
        D2D1_BITMAP_OPTIONS_NONE, D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED));
    dc_->CreateBitmap(D2D1::SizeU(im.w, im.h), im.bgra.data(), im.w * 4, &bp, &b);
    return b;
}

ComPtr<IDWriteFontFace> App::system_face(const wchar_t* family, DWRITE_FONT_WEIGHT weight) {
    ComPtr<IDWriteFontCollection> fonts;
    ComPtr<IDWriteFontFamily> fam;
    ComPtr<IDWriteFont> font;
    ComPtr<IDWriteFontFace> face;
    UINT32 index = 0;
    BOOL found = FALSE;
    if (SUCCEEDED(dw_->GetSystemFontCollection(&fonts)) && SUCCEEDED(fonts->FindFamilyName(family, &index, &found)) &&
        found && SUCCEEDED(fonts->GetFontFamily(index, &fam)) &&
        SUCCEEDED(fam->GetFirstMatchingFont(weight, DWRITE_FONT_STRETCH_NORMAL, DWRITE_FONT_STYLE_NORMAL, &font)))
        font->CreateFontFace(&face);
    return face;
}

// Carte graphique perdue (ou une reprise d'avant qui a echoue) : on refait tout
// ce qui en depend, a partir des images gardees en memoire. Les geometries
// (glyphes) ne dependent pas d'elle.
void App::recreate() {
    if (dc_) dc_->SetTarget(nullptr);
    dc_.Reset();
    target_.Reset();
    front_.Reset();
    swap_.Reset();
    d3d_.Reset();
    brush_.Reset();
    glow_.Reset();
    vignette_.Reset();
    house_bm_.Reset();
    sun_bm_.Reset();
    paper_bm_.Reset();
    create_device();
}

void App::render() {
    if (closing_ || IsIconic(hwnd_)) return;
    if (!target_ || !front_) recreate();   // une reprise ratee : on retente a chaque image
    if (!target_ || !front_) return;
    const M g = M::Scale(scale_, scale_);
    if (front_dirty_) {
        front_dirty_ = false;
        dc_->SetTarget(front_.Get());
        dc_->BeginDraw();
        dc_->Clear(D2D1::ColorF(0, 0));
        draw_front(g);
        dc_->EndDraw();
    }
    dc_->SetTarget(target_.Get());
    dc_->BeginDraw();
    dc_->SetTransform(g);
    draw_back();
    dc_->SetTransform(M::Identity());
    dc_->DrawBitmap(front_.Get());   // pixel pour pixel
    HRESULT hr = dc_->EndDraw();
    if (SUCCEEDED(hr)) hr = swap_->Present(1, 0);
    if (hr == D2DERR_RECREATE_TARGET || hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
        recreate();
}

void App::draw_back() {
    const D2D1_RECT_F all = {0, 0, kW, kH};
    dc_->FillRectangle(all, glow_.Get());
    if (sun_bm_ && fade_ > 0) {
        D2D1_RECT_F r = {kSunX - kSunR, kSunY - kSunR, kSunX + kSunR, kSunY + kSunR};
        dc_->SetTransform(M::Rotation(sun_angle_, {kSunX, kSunY}) * M::Scale(scale_, scale_));
        dc_->DrawBitmap(sun_bm_.Get(), &r, 0.2f * fade_, D2D1_INTERPOLATION_MODE_LINEAR, nullptr, nullptr);
        dc_->SetTransform(M::Scale(scale_, scale_));
    }
    dc_->FillRectangle(all, vignette_.Get());
}

void App::draw_front(const M& g) {
    zones_.clear();
    base_ = g;
    on_paper_ = false;
    if (house_bm_) {
        D2D1_RECT_F r = {kHouseX, kHouseY, kHouseX + kHouseW, kHouseY + kHouseH};
        dc_->SetTransform(g);
        dc_->DrawBitmap(house_bm_.Get(), &r, fade_, D2D1_INTERPOLATION_MODE_HIGH_QUALITY_CUBIC, nullptr, nullptr);
    }
    // Ce que le mod change, ecrit a la main au-dessus de la maison.
    base_ = M::Rotation(4, {915, 160}) * g;
    {   // deux lignes, une seule phrase a traduire
        const std::string motto = tr(u8"Chacun sa maison et ses chats,\nl'aventure ensemble !");
        const size_t nl = motto.find('\n');
        text(kHand, 29, 915, 150, motto.substr(0, nl), rgb(kLight, 0.9f), 1);
        if (nl != std::string::npos) text(kHand, 29, 915, 190, motto.substr(nl + 1), rgb(kLight, 0.9f), 1);
    }

    paper_m_ = M::Rotation(kPaperTilt, {kPaperW / 2, kPaperH / 2}) * M::Translation(kPaperX, kPaperY);
    base_ = paper_m_ * g;
    on_paper_ = true;
    draw_paper();
    switch (panel_) {
    case Panel::Main:     panel_main(); break;
    case Panel::Settings: panel_settings(); break;
    case Panel::Logs:     panel_logs(); break;
    case Panel::Launch:   panel_launch(); break;
    }

    base_ = g;
    on_paper_ = false;
    window_buttons();
    if (art_missing_)
        text(kText, 14, kW - 22, kH - 16, tr(u8"(illustrations du jeu introuvables)"), rgb(kLight, 0.55f), 2);
}

void App::draw_paper() {
    const D2D1_RECT_F r = {0, 0, kPaperW, kPaperH};
    const float a = paper_bm_ ? fade_ : 0;   // part du papier du jeu (fondu)
    const M keep = base_;
    if (a < 1) {
        // Le repli : une feuille creme aux bords qui tremblent, et son ombre.
        auto g = blob(r, 8, 3.5f, 7);
        base_ = M::Translation(6, 9) * keep;
        fill(g.Get(), rgb(0, 0.3f * (1 - a)));
        base_ = keep;
        fill(g.Get(), rgb(kCream, 1 - a));
        stroke(g.Get(), rgb(kInk, 1 - a), 3);
    }
    if (a > 0) {
        ComPtr<ID2D1Effect> shadow;
        if (SUCCEEDED(dc_->CreateEffect(CLSID_D2D1Shadow, &shadow))) {
            shadow->SetInput(0, paper_bm_.Get());
            shadow->SetValue(D2D1_SHADOW_PROP_BLUR_STANDARD_DEVIATION, 9.f);
            shadow->SetValue(D2D1_SHADOW_PROP_COLOR, D2D1::Vector4F(0, 0, 0, 0.6f * a));
            dc_->SetTransform(M::Scale(kPaperW / paper_.w, kPaperH / paper_.h) * M::Translation(7, 11) * keep);
            dc_->DrawImage(shadow.Get());
        }
        dc_->SetTransform(keep);
        dc_->DrawBitmap(paper_bm_.Get(), &r, a, D2D1_INTERPOLATION_MODE_HIGH_QUALITY_CUBIC, nullptr, nullptr);
    }
}

void App::window_buttons() {
    const float cy = 28, r = 15;
    for (int id : {kMin, kClose}) {
        float cx = id == kMin ? kW - 66 : kW - 28;
        zone(id, {cx - r - 4, cy - r - 4, cx + r + 4, cy + r + 4});
        bool hov = hover_ == id;
        ComPtr<ID2D1EllipseGeometry> e;
        d2d_->CreateEllipseGeometry(D2D1::Ellipse({cx, cy}, r, r), &e);
        fill(e.Get(), hov ? (id == kClose ? rgb(kRed) : rgb(kLight, 0.25f)) : rgb(0, 0.3f));
        stroke(e.Get(), rgb(kLight, hov ? 0.9f : 0.45f), 1.5f);
        auto g = path([&](ID2D1GeometrySink* k) {
            auto seg = [&](float x0, float y0, float x1, float y1) {
                k->BeginFigure({cx + x0, cy + y0}, D2D1_FIGURE_BEGIN_HOLLOW);
                k->AddLine({cx + x1, cy + y1});
                k->EndFigure(D2D1_FIGURE_END_OPEN);
            };
            if (id == kMin) seg(-6, 1, 6, 1);
            else { seg(-5.5f, -5.5f, 5.5f, 5.5f); seg(5.5f, -5.5f, -5.5f, 5.5f); }
        });
        stroke(g.Get(), rgb(kLight), 2.4f);
    }
}

// --- art et texte -----------------------------------------------------------

void App::start_assets() {
    if (no_art_) {
        art_missing_ = true;
        return;
    }
    pending_ = ml::load_assets_async(gpak_path());
}

// La langue des reglages (ou celle du jeu, de Windows) : tr() la suit des le dessin suivant.
void App::apply_lang() {
    lang_ = ml::ui_lang(settings_.lang, &lang_from_);
    mm::lang_set(lang_);
    const std::string code = mm::lang_code(lang_);
    if (ml::intl_fonts_needed(code)) intl_want_ = code;
    sys_.clear();
    sys_ready_ = false;
    for (auto& g : glyphs_) g.clear();
    front_dirty_ = true;
    poll_intl();
}

void App::poll_intl() {
    if (intl_pending_.valid()) {
        if (intl_pending_.wait_for(std::chrono::seconds(0)) != std::future_status::ready) return;
        ml::IntlFonts f;
        try {
            f = intl_pending_.get();
        } catch (...) {   // std::bad_alloc : on garde les replis (polices du systeme)
            f.lang = intl_want_;
        }
        intl_[0] = std::move(f.text);
        intl_[1] = std::move(f.hand);
        intl_lang_ = f.lang;
        for (auto& g : glyphs_) g.clear();
        front_dirty_ = true;
    }
    // Une seule lecture a la fois (le destructeur d'un std::async attend la fin) :
    // la langue choisie entre-temps passe ensuite.
    if (!no_art_ && !intl_want_.empty() && intl_want_ != intl_lang_)
        intl_pending_ = ml::load_intl_fonts_async(gpak_path(), intl_want_);
}

// Le choix « Langue » : enregistre tout de suite dans mgmp.json et applique.
void App::cycle_lang(int step) {
    int i = 0;
    for (int k = 0; k < kLangChoiceCount; ++k)
        if (settings_.lang == kLangChoices[k]) i = k;
    ml::Settings s = settings_;
    s.lang = kLangChoices[(i + step + kLangChoiceCount) % kLangChoiceCount];
    std::string why;
    settings_note_ = ml::config_save(paths_, s, &why) ? std::string() : why;
    settings_.lang = edit_.lang = s.lang;
    repair_note_.clear();   // ecrite dans l'ancienne langue ; la ligne reprend la phrase de check_all
    apply_lang();
    refresh_checks();
}

std::string App::lang_label() {
    mm::Lang l;
    if (settings_.lang != "auto" && mm::lang_from_code(settings_.lang, &l)) return mm::lang_name(l);
    const char* f = lang_from_ == ml::LangFrom::Game      ? tr(u8"Comme le jeu (%s)")
                    : lang_from_ == ml::LangFrom::Windows ? tr(u8"Comme Windows (%s)")
                                                          : tr(u8"Automatique (%s)");
    char buf[256];
    snprintf(buf, sizeof(buf), f, mm::lang_name(lang_));
    return buf;
}

// Un caractere qu'aucune police du jeu n'a : Segoe UI, puis les polices coreenne,
// japonaise et chinoise de Windows (dans l'ordre de la langue : les memes
// ideogrammes n'ont pas la meme forme en japonais et en chinois). Sans aucune :
// le glyphe « absent » de Segoe UI.
IDWriteFontFace* App::system_glyph(char16_t c, bool bold, UINT16* gi) {
    IDWriteFontFace* segoe = bold ? segoe_bold_.Get() : segoe_.Get();
    UINT32 cp = c;
    *gi = 0;
    if (segoe && SUCCEEDED(segoe->GetGlyphIndices(&cp, 1, gi)) && *gi) return segoe;
    if (!sys_ready_) {
        sys_ready_ = true;
        const wchar_t* ko = L"Malgun Gothic";
        const wchar_t* ja = L"Yu Gothic UI";
        const wchar_t* zh = L"Microsoft YaHei UI";
        const wchar_t* order[3] = {ko, ja, zh};
        if (lang_ == mm::Lang::ja) order[0] = ja, order[1] = zh, order[2] = ko;
        if (lang_ == mm::Lang::zh_cn) order[0] = zh, order[1] = ja, order[2] = ko;
        for (const wchar_t* fam : order)
            if (auto f = system_face(fam, DWRITE_FONT_WEIGHT_NORMAL)) sys_.push_back(f);
    }
    for (auto& f : sys_)
        if (SUCCEEDED(f->GetGlyphIndices(&cp, 1, gi)) && *gi) return f.Get();
    *gi = 0;
    return segoe;
}

void App::poll_assets() {
    if (!pending_.valid() || pending_.wait_for(std::chrono::seconds(0)) != std::future_status::ready) return;
    ml::Assets a;
    try {
        a = pending_.get();
    } catch (...) {   // seul std::bad_alloc peut sortir : on garde le repli
        a.problems.push_back("memoire");
    }
    house_ = std::move(a.house);
    sun_   = std::move(a.sun);
    paper_ = std::move(a.paper);
    tint(house_, 1.f, .95f, .88f);   // un peu plus chaude, comme le fond
    gild(sun_);
    tint(paper_, 1.f, .95f, .84f);   // le papier gris du jeu, en creme
    fonts_[kTitle] = std::move(a.title);
    fonts_[kText]  = std::move(a.text);
    fonts_[kHand]  = std::move(a.hand);
    for (auto& g : glyphs_) g.clear();
    house_bm_ = bitmap(house_);
    sun_bm_   = bitmap(sun_);
    paper_bm_ = bitmap(paper_);
    art_missing_ = !a.complete();
    art_ = true;
    fade_ = 0;
    fade_start_ = GetTickCount64();
    front_dirty_ = true;
}

template <class F> ComPtr<ID2D1PathGeometry> App::path(F&& add) {
    ComPtr<ID2D1PathGeometry> g;
    ComPtr<ID2D1GeometrySink> k;
    if (FAILED(d2d_->CreatePathGeometry(&g)) || FAILED(g->Open(&k))) return nullptr;
    add(k.Get());
    k->Close();
    return g;
}

const Shape& App::glyph(int face, char16_t c) {
    auto& cache = glyphs_[face];
    auto it = cache.find(c);
    if (it != cache.end()) return it->second;
    Shape s;
    // La police du jeu, puis celle de l'alphabet de la langue (international_fonts.swf).
    const ml::Font& intl = face == kHand && !intl_[1].glyphs.empty() ? intl_[1] : intl_[0];
    const ml::Glyph* found = fonts_[face].find(c);
    if (!found) found = intl.find(c);
    if (c == u'\u200B') {
        // Coupure permise (japonais, chinois) : ni largeur ni dessin, comme dans le jeu.
    } else if (const ml::Glyph* g = found) {
        s.adv = g->advance;
        if (!g->path.empty()) {
            s.geo = path([&](ID2D1GeometrySink* k) {
                k->SetFillMode(D2D1_FILL_MODE_ALTERNATE);   // la regle convenue (ml_swf.h)
                bool open = false;
                for (const ml::PathCmd& p : g->path) {
                    if (p.op == ml::PathCmd::Move) {
                        if (open) k->EndFigure(D2D1_FIGURE_END_CLOSED);
                        k->BeginFigure({p.x, p.y}, D2D1_FIGURE_BEGIN_FILLED);
                        open = true;
                    } else if (open && p.op == ml::PathCmd::Line) {
                        k->AddLine({p.x, p.y});
                    } else if (open) {
                        k->AddQuadraticBezier(D2D1::QuadraticBezierSegment({p.cx, p.cy}, {p.x, p.y}));
                    }
                }
                if (open) k->EndFigure(D2D1_FIGURE_END_CLOSED);
            });
        }
    } else if (UINT16 gi = 0; IDWriteFontFace* f = system_glyph(c, face != kText, &gi)) {
        // Absent des polices du jeu (ou pas d'art) : le contour d'une police du systeme, en EM aussi.
        DWRITE_FONT_METRICS fm;
        DWRITE_GLYPH_METRICS gm;
        f->GetMetrics(&fm);
        if (SUCCEEDED(f->GetDesignGlyphMetrics(&gi, 1, &gm, FALSE))) {
            s.adv = gm.advanceWidth / (float)fm.designUnitsPerEm;
            s.geo = path([&](ID2D1GeometrySink* k) { f->GetGlyphRunOutline(1.f, &gi, nullptr, nullptr, 1, FALSE, FALSE, k); });
        }
    }
    return cache.emplace(c, std::move(s)).first->second;
}

float App::width(int face, float size, const std::u16string& s) {
    float w = 0;
    for (size_t i = 0; i < s.size(); ++i) {
        w += glyph(face, s[i]).adv;
        if (i + 1 < s.size()) w += fonts_[face].kern(s[i], s[i + 1]);
    }
    return w * size;
}

// (x, y) : debut de la ligne de base ; align 0 gauche, 1 centre, 2 droite. halo :
// un contour brun sous les lettres (en pixels), dessine avant toutes les lettres
// pour qu'il ne mange pas la voisine.
float App::text16(int face, float size, float x, float y, const std::u16string& s, D2D1_COLOR_F col, int align,
                  float halo) {
    float w = width(face, size, s);
    if (align == 1) x -= w / 2;
    else if (align == 2) x -= w;
    for (int pass = halo > 0 ? 0 : 1; pass < 2; ++pass) {
        brush_->SetColor(pass ? col : rgb(kInk, col.a));
        float px = x;
        for (size_t i = 0; i < s.size(); ++i) {
            const Shape& g = glyph(face, s[i]);
            if (g.geo) {
                dc_->SetTransform(M::Scale(size, size) * M::Translation(px, y) * base_);
                if (pass) dc_->FillGeometry(g.geo.Get(), brush_.Get());
                else dc_->DrawGeometry(g.geo.Get(), brush_.Get(), halo * 2 / size, round_.Get());
            }
            px += g.adv * size;
            if (i + 1 < s.size()) px += fonts_[face].kern(s[i], s[i + 1]) * size;
        }
    }
    return w;
}

std::vector<std::u16string> App::wrap(int face, float size, float w, const std::string& s) {
    return ml::wrap_lines(ml::utf8_to_u16(s), w, [&](char16_t c) { return glyph(face, c).adv * size; });
}

// Un chemin trop long pour une ligne : le debut et la fin, des points de suspension au milieu.
std::string App::elide(int face, float size, float w, const std::string& s) {
    const std::u16string t = ml::utf8_to_u16(s);
    if (width(face, size, t) <= w) return s;
    std::u16string shown;
    for (size_t keep = t.size(); keep-- > 0;) {
        const size_t head = (keep + 1) / 2;
        shown = t.substr(0, head) + u'…' + t.substr(t.size() - (keep - head));
        if (width(face, size, shown) <= w) break;
    }
    return ml::u16_to_utf8(shown);
}

// Rend la ligne de base de la ligne suivante.
float App::para(int face, float size, float x, float y, float w, const std::string& s, D2D1_COLOR_F col,
                float lh) {
    for (const auto& line : wrap(face, size, w, s)) {
        text16(face, size, x, y, line, col);
        y += lh;
    }
    return y;
}

// --- dessin a la main -------------------------------------------------------

ComPtr<ID2D1PathGeometry> App::blob(D2D1_RECT_F r, float rad, float wob, unsigned seed) {
    return path([&](ID2D1GeometrySink* k) {
        auto J = [&](int i) { return jitter(seed, i) * wob; };
        auto q = [&](float cx, float cy, float x, float y) {
            k->AddQuadraticBezier(D2D1::QuadraticBezierSegment({cx, cy}, {x, y}));
        };
        const float l = r.left, t = r.top, R = r.right, b = r.bottom, mx = (l + R) / 2, my = (t + b) / 2;
        k->BeginFigure({l + rad, t}, D2D1_FIGURE_BEGIN_FILLED);
        q(mx, t + J(1), R - rad, t + J(2) * .3f);
        q(R, t, R, t + rad);
        q(R + J(4), my, R + J(5) * .3f, b - rad);
        q(R, b, R - rad, b);
        q(mx, b + J(7), l + rad, b + J(8) * .3f);
        q(l, b, l, b - rad);
        q(l + J(10), my, l + J(11) * .3f, t + rad);
        q(l, t, l + rad, t);
        k->EndFigure(D2D1_FIGURE_END_CLOSED);
    });
}

void App::fill(ID2D1Geometry* g, D2D1_COLOR_F c) {
    if (!g) return;
    dc_->SetTransform(base_);
    brush_->SetColor(c);
    dc_->FillGeometry(g, brush_.Get());
}

void App::stroke(ID2D1Geometry* g, D2D1_COLOR_F c, float w) {
    if (!g) return;
    dc_->SetTransform(base_);
    brush_->SetColor(c);
    dc_->DrawGeometry(g, brush_.Get(), w, round_.Get());
}

// Coche, croix ou point d'exclamation, en traits epais de crayon cernes de brun.
// (x, y) : coin haut gauche d'un carre de cote s.
void App::mark(Mark m, float x, float y, float s) {
    auto g = path([&](ID2D1GeometrySink* k) {
        auto P = [&](float u, float v) { return D2D1::Point2F(x + u * s, y + v * s); };
        auto stroke1 = [&](D2D1_POINT_2F a, D2D1_POINT_2F c, D2D1_POINT_2F b) {
            k->BeginFigure(a, D2D1_FIGURE_BEGIN_HOLLOW);
            k->AddQuadraticBezier(D2D1::QuadraticBezierSegment(c, b));
            k->EndFigure(D2D1_FIGURE_END_OPEN);
        };
        if (m == Mark::Ok) {
            k->BeginFigure(P(.12f, .55f), D2D1_FIGURE_BEGIN_HOLLOW);
            k->AddQuadraticBezier(D2D1::QuadraticBezierSegment(P(.26f, .62f), P(.38f, .86f)));
            k->AddQuadraticBezier(D2D1::QuadraticBezierSegment(P(.55f, .40f), P(.94f, .10f)));
            k->EndFigure(D2D1_FIGURE_END_OPEN);
        } else if (m == Mark::Bad) {
            stroke1(P(.16f, .14f), P(.52f, .46f), P(.86f, .88f));
            stroke1(P(.84f, .12f), P(.46f, .52f), P(.14f, .86f));
        } else {
            stroke1(P(.50f, .06f), P(.55f, .36f), P(.50f, .60f));
            stroke1(P(.50f, .86f), P(.505f, .865f), P(.51f, .87f));   // le point
        }
    });
    stroke(g.Get(), rgb(kInk), s * 0.27f);
    stroke(g.Get(), rgb(m == Mark::Ok ? kGreen : m == Mark::Bad ? kRed : kOrange), s * 0.15f);
}

void App::squiggle(float x0, float x1, float y, D2D1_COLOR_F c, float w) {
    auto g = path([&](ID2D1GeometrySink* k) {
        k->BeginFigure({x0, y}, D2D1_FIGURE_BEGIN_HOLLOW);
        int n = std::max(1, (int)((x1 - x0) / 30));
        float step = (x1 - x0) / n;
        for (int i = 0; i < n; ++i)
            k->AddQuadraticBezier(D2D1::QuadraticBezierSegment({x0 + step * (i + .5f), y + (i % 2 ? 1.8f : -1.8f)},
                                                               {x0 + step * (i + 1), y}));
        k->EndFigure(D2D1_FIGURE_END_OPEN);
    });
    stroke(g.Get(), c, w);
}

void App::spinner(float cx, float cy, float r) {
    float a = (GetTickCount64() % 100000) / 1000.f * 5.f;
    auto g = path([&](ID2D1GeometrySink* k) {
        k->BeginFigure({cx + r * cosf(a), cy + r * sinf(a)}, D2D1_FIGURE_BEGIN_HOLLOW);
        k->AddArc(D2D1::ArcSegment({cx + r * cosf(a + 4.4f), cy + r * sinf(a + 4.4f)}, {r, r}, 0,
                                   D2D1_SWEEP_DIRECTION_CLOCKWISE, D2D1_ARC_SIZE_LARGE));
        k->EndFigure(D2D1_FIGURE_END_OPEN);
    });
    stroke(g.Get(), rgb(kInk), 5);
    stroke(g.Get(), rgb(kOrange), 2.6f);
}

// Un bouton de papier (ou orange, primary) aux bords qui tremblent.
void App::button(int id, D2D1_RECT_F r, const std::string& label, bool primary, bool enabled) {
    zone(id, r, enabled);
    const bool hov = enabled && hover_ == id, down = hov && pressed_ == id;
    const M keep = base_;
    auto g = blob(r, 10, 1.6f, id * 17 + 3);
    if (!down) {
        base_ = M::Translation(0, 3) * keep;
        fill(g.Get(), rgb(kInk, enabled ? 0.3f : 0.12f));
    }
    base_ = down ? M::Translation(0, 2) * keep : keep;
    D2D1_COLOR_F f = !enabled ? rgb(0xdccfb6) : primary ? rgb(hov ? 0xe36c45 : kWarm) : rgb(hov ? 0xfff8e8 : 0xf6e9cd);
    fill(g.Get(), f);
    stroke(g.Get(), rgb(kInk, enabled ? 1 : 0.45f), 2.5f);
    const float size = primary ? 22.f : 18.f;
    text(primary ? kTitle : kText, size, (r.left + r.right) / 2, (r.top + r.bottom) / 2 + size * 0.36f, label,
         primary ? rgb(0xfff4dc) : rgb(kInk, enabled ? 1 : 0.45f), 1, primary ? 2.f : 0.f);
    base_ = keep;
}

// Le grand bouton du papier : une pastille orange epaisse. Survol : elle grossit
// et penche un peu ; clic : elle s'enfonce sur son epaisseur.
void App::big_button(int id, D2D1_RECT_F r, const std::string& label, bool enabled) {
    zone(id, r, enabled);
    const float h = enabled ? play_anim_ : 0, depth = 7;
    const bool down = enabled && pressed_ == id && hover_ == id;
    const float cx = (r.left + r.right) / 2, cy = (r.top + r.bottom) / 2;
    const D2D1_RECT_F local = {r.left - cx, r.top - cy, r.right - cx, r.bottom - cy};
    const M keep = base_;
    const M m = M::Scale(1 + 0.06f * h, 1 + 0.06f * h) * M::Rotation(-2.5f * h) * M::Translation(cx, cy) * keep;
    auto g = blob(local, 22, 3.f, 101);

    base_ = M::Translation(4, depth + 5) * m;   // ombre sur le papier
    fill(g.Get(), rgb(kInk, 0.25f));
    base_ = M::Translation(0, depth) * m;       // l'epaisseur
    fill(g.Get(), enabled ? rgb(0x8f3a22) : rgb(0x7c6f5e));
    stroke(g.Get(), rgb(kInk), 3.5f);
    base_ = M::Translation(0, down ? depth - 1 : 0) * m;
    D2D1_COLOR_F face = enabled ? mix(rgb(kWarm), rgb(0xea7448), h) : rgb(0xa99b86);
    fill(g.Get(), face);
    auto shine = blob({local.left + 16, local.top + 7, local.right - 16, local.top + 22}, 7, 1.f, 102);
    fill(shine.Get(), rgb(0xffffff, enabled ? 0.22f : 0.12f));
    stroke(g.Get(), rgb(kInk), 3.5f);
    text(kTitle, 38, 0, 13, label, enabled ? rgb(0xfff4dc) : rgb(0xf1e8da, 0.85f), 1, 3.f);
    base_ = keep;
}

// Un lien manuscrit ; souligne d'un trait qui ondule au survol.
void App::link(int id, float x, float y, const std::string& label, int align) {
    const std::u16string s = ml::utf8_to_u16(label);
    const float size = 25, w = width(kHand, size, s);
    const float x0 = align == 2 ? x - w : align == 1 ? x - w / 2 : x;
    zone(id, {x0 - 8, y - size, x0 + w + 8, y + 10});
    const bool hov = hover_ == id;
    const D2D1_COLOR_F c = hov ? rgb(0x9a3d1f) : rgb(kInk);
    text16(kHand, size, x0, y, s, c);
    if (hov) squiggle(x0, x0 + w, y + 6, c, 2.2f);
}

void App::heading(const std::string& title) {
    text(kTitle, 44, kLeft, 96, title, rgb(kInk));
    squiggle(kLeft, kRight, 118, rgb(kInkSoft, 0.7f), 2);
}

// --- panneaux -----------------------------------------------------------------

void App::panel_main() {
    text(kTitle, 48, kLeft, 98, u8"Mewgenics Multi", rgb(kInk));
    text(kText, 17, kLeft + 2, 128,
         ml::fmt(tr(u8"build %s · protocole %u"), ml::build_id(), (unsigned)ml::proto_version()), rgb(kInkSoft));
    squiggle(kLeft, kRight, 146, rgb(kInkSoft, 0.7f), 2);

    struct Row { Mark mark; std::string label, phrase; bool repair; };
    std::vector<Row> rows;
    bool game_ok = false;   // "game" vient avant "appid"
    for (const ml::Check& c : checks_) {
        const bool ok = c.status == ml::Status::Ok;
        if (!strcmp(c.id, "game")) game_ok = ok;
        // steam_appid.txt ne se verifie qu'une fois le jeu trouve : pas une croix de plus.
        if (!strcmp(c.id, "version") || (!strcmp(c.id, "config") && ok) || (!strcmp(c.id, "appid") && !game_ok))
            continue;
        Row r = {ok ? Mark::Ok : c.status == ml::Status::Warning ? Mark::Warn : Mark::Bad, c.text,
                 ok ? std::string() : c.text, c.repairable};
        if (!strcmp(c.id, "steam")) {
            // Steam ferme n'empeche pas de lancer : on le dit, sans bloquer.
            r.label = ok ? tr(u8"Steam ouvert") : tr(u8"Steam fermé");
            if (!ok) r.mark = Mark::Warn;
        } else if (!strcmp(c.id, "config")) {
            r.label = tr(u8"Réglages (mgmp.json)");
        } else if (!strcmp(c.id, "game")) {
            // l'exe est la sans resources.gpak (comme check_all), pas d'apres le texte traduit
            r.label = ok ? tr(u8"Jeu trouvé")
                         : is_file(ml::game_exe(settings_)) ? tr(u8"Jeu incomplet") : tr(u8"Jeu introuvable");
            if (ok) {   // le chemin sur deux lignes au plus
                const float w = kRight - kLeft - 42;
                r.phrase = ml::wide_to_utf8(ml::game_dir(settings_));
                if (wrap(kText, 15.5f, w, r.phrase).size() > 2) r.phrase = elide(kText, 15.5f, w, r.phrase);
            }
        } else if (!strcmp(c.id, "appid")) {
            r.label = "steam_appid.txt";
            if (!repair_note_.empty()) r.phrase = repair_note_;
        } else if (!strcmp(c.id, "files")) {
            r.label = tr(u8"Fichiers du mod");
        }
        rows.push_back(r);
    }

    // Les phrases a la taille normale, ou plus petites si les problemes s'accumulent.
    const float x_text = kLeft + 42, top = 168, play_min = 418, play_h = 72;
    float ps = 15.5f, end = top;
    std::vector<std::vector<std::u16string>> lines;
    for (int pass = 0; pass < 2; ++pass, ps = 13.5f) {
        lines.clear();
        end = top;
        for (const Row& r : rows) {
            float w = kRight - x_text - (r.repair ? 122 : 0);
            lines.push_back(r.phrase.empty() ? std::vector<std::u16string>() : wrap(kText, ps, w, r.phrase));
            end += 30 + lines.back().size() * ps * 1.2f + 12;
        }
        if (end <= play_min - 6) break;
    }
    float y = top;
    for (size_t i = 0; i < rows.size(); ++i) {
        mark(rows[i].mark, kLeft, y - 1, 28);
        text(kText, 21, x_text, y + 21, rows[i].label, rgb(kInk));
        float py = y + 23 + ps * 1.3f;
        for (const auto& line : lines[i]) {
            text16(kText, ps, x_text, py, line, rgb(kInkSoft));
            py += ps * 1.2f;
        }
        if (rows[i].repair) button(kRepair, {kRight - 112, y - 2, kRight, y + 32}, tr(u8"Réparer"), false);
        y += 30 + lines[i].size() * ps * 1.2f + 12;
    }

    const float py = std::clamp(y + 6, play_min, 452.f);
    const bool ok = !blocked();
    big_button(kPlay, {kMid - 140, py, kMid + 140, py + play_h}, tr(u8"Jouer"), ok);
    if (!ok)   // court : il tient entre les deux liens du bas
        text(kText, 13.5f, kMid, std::min(py + play_h + 32, 560.f), tr(u8"Règle d'abord les croix rouges."), rgb(0x9b2f1f), 1);
    link(kToSettings, kLeft, 562, tr(u8"Réglages"), 0);
    link(kToLogs, kRight, 562, tr(u8"Journaux"), 2);
}

void App::panel_settings() {
    heading(tr(u8"Réglages"));

    // La langue : < choix >, un clic sur le nom passe au suivant. Enregistree et
    // appliquee tout de suite (cycle_lang), sans attendre « Enregistrer ».
    const float lgy = 134, lx = kLeft + 170;
    text(kText, 21, kLeft, lgy + 23, tr(u8"Langue"), rgb(kInk));
    zone(kLangNext, {lx + 40, lgy, kRight - 40, lgy + 36});
    button(kLangPrev, {lx, lgy, lx + 40, lgy + 36}, "<", false);
    button(kLangNext, {kRight - 40, lgy, kRight, lgy + 36}, ">", false);
    {
        const std::string label = lang_label();
        const float avail = kRight - lx - 80 - 16;
        float size = 19;
        while (size > 13 && width(kText, size, ml::utf8_to_u16(label)) > avail) size -= 0.5f;
        text(kText, size, (lx + kRight) / 2, lgy + 18 + size * 0.36f, elide(kText, size, avail, label),
             rgb(hover_ == kLangNext ? 0x9a3d1f : kInk), 1);
    }

    text(kText, 21, kLeft, 212, tr(u8"Le jeu (Mewgenics.exe)"), rgb(kInk));
    button(kBrowse, {kRight - 150, 184, kRight, 220}, tr(u8"Parcourir…"), false);
    const D2D1_RECT_F box = {kLeft, 228, kRight, 298};   // trois lignes de chemin
    auto g = blob(box, 8, 1.2f, 99);
    fill(g.Get(), rgb(0xfffaf0, 0.55f));
    stroke(g.Get(), rgb(kInkSoft), 2);
    const float bw = box.right - box.left - 24;
    const std::string shown = ml::wide_to_utf8(ml::game_exe(edit_));   // celui d'a cote du mod, s'il a servi
    auto path_lines = wrap(kText, 15, bw, shown);
    for (float f = 2.8f; path_lines.size() > 3 && f > 1; f -= 0.2f)   // trop long : la fin reste visible
        path_lines = wrap(kText, 15, bw, elide(kText, 15, bw * f, shown));
    float ly = box.top + 22 + (3 - (float)std::min<size_t>(path_lines.size(), 3)) * 9;   // centre dans la boite
    for (size_t i = 0; i < path_lines.size() && i < 3; ++i, ly += 18) text16(kText, 15, box.left + 12, ly, path_lines[i], rgb(kInk));

    const bool exe_ok = is_file(ml::game_exe(edit_));
    const bool gpak_ok = exe_ok && is_file(ml::game_dir(edit_) + L"\\resources.gpak");
    mark(gpak_ok ? Mark::Ok : Mark::Bad, kLeft, 308, 24);
    para(kText, 15.5f, kLeft + 34, 326, kRight - kLeft - 34,
         gpak_ok  ? tr(u8"Mewgenics.exe et resources.gpak sont là.")
         : exe_ok ? tr(u8"resources.gpak manque à côté : ce n'est pas le bon dossier.")
                  : tr(u8"Aucun Mewgenics.exe ici : choisis-le avec Parcourir."),
         rgb(kInkSoft), 19);

    const float cy = 366;
    zone(kSteam, {kLeft - 4, cy - 6, kRight, cy + 34});
    auto cb = blob({kLeft, cy, kLeft + 28, cy + 28}, 5, 1.f, 5);
    fill(cb.Get(), rgb(0xfffaf0, hover_ == kSteam ? 0.95f : 0.6f));
    stroke(cb.Get(), rgb(kInk), 2.5f);
    if (edit_.steam) mark(Mark::Ok, kLeft + 3, cy - 4, 28);
    text(kText, 20, kLeft + 42, cy + 21, tr(u8"Jouer ensemble par Steam (liste d'amis)"), rgb(kInk));
    para(kText, 15, kLeft + 42, cy + 46, kRight - kLeft - 42,
         tr(u8"Décoche si Steam pose problème : il ne reste que l'adresse, dans le menu Multijoueur du jeu."),
         rgb(kInkSoft), 18);

    if (!settings_note_.empty()) para(kText, 15.5f, kLeft, 456, kRight - kLeft, settings_note_, rgb(0x9b2f1f), 19);
    button(kSave, {kMid - 115, 484, kMid + 115, 532}, tr(u8"Enregistrer"), true);
    link(kBack, kLeft, 562, tr(u8"Retour"), 0);
}

void App::panel_logs() {
    heading(tr(u8"Journaux"));
    // Espace insecable (ici et dans ml_env, ml_loader) : le nom de l'exe ne se coupe pas.
    std::string count = logs_.count == 0 ? tr(u8"Aucun journal pour l'instant : le mod en écrit un à chaque partie.")
                        : logs_.count == 1 ? tr(u8"1 journal du mod, à côté de Mewgenics\u00A0Multi.exe.")
                                           : ml::fmt(tr(u8"%d journaux du mod, à côté de Mewgenics\u00A0Multi.exe."), logs_.count);
    float y = para(kText, 20, kLeft, 162, kRight - kLeft, count, rgb(kInk), 24);
    if (logs_.count) {
        size_t slash = logs_.latest.find_last_of(L'\\');
        std::string name = ml::wide_to_utf8(slash == std::wstring::npos ? logs_.latest : logs_.latest.substr(slash + 1));
        y = para(kText, 16.5f, kLeft, y + 6, kRight - kLeft, ml::fmt(tr(u8"Le dernier : %s"), name.c_str()), rgb(kInkSoft), 21);
        para(kText, 16.5f, kLeft, y, kRight - kLeft, ml::fmt(tr(u8"écrit le %s"), logs_.when.c_str()), rgb(kInkSoft), 21);
    }
    button(kOpenDir, {kLeft, 270, kLeft + 230, 314}, tr(u8"Ouvrir le dossier"), false);
    button(kCopyLog, {kLeft, 332, kLeft + 290, 376}, tr(u8"Copier le dernier journal"), false, logs_.count > 0);
    if (!logs_note_.empty()) {
        mark(logs_ok_ ? Mark::Ok : Mark::Bad, kLeft, 404, 24);
        para(kText, 16.5f, kLeft + 34, 422, kRight - kLeft - 34, logs_note_, rgb(kInk), 21);
    } else {
        para(kText, 16, kLeft, 422, kRight - kLeft,
             tr(u8"Pour demander de l'aide : copie le dernier journal, puis colle-le dans Discord avec Ctrl+V."),
             rgb(kInkSoft), 20);
    }
    link(kBack, kLeft, 562, tr(u8"Retour"), 0);
}

void App::panel_launch() {
    using State = ml::Launch::State;
    heading(tr(u8"Lancement"));
    if (snap_.state == State::Loaded) {
        mark(Mark::Ok, kMid - 36, 180, 72);
        text(kTitle, 28, kMid, 310, tr(u8"Le jeu se lance avec le mod…"), rgb(kInk), 1);
        text(kText, 17, kMid, 346, tr(u8"Bonne partie ! Cette fenêtre se ferme toute seule."), rgb(kInkSoft), 1);
        return;
    }
    const bool failed = snap_.state == State::Failed;
    const bool cancelled = failed && snap_.cancelled;   // voulu par le joueur : pas une panne
    // Les etapes deja passees (cochees), puis celle en cours (la roue) ou l'echec.
    float y = 150;
    for (size_t i = steps_.size() > 5 ? steps_.size() - 5 : 0; i < steps_.size(); ++i) {
        const bool last = i + 1 == steps_.size();
        std::string step = steps_[i];
        const size_t dots = sizeof(u8"…") - 1;   // une etape finie perd ses points de suspension
        if (!last && step.size() > dots && step.compare(step.size() - dots, dots, u8"…") == 0) step.resize(step.size() - dots);
        if (!last) mark(Mark::Ok, kLeft, y - 2, 24);
        else if (failed) mark(cancelled ? Mark::Warn : Mark::Bad, kLeft, y - 3, 28);
        else spinner(kLeft + 13, y + 11, 10);
        y = para(kText, last ? 18.f : 16.f, kLeft + 42, y + 17, kRight - kLeft - 42, step,
                 last ? (failed && !cancelled ? rgb(0x8e2a1c) : rgb(kInk)) : rgb(kInkSoft), last ? 22.f : 19.f) + 8;
    }
    if (failed) {
        if (!cancelled)
            para(kText, 14.5f, kLeft + 42, y + 6, kRight - kLeft - 42,
                 tr(u8"Le détail est dans loader.txt, à côté de Mewgenics\u00A0Multi.exe."), rgb(kInkSoft), 18);
        big_button(kRetry, {kMid - 140, 440, kMid + 140, 506}, tr(u8"Réessayer"), true);
        link(kBack, kLeft, 562, tr(u8"Retour"), 0);
    } else {
        para(kText, 14.5f, kLeft, 510, kRight - kLeft,
             tr(u8"Si Steam relance le jeu, ça peut prendre une minute ou deux."), rgb(kInkSoft), 19);
        link(kCancel, kLeft, 562, tr(u8"Annuler"), 0);
    }
}

// --- actions --------------------------------------------------------------------

void App::tick() {
    using State = ml::Launch::State;
    const ULONGLONG now = GetTickCount64();
    const float dt = std::min((now - last_tick_) / 1000.f, 0.1f);
    last_tick_ = now;
    const bool running = panel_ == Panel::Launch && snap_.state == State::Running;
    sun_angle_ = fmodf(sun_angle_ + dt * (running ? 14.f : 3.f), 360.f);   // le soleil s'emballe au lancement

    poll_assets();
    poll_intl();
    if (art_ && fade_ < 1) {
        fade_ = std::min(1.f, (now - fade_start_) / 450.f);
        front_dirty_ = true;
    }
    if (panel_ == Panel::Launch) {
        ml::Launch::Snapshot s = launch_.snapshot();
        if (s.seq != snap_.seq) {
            snap_ = std::move(s);
            if (!snap_.phrase.empty() && (steps_.empty() || steps_.back() != snap_.phrase)) steps_.push_back(snap_.phrase);
            front_dirty_ = true;
        }
        if (snap_.state == State::Loaded && !close_at_) close_at_ = now + 2000;
        if (close_at_ && now >= close_at_) {
            DestroyWindow(hwnd_);
            return;
        }
        if (snap_.state == State::Running) front_dirty_ = true;   // la roue tourne
    }
    if (panel_ == Panel::Main && now - checked_at_ > 2000) refresh_checks();   // Steam ouvert entre-temps...

    const float want = hover_ == kPlay || hover_ == kRetry ? 1.f : 0.f;
    if (play_anim_ != want) {
        play_anim_ += (want - play_anim_) * std::min(1.f, dt * 12);
        if (fabsf(want - play_anim_) < 0.003f) play_anim_ = want;
        front_dirty_ = true;
    }
    render();
}

void App::refresh_checks() {
    const std::string lang = settings_.lang;
    ml::config_load(paths_, &settings_);
    if (settings_.lang != lang) apply_lang();   // mgmp.json change a la main
    std::vector<ml::Check> c = ml::check_all();
    checked_at_ = GetTickCount64();
    bool same = c.size() == checks_.size();
    for (size_t i = 0; same && i < c.size(); ++i)
        same = c[i].status == checks_[i].status && c[i].text == checks_[i].text;
    if (!same) {
        checks_ = std::move(c);
        front_dirty_ = true;
    }
}

// Un point rouge empeche de lancer ; Steam ferme seulement l'avertit.
bool App::blocked() const {
    for (const ml::Check& c : checks_)
        if (c.status == ml::Status::Problem && strcmp(c.id, "steam") != 0) return true;
    return false;
}

void App::activate(int id) {
    switch (id) {
    case kClose: DestroyWindow(hwnd_); return;
    case kMin: ShowWindow(hwnd_, SW_MINIMIZE); return;
    case kPlay:
    case kRetry: play(); break;
    case kRepair: {
        std::string why;
        repair_note_ = ml::repair_appid(hwnd_, &why) == ml::Repair::Done ? std::string() : why;
        refresh_checks();
        break;
    }
    case kToSettings:
        edit_ = settings_;
        settings_note_.clear();
        panel_ = Panel::Settings;
        break;
    case kToLogs:
        logs_ = ml::logs_find(paths_);
        logs_note_.clear();
        panel_ = Panel::Logs;
        break;
    case kBack: back(); break;
    case kBrowse: browse(); break;
    case kSteam: edit_.steam = !edit_.steam; break;
    case kSave: save(); break;
    case kOpenDir: ml::logs_open_folder(paths_); break;
    case kCopyLog:
        logs_ok_ = ml::logs_copy_latest(hwnd_, paths_);
        logs_note_ = logs_ok_ ? tr(u8"Journal copié : colle-le dans Discord avec Ctrl+V.")
                              : tr(u8"Le presse-papiers n'a pas pu être rempli : réessaie, ou ouvre le dossier.");
        break;
    case kCancel: launch_.cancel(); break;
    case kLangPrev: cycle_lang(-1); break;
    case kLangNext: cycle_lang(1); break;
    }
    front_dirty_ = true;
    render();
}

void App::enter() {
    if ((panel_ == Panel::Main && !blocked()) ||
        (panel_ == Panel::Launch && snap_.state == ml::Launch::State::Failed))
        activate(kPlay);
    else if (panel_ == Panel::Settings)
        activate(kSave);
}

void App::back() {
    switch (panel_) {
    case Panel::Main: DestroyWindow(hwnd_); return;
    case Panel::Launch:
        if (snap_.state == ml::Launch::State::Running) launch_.cancel();
        else if (snap_.state == ml::Launch::State::Failed) panel_ = Panel::Main;
        break;
    default: panel_ = Panel::Main; break;
    }
    if (panel_ == Panel::Main) refresh_checks();
    front_dirty_ = true;
    render();
}

void App::play() {
    refresh_checks();
    panel_ = Panel::Main;
    if (blocked()) return;
    steps_.clear();
    snap_ = {};
    close_at_ = 0;
    hover_ = pressed_ = kNone;
    launch_.start(paths_.dir);
    panel_ = Panel::Launch;
}

void App::browse() {
    ComPtr<IFileOpenDialog> dlg;
    if (FAILED(CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&dlg)))) return;
    COMDLG_FILTERSPEC spec = {L"Mewgenics.exe", L"Mewgenics.exe"};
    dlg->SetFileTypes(1, &spec);
    dlg->SetTitle(ml::utf8_to_wide(tr(u8"Où est Mewgenics.exe ?")).c_str());
    FILEOPENDIALOGOPTIONS opts = 0;
    dlg->GetOptions(&opts);
    dlg->SetOptions(opts | FOS_FORCEFILESYSTEM | FOS_FILEMUSTEXIST);
    ComPtr<IShellItem> folder;
    if (SUCCEEDED(SHCreateItemFromParsingName(ml::game_dir(edit_).c_str(), nullptr, IID_PPV_ARGS(&folder))))
        dlg->SetFolder(folder.Get());
    ComPtr<IShellItem> item;
    PWSTR chosen = nullptr;
    if (dlg->Show(hwnd_) != S_OK || FAILED(dlg->GetResult(&item)) ||
        FAILED(item->GetDisplayName(SIGDN_FILESYSPATH, &chosen)))
        return;
    edit_.game = ml::wide_to_utf8(chosen);
    CoTaskMemFree(chosen);
}

void App::save() {
    std::string why;
    if (!ml::config_save(paths_, edit_, &why)) {
        settings_note_ = why;
        return;
    }
    const bool moved = edit_.game != settings_.game;
    settings_ = edit_;
    // Le jeu a change de place et l'art manquait : on le relit au nouvel endroit.
    if (moved && art_missing_ && !no_art_ && !pending_.valid()) start_assets();
    // Les polices de l'alphabet aussi, si elles manquaient.
    if (moved && intl_[0].glyphs.empty() && !intl_pending_.valid()) {
        intl_lang_.clear();
        poll_intl();
    }
    panel_ = Panel::Main;
    refresh_checks();
}

const Zone* App::hit(D2D1_POINT_2F p) const {
    M inv = paper_m_;
    inv.Invert();
    const D2D1_POINT_2F q = inv.TransformPoint(p);
    for (auto it = zones_.rbegin(); it != zones_.rend(); ++it)
        if (inside(it->r, it->paper ? q : p)) return &*it;
    return nullptr;
}

const Zone* App::hit_screen(POINT p) const {
    ScreenToClient(hwnd_, &p);
    return hit({p.x / scale_, p.y / scale_});
}

} // namespace

int WINAPI wWinMain(HINSTANCE inst, HINSTANCE, PWSTR, int) {
    // La copie en administrateur de steam_appid.txt (ml_env.h) : rien d'autre.
    int rc = ml::handle_command_line();
    if (rc >= 0) return rc;

    // Un seul launcher a la fois : un double-clic de trop ne lance pas deux jeux.
    HANDLE once = CreateMutexW(nullptr, FALSE, L"MewgenicsMultiLauncher");
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        if (HWND other = FindWindowW(L"MewgenicsMultiLauncher", nullptr)) {
            ShowWindow(other, SW_RESTORE);
            SetForegroundWindow(other);
        }
        return 0;
    }
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    {
        App app;
        rc = app.run(inst);
    }
    CoUninitialize();
    if (once) CloseHandle(once);
    return rc;
}
