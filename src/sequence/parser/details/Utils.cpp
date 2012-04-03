/*
 * Utils.cpp
 *
 *  Created on: Apr 2, 2012
 *      Author: Guillaume Chatelet
 */

#include "Utils.h"

using namespace std;

namespace sequence {
namespace parser {
namespace details {

static inline bool isDigit(string::value_type c) {
    return c >= '0' && c <= '9';
}

static inline size_t atoi(string::const_iterator begin, const string::const_iterator end) {
    size_t value = 0;
    for (; begin != end; ++begin)
        value = value * 10 + (*begin - '0');
    return value;
}

void extractPattern(string &pattern, Locations &locations, Values &values) {
    locations.clear();
    values.clear();
    typedef string::iterator Itr;
    const Itr begin = pattern.begin();
    Itr current = begin;
    const Itr end = pattern.end();
    while ((current = find_if(current, end, isDigit)) != end) {
        Location location;
        location.first = distance(begin, current);
        const Itr pastDigitEnd = find_if(current, end, not1(ptr_fun(isDigit)));
        location.count = distance(current, pastDigitEnd);
        values.push_back(atoi(current, pastDigitEnd));
        locations.push_back(location);
        fill(current, pastDigitEnd, '#');
        current = pastDigitEnd;
    }
}

void insert(TmpData &tmpData, PatternsPerDir &map, string key) {
    extractPattern(key, tmpData.locations, tmpData.values);
    PatternsPerDir::iterator keyItr = map.find(key);
    if (keyItr == map.end())
        keyItr = map.insert(make_pair(key, Pattern(key, tmpData.locations))).first;
    keyItr->second.insert(tmpData.values);
}

void insertPath(TmpData &tmpData, AllPatterns &allPatterns, const std::string& absolutePath) {
    const size_t lastSeparator = absolutePath.find_last_of("/\\");
    const bool emptyParent = lastSeparator == string::npos;
    static const string EMPTY;
    const string parent = emptyParent ? EMPTY : absolutePath.substr(0, lastSeparator);
    AllPatterns::iterator found = allPatterns.find(parent);
    if (found == allPatterns.end())
        found = allPatterns.insert(make_pair(parent, PatternsPerDir())).first;
    const string filename = emptyParent ? absolutePath.c_str() : absolutePath.c_str() + lastSeparator + 1;
    insert(tmpData, found->second, filename);
}

} /* namespace details */
} /* namespace parser */
} /* namespace sequence */
