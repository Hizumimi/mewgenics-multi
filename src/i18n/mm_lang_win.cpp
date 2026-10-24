// mm_lang_win.cpp -- voir mm_lang_win.h.
#include "mm_lang_win.h"

#include <windows.h>
#include <shlobj.h>

#include <fstream>
#include <sstream>

namespace mm {

std::wstring game_settings_newest() {
    wchar_t appdata[MAX_PATH] = {};
    if (FAILED(SHGetFolderPathW(nullptr, CSIDL_APPDATA, nullptr, SHGFP_TYPE_CURRENT, appdata)) || !appdata[0])
        return {};
    const std::wstring root = std::wstring(appdata) + L"\\Glaiel Games\\Mewgenics\\";
    WIN32_FIND_DATAW fd{};
    HANDLE h = FindFirstFileW((root + L"*").c_str(), &fd);
    if (h == INVALID_HANDLE_VALUE) return {};
    std::wstring best;
    uint64_t best_stamp = 0;
    do {
        if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) || fd.cFileName[0] == L'.') continue;
        const std::wstring path = root + fd.cFileName + L"\\settings.txt";
        uint64_t s = 0;
        if (file_stamp(path, &s) && s >= best_stamp) {
            best = path;
            best_stamp = s;
        }
    } while (FindNextFileW(h, &fd));
    FindClose(h);
    return best;
}

bool file_stamp(const std::wstring& path, uint64_t* out) {
    WIN32_FILE_ATTRIBUTE_DATA a{};
    if (!GetFileAttributesExW(path.c_str(), GetFileExInfoStandard, &a) || (a.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
        return false;
    *out = (uint64_t)a.ftLastWriteTime.dwHighDateTime << 32 | a.ftLastWriteTime.dwLowDateTime;
    return true;
}

bool game_settings_lang(const std::wstring& path, Lang* out) {
    std::ifstream in(path.c_str(), std::ios::binary);
    if (!in) return false;
    std::ostringstream ss;
    ss << in.rdbuf();
    return lang_from_settings(ss.str(), out);
}

} // namespace mm
