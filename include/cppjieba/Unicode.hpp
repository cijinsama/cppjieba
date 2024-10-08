#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <ostream>
#include "limonp/LocalVector.hpp"
#include "limonp/StringUtil.hpp"

#if defined(_WIN32) || defined(_WIN64)
#include <ciso646>
#endif

namespace cppjieba {

using std::string;
using std::vector;

typedef uint32_t Rune;

struct Word {
    string word;
    uint32_t offset;
    uint32_t unicode_offset;
    uint32_t unicode_length;
    Word(const string& w, uint32_t o)
        : word(w), offset(o) {
    }
    Word(const string& w, uint32_t o, uint32_t unicode_offset, uint32_t unicode_length)
        : word(w), offset(o), unicode_offset(unicode_offset), unicode_length(unicode_length) {
    }
}; // struct Word

inline std::ostream& operator << (std::ostream& os, const Word& w) {
    return os << "{\"word\": \"" << w.word << "\", \"offset\": " << w.offset << "}";
}

struct RuneInfo {
    Rune rune;
    uint32_t offset;
    uint32_t len;
    uint32_t unicode_offset = 0;
    uint32_t unicode_length = 0;
    RuneInfo(): rune(0), offset(0), len(0) {
    }
    RuneInfo(Rune r, uint32_t o, uint32_t l)
        : rune(r), offset(o), len(l) {
    }
    RuneInfo(Rune r, uint32_t o, uint32_t l, uint32_t unicode_offset, uint32_t unicode_length)
        : rune(r), offset(o), len(l), unicode_offset(unicode_offset), unicode_length(unicode_length) {
    }
}; // struct RuneInfo

inline std::ostream& operator << (std::ostream& os, const RuneInfo& r) {
    return os << "{\"rune\": \"" << r.rune << "\", \"offset\": " << r.offset << ", \"len\": " << r.len << "}";
}

typedef limonp::LocalVector<Rune> RuneArray;
typedef limonp::LocalVector<struct RuneInfo> RuneStrArray;

// [left, right]
struct WordRange {
    RuneStrArray::const_iterator left;
    RuneStrArray::const_iterator right;
    WordRange(RuneStrArray::const_iterator l, RuneStrArray::const_iterator r)
        : left(l), right(r) {
    }
    size_t Length() const {
        return right - left + 1;
    }

    bool IsAllAscii() const {
        for (RuneStrArray::const_iterator iter = left; iter <= right; ++iter) {
            if (iter->rune >= 0x80) {
                return false;
            }
        }

        return true;
    }
}; // struct WordRange


inline bool DecodeRunesInString(const string& s, RuneArray& arr) {
    arr.clear();
    return limonp::Utf8ToUnicode32(s, arr);
}

inline RuneArray DecodeRunesInString(const string& s) {
    RuneArray result;
    DecodeRunesInString(s, result);
    return result;
}

inline bool DecodeRunesInString(const string& s, RuneStrArray& runes) {
    RuneArray arr;

    if (not DecodeRunesInString(s, arr)) {
        return false;
    }

    runes.clear();

    uint32_t offset = 0;

    for (uint32_t i = 0; i < arr.size(); ++i) {
        const uint32_t len = limonp::UnicodeToUtf8Bytes(arr[i]);
        RuneInfo x(arr[i], offset, len, i, 1);
        runes.push_back(x);
        offset += len;
    }

    return true;
}

class RunePtrWrapper {
public:
    const RuneInfo * m_ptr = nullptr;

public:
    explicit RunePtrWrapper(const RuneInfo * p) : m_ptr(p) {}

    uint32_t operator *() {
        return m_ptr->rune;
    }

    RunePtrWrapper operator ++(int) {
        m_ptr ++;
        return RunePtrWrapper(m_ptr);
    }

    bool operator !=(const RunePtrWrapper & b) const {
        return this->m_ptr != b.m_ptr;
    }
};

inline string EncodeRunesToString(RuneStrArray::const_iterator begin, RuneStrArray::const_iterator end) {
    string str;
    RunePtrWrapper it_begin(begin), it_end(end);
    limonp::Unicode32ToUtf8(it_begin, it_end, str);
    return str;
}

class Unicode32Counter {
public :
    size_t length = 0;
    void clear() {
        length = 0;
    }
    void push_back(uint32_t) {
        ++length;
    }
};

inline size_t Utf8CharNum(const char * str, size_t length) {
    Unicode32Counter c;

    if (limonp::Utf8ToUnicode32(str, length, c)) {
        return c.length;
    }

    return 0;
}

inline size_t Utf8CharNum(const string & str) {
    return Utf8CharNum(str.data(), str.size());
}

inline bool IsSingleWord(const string& str) {
    return Utf8CharNum(str) == 1;
}


// [left, right]
inline Word GetWordFromRunes(const string& s, RuneStrArray::const_iterator left, RuneStrArray::const_iterator right) {
    assert(right->offset >= left->offset);
    uint32_t len = right->offset - left->offset + right->len;
    uint32_t unicode_length = right->unicode_offset - left->unicode_offset + right->unicode_length;
    return Word(s.substr(left->offset, len), left->offset, left->unicode_offset, unicode_length);
}

inline void GetWordsFromWordRanges(const string& s, const vector<WordRange>& wrs, vector<Word>& words) {
    for (size_t i = 0; i < wrs.size(); i++) {
        words.push_back(GetWordFromRunes(s, wrs[i].left, wrs[i].right));
    }
}

inline void GetStringsFromWords(const vector<Word>& words, vector<string>& strs) {
    strs.resize(words.size());

    for (size_t i = 0; i < words.size(); ++i) {
        strs[i] = words[i].word;
    }
}

const size_t MAX_WORD_LENGTH = 512;

} // namespace cppjieba

