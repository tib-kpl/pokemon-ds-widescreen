// Translations of the menus' text (the settings page, the game list, the pause menu).
//
// lang/en.json is the reference: every text of the menus with a stable id ("settings.fps_counter").
// lang/xx.json (lang/fr.json) holds the same ids with the text in another language. The code
// still has the English wording in it, and it is looked up in en.json to find the id, so that
// a translator only ever works on the JSON files. tools/i18n_check.py lists what is missing,
// what is left over and what no longer matches the code.
//
// CMakeLists.txt embeds every lang/*.json in the program (ssr_lang_data.h): nothing to install.
// A text without a translation is shown in English. To add a language: copy lang/fr.json to
// lang/xx.json, translate the values, and add the language to the "language" setting in
// launcher.h.
//
// The bitmap font has capitals, digits and a few signs only (ui.h, Canvas), so the translation
// is folded to what it can draw: accented letters are single bytes (\x05 and up) that the
// canvas draws as the letter with its accent, lower case is drawn as capitals.
#pragma once
#include <cstdlib>
#include <cstring>
#include <initializer_list>
#include <string>
#include <unordered_map>

#if defined(__has_include)
#if __has_include(<SDL2/SDL.h>)
#include <SDL2/SDL.h>
#define SSR_I18N_SDL 1
#endif
#endif

namespace i18n
{

struct Embedded
{
    const char* code;
    const char* text;
};
inline const Embedded* EmbeddedLanguages()
{
    static const Embedded list[] = {
#include "ssr_lang_data.h"
        { nullptr, nullptr },
    };
    return list;
}

// The internal codes of accented capitals (Canvas::Text draws them).
enum : char
{
    ACUTE_E = '\x05', GRAVE_E = '\x06', CIRC_E = '\x07', GRAVE_A = '\x08', CIRC_A = '\x0B', CEDILLA_C = '\x0C',
    DIAER_E = '\x0E', CIRC_I = '\x0F', DIAER_I = '\x10', CIRC_O = '\x11', GRAVE_U = '\x12', CIRC_U = '\x13',
    DIAER_U = '\x14',
};

// UTF-8 to what the font can draw. Characters it cannot draw are left out.
inline std::string Fold(const std::string& s)
{
    bool plain = true;
    for (unsigned char c : s)
        if (c >= 0x80) { plain = false; break; }
    if (plain) return s;
    std::string out;
    for (size_t i = 0; i < s.size(); i++)
    {
        const unsigned char c = (unsigned char)s[i];
        if (c < 0x80) { out += (char)c; continue; }
        if (c == 0xC3 && i + 1 < s.size())
        {
            const unsigned char d = (unsigned char)s[++i] & 0xDF; // (lower case drawn as capitals)
            switch (d)
            {
            case 0x80: out += GRAVE_A; break;
            case 0x82: out += CIRC_A; break;
            case 0x87: out += CEDILLA_C; break;
            case 0x88: out += GRAVE_E; break;
            case 0x89: out += ACUTE_E; break;
            case 0x8A: out += CIRC_E; break;
            case 0x8B: out += DIAER_E; break;
            case 0x8E: out += CIRC_I; break;
            case 0x8F: out += DIAER_I; break;
            case 0x94: out += CIRC_O; break;
            case 0x99: out += GRAVE_U; break;
            case 0x9B: out += CIRC_U; break;
            case 0x9C: out += DIAER_U; break;
            default: break;
            }
            continue;
        }
        if (c == 0xC5 && i + 1 < s.size() && ((unsigned char)s[i + 1] == 0x92 || (unsigned char)s[i + 1] == 0x93))
        {
            out += "OE";
            i++;
            continue;
        }
        if (c == 0xE2 && i + 2 < s.size() && (unsigned char)s[i + 1] == 0x80 &&
            ((unsigned char)s[i + 2] == 0x99 || (unsigned char)s[i + 2] == 0x98))
        {
            out += '\'';
            i += 2;
            continue;
        }
        // Any other multi-byte character: skipped.
        while (i + 1 < s.size() && ((unsigned char)s[i + 1] & 0xC0) == 0x80) i++;
    }
    return out;
}

// A JSON file that is one object of strings ({ "id": "text", ... }): the pairs in it. Anything
// else in the file is ignored.
inline std::unordered_map<std::string, std::string> ParseJson(const char* p)
{
    std::unordered_map<std::string, std::string> out;
    auto skip = [&]() { while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n' || *p == ',' || *p == ':') p++; };
    auto str = [&](std::string& s) {
        s.clear();
        if (*p != '"') return false;
        p++;
        while (*p && *p != '"')
        {
            if (*p != '\\') { s += *p++; continue; }
            p++;
            switch (*p)
            {
            case 'n': s += '\n'; break;
            case 't': s += '\t'; break;
            case 'r': s += '\r'; break;
            case 'u':
            {
                unsigned cp = 0;
                for (int k = 1; k <= 4 && p[k]; k++) cp = cp * 16 + (unsigned)(isdigit((unsigned char)p[k]) ? p[k] - '0' : (tolower((unsigned char)p[k]) - 'a' + 10));
                p += 4;
                if (cp < 0x80) s += (char)cp;
                else if (cp < 0x800) { s += (char)(0xC0 | cp >> 6); s += (char)(0x80 | (cp & 0x3F)); }
                else { s += (char)(0xE0 | cp >> 12); s += (char)(0x80 | ((cp >> 6) & 0x3F)); s += (char)(0x80 | (cp & 0x3F)); }
                break;
            }
            default: s += *p; break; // (\" \\ \/)
            }
            if (*p) p++;
        }
        if (*p == '"') p++;
        return true;
    };
    while (*p && *p != '{') p++;
    if (*p) p++;
    for (;;)
    {
        skip();
        std::string k, v;
        if (!str(k)) break;
        skip();
        if (!str(v)) break;
        out[k] = v;
    }
    return out;
}

struct State
{
    std::unordered_map<std::string, std::string> table; // English text to the chosen language's; empty: English
    std::string code = "en";
};
inline State& Current()
{
    static State s;
    return s;
}

// The language the device is set to ("fr" for fr_FR): "en" when it is unknown.
inline std::string SystemLanguage()
{
    std::string code;
#ifdef SSR_I18N_SDL
#if SDL_VERSION_ATLEAST(2, 0, 14)
    if (SDL_Locale* loc = SDL_GetPreferredLocales())
    {
        if (loc[0].language) code = loc[0].language;
        SDL_free(loc);
    }
#endif
#endif
    if (code.empty())
        for (const char* name : { "LC_ALL", "LC_MESSAGES", "LANG", "LANGUAGE" })
            if (const char* v = getenv(name))
                if (*v && strcmp(v, "C") && strcmp(v, "POSIX"))
                {
                    code = v;
                    break;
                }
    code = code.substr(0, code.find_first_of("_-.:"));
    for (char& c : code) c = (char)tolower((unsigned char)c);
    return code.empty() ? "en" : code;
}

inline const char* EmbeddedText(const char* code)
{
    for (const Embedded* e = EmbeddedLanguages(); e->code; e++)
        if (!strcmp(code, e->code)) return e->text;
    return nullptr;
}

// Chooses the language: a code ("fr"), or "auto"/"" for the device's. One without a file is
// English.
inline void Set(std::string code)
{
    if (code.empty() || code == "auto") code = SystemLanguage();
    State& st = Current();
    st.table.clear();
    st.code = "en";
    const char* english = EmbeddedText("en");
    const char* other = code == "en" ? nullptr : EmbeddedText(code.c_str());
    if (!english || !other) return;
    st.code = code;
    const auto en = ParseJson(english), tr = ParseJson(other);
    for (const auto& e : en)
    {
        const auto t = tr.find(e.first);
        if (t != tr.end() && !t->second.empty()) st.table[e.second] = Fold(t->second);
    }
}

// The text in the chosen language (what the font can draw), or as it is.
inline std::string Tr(const std::string& s)
{
    const State& st = Current();
    if (!st.table.empty())
    {
        const auto it = st.table.find(s);
        if (it != st.table.end()) return it->second;
        // (The canvas draws lower case as capitals, so a text in lower case, as a layout's
        // name, is looked up in capitals too.)
        std::string up = s;
        for (char& c : up)
            if (c >= 'a' && c <= 'z') c = (char)(c - 32);
        if (up != s)
        {
            const auto iu = st.table.find(up);
            if (iu != st.table.end()) return iu->second;
        }
    }
    return Fold(s);
}


// A text with parts that change ({0}, {1}...): the pattern is translated as a whole, the parts
// are put in afterwards (and translated too when they are texts of the menus, as a button's
// name). Write the pattern in English in the code and in lang/en.json with the {n} in it.
inline std::string Fmt(const std::string& pattern, std::initializer_list<std::string> args)
{
    const std::string t = Tr(pattern);
    std::string out;
    for (size_t i = 0; i < t.size(); i++)
    {
        if (t[i] == '{' && i + 2 < t.size() && t[i + 1] >= '0' && t[i + 1] <= '9' && t[i + 2] == '}')
        {
            const size_t n = (size_t)(t[i + 1] - '0');
            if (n < args.size()) out += Tr(*(args.begin() + n));
            i += 2;
            continue;
        }
        out += t[i];
    }
    return out;
}

}
