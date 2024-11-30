#include "Indexer.h"

#include <boost/regex.hpp>
#include <boost/locale.hpp>

#include <iostream>
#include <regex>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <sstream>
#include <codecvt>

std::vector<std::pair<std::string, int>> Indexer::index(const std::string& content) {
    std::string cleanedContent = removeHtmlTags(content);

    std::unordered_map<std::string, int> wordCount;
    std::istringstream stream(cleanedContent);
    std::string word;

    while (stream >> word) {
        if (word.length() >= 3 && word.length() <= 32) {
            wordCount[word]++;
        }
    }

    std::vector<std::pair<std::string, int>> wordFrequency(wordCount.begin(), wordCount.end());

    std::sort(wordFrequency.begin(), wordFrequency.end(),
        [](const std::pair<std::string, int>& a, const std::pair<std::string, int>& b) {
            return a.second > b.second;
        });

    return wordFrequency;
}

std::string Indexer::removeHtmlTags(const std::string& HTMLContent) {
    // Устанавливаем локаль для обработки Unicode
    std::locale::global(std::locale("ru_RU.UTF-8"));

    // Конвертация строки из UTF-8 в wstring
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::wstring wText = converter.from_bytes(HTMLContent);

    // Удаление содержимого <script>, <style>, <noscript>, <iframe> (включая их содержимое)
    wText = std::regex_replace(wText, std::wregex(L"<(script|style|noscript|iframe)[^>]*>[\\s\\S]*?</\\1>", std::regex_constants::icase), L" ");

    // Удаление HTML-комментариев
    wText = std::regex_replace(wText, std::wregex(L"<!--.*?-->"), L" ");

    // Удаление всех HTML-тегов
    wText = std::regex_replace(wText, std::wregex(L"<[^>]*>"), L" ");

    // Удаление HTML-сущностей (&nbsp;, &lt;, &gt;)
    wText = std::regex_replace(wText, std::wregex(L"&[a-zA-Z0-9#]+;"), L" ");

    // Удаление CSS-значений
    wText = std::regex_replace(wText, std::wregex(L"\\b[0-9]+(px|em|rem|%)\\b", std::regex_constants::icase), L" ");
    wText = std::regex_replace(wText, std::wregex(L"\\bhsla?\\([^\\)]*\\)\\b", std::regex_constants::icase), L" ");

    // Замена всех ненужных символов
    wText = std::regex_replace(wText, std::wregex(L"[^\\w\\s-]"), L" ");

    // Приведение текста к нижнему регистру
    std::locale loc("ru_RU.UTF-8");
    for (auto& ch : wText) {
        ch = std::tolower(ch, loc);
    }

    // Удаление лишних пробелов
    wText = std::regex_replace(wText, std::wregex(L"\\s+"), L" ");

    // Фильтрация некорректных символов UTF-8
    wText = cleanUtf8String(wText);

    // Конвертация обратно в UTF-8
    std::string result = converter.to_bytes(wText);

    return result;
}

std::wstring Indexer::cleanUtf8String(const std::wstring& input) {
    std::wstring result;
    for (wchar_t ch : input) {
        if ((ch >= 0x20 && ch <= 0xD7FF) || (ch >= 0xE000 && ch <= 0xFFFD)) {
            result += ch;
        }
    }
    return result;
}