    #include "Indexer.h"

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
        // Конвертация строки из UTF-8 в wstring
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
        std::wstring wText = converter.from_bytes(HTMLContent);

        // Удаление содержимого <script>, <style>, <noscript>, <iframe> (включая их содержимое)
        wText = std::regex_replace(wText, std::wregex(L"<(script|style|noscript|iframe)[^>]*>[\\s\\S]*?</\\1>", std::regex_constants::icase), L" ");

        // Удаление HTML-комментариев
        wText = std::regex_replace(wText, std::wregex(L"<!--.*?-->"), L" ");

        // Удаление всех HTML-тегов
        wText = std::regex_replace(wText, std::wregex(L"<[^>]*>"), L" ");

        // Удаление JSON и HTML-атрибутов
        wText = std::regex_replace(wText, std::wregex(L"[\"{\\}\\[\\]:]+"), L" ");

        // Удаление слов с подчёркиванием или других технических конструкций
        wText = std::regex_replace(wText, std::wregex(L"\\b[a-zA-Z0-9_]+_[a-zA-Z0-9_]+\\b"), L" ");

        // Замена всех ненужных символов
        wText = std::regex_replace(wText, std::wregex(L"[^\\w\\s-]"), L" ");

        // Приведение текста к нижнему регистру
        std::locale loc("ru_RU.UTF-8");
        for (auto& ch : wText) {
            ch = std::tolower(ch, loc);
        }

        // Удаление лишних пробелов
        wText = std::regex_replace(wText, std::wregex(L"\\s+"), L" ");

        // Конвертация обратно в UTF-8
        std::string result = converter.to_bytes(wText);

        return result;
    }