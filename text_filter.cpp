#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <set>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <cctype>

namespace py = pybind11;

static const std::set<char> DELIMITERS = {
    ' ', '.', ',', ':', ';', '?', '!', '\n', '\t', '-', '(', ')', 
    '[', ']', '{', '}', '=', '<', '>', '\"', '\'', '&', '^', '%', 
    '$', '#', '@', '*', '+', '/', '|', '\\', '~'
};

static const std::set<std::string> END_OF_SENTENCE = {
    ".", "?", "!", "\n"
};

std::string toLower(const std::string& input) {
    std::string result = input;
    for (char& c : result) {
        c = std::tolower(static_cast<unsigned char>(c));
    }
    return result;
}

std::vector<std::string> tokenize(const std::string& input) {
    if (input.empty()) {
        return {};
    }

    std::vector<std::string> tokens;
    std::string cur_token;
    bool char_is_delimiter, is_digit_boundary, last_char_was_numeric = isdigit(input[0]);

    for (char c : input) {
        char_is_delimiter = DELIMITERS.find(c) != DELIMITERS.end();
        is_digit_boundary = last_char_was_numeric != isdigit(c);
        last_char_was_numeric = isdigit(c);

        if (!char_is_delimiter && !is_digit_boundary) {
            cur_token.push_back(c);
            continue;
        }

        if (!cur_token.empty()) {
            tokens.push_back(cur_token);
            cur_token.clear();
        }

        if (is_digit_boundary) {
            cur_token.push_back(c);
            continue;
        }

        tokens.push_back(std::string(1, c));
    }

    if (!cur_token.empty()) {
        tokens.push_back(cur_token);
    }

    return tokens;
}

struct FilteringInfo {
    int word_count;
    int oov_count;

    std::string __repr__() const {
        std::stringstream ss;
        ss << "FilteringInfo(word_count=" << word_count << ", oov_count=" << oov_count << ")";
        return ss.str();
    }
};

bool is_integer(const std::string& s) {
    return std::all_of(s.begin(), s.end(), ::isdigit);
}

bool is_delimiter(const std::string& s) {
    return s.size() == 1 && DELIMITERS.find(s[0]) != DELIMITERS.end();
}

std::vector<std::pair<std::string, FilteringInfo>> filter_vocab(
    const std::string& text,
    const std::unordered_set<std::string>& vocab,
    int min_words,
    float p_max_oov,
    bool from_sentence_start_only
) {
    auto tokens = tokenize(text);

    std::vector<std::pair<std::string, FilteringInfo>> chunks_with_info;
    std::string current_chunk;
    int word_count = 0, oov_count = 0;
    float p_oov;
    bool start_of_sentence = true;

    for (auto& token : tokens) {
        if (from_sentence_start_only && current_chunk.empty() && !start_of_sentence) {
            current_chunk.clear();
            start_of_sentence = END_OF_SENTENCE.find(token) != END_OF_SENTENCE.end();
            continue;
        }

        if (current_chunk.empty() && (token == " " || token == "\n")) {
            continue;
        }

        if (!is_delimiter(token)) {
            word_count++;
        }
        bool is_vocab_word = vocab.find(toLower(token)) != vocab.end();

        if (is_vocab_word || is_integer(token) || is_delimiter(token)) {
            current_chunk += token;
        } else {
            oov_count++;
            p_oov = static_cast<float>(oov_count) / std::max(word_count, min_words);
            if (p_oov <= p_max_oov) {
                current_chunk += token;
            } else {
                // trim trailing space and \n
                while (current_chunk.size() > 0 && (current_chunk.back() == ' ' || current_chunk.back() == '\n')) {
                    current_chunk.pop_back();
                }
                if (word_count > min_words) {
                    chunks_with_info.push_back({current_chunk, {word_count, oov_count}});
                }
                current_chunk.clear();
                oov_count = 0;
                word_count = 0;
            }
        }
        start_of_sentence = END_OF_SENTENCE.find(token) != END_OF_SENTENCE.end();
    }

    if (!current_chunk.empty() && word_count > min_words) {
        while (current_chunk.size() > 0 && (current_chunk.back() == ' ' || current_chunk.back() == '\n')) {
            current_chunk.pop_back();
        }
        chunks_with_info.push_back({current_chunk, {word_count, oov_count}});
    }

    return chunks_with_info;
}

PYBIND11_MODULE(text_filter, m) {
    m.doc() = "Text Filter using vocabulary and pybind11";

    m.def("tokenize", &tokenize, "Tokenize a given string");

    py::class_<FilteringInfo>(m, "FilteringInfo")
        .def(py::init<>())
        .def_readwrite("word_count", &FilteringInfo::word_count)
        .def_readwrite("oov_count", &FilteringInfo::oov_count)
        .def("__repr__", &FilteringInfo::__repr__);

    m.def("filter_vocab", &filter_vocab, R"pbdoc(
        Filter vocabulary based on given parameters.

        Parameters
        ----------
        text : str
            The input text to be filtered.
        vocab : set[str]
            The vocabulary set.
        min_words : int
            The minimum number of words to consider for a chunk.
        p_max_oov : float
            The maximum out-of-vocabulary word percentage allowed in a chunk.
        from_sentence_start_only : bool
            Whether to only start chunks at the start of sentences.

        Returns
        -------
        list[tuple[str, FilteringInfo]]
            List of tuples where each tuple consists of the chunk string and its associated FilteringInfo.
    )pbdoc");
}
