/*
 * Utils.h
 *
 *  Created on: Apr 2, 2012
 *      Author: Guillaume Chatelet
 */

#ifndef UTILS_H_
#define UTILS_H_

#include <sequence/Config.h>
#include <sequence/Range.h>
#include <sequence/Sequence.h>
#include <sequence/BrowseItem.h>
#include <boost/bind.hpp>
#include <boost/unordered_map.hpp>
#include <boost/container/flat_set.hpp>
#include <vector>
#include <numeric>
#include <iterator>

#include <iostream>

namespace sequence {
namespace parser {
namespace details {

/**
 * The type used to store values found in the pattern
 */
typedef unsigned int value_type;

/**
 * A set of numbers found in filenames
 */
typedef std::vector<value_type> Values;

/**
 * A location within a string
 */
struct SEQUENCEPARSER_LOCAL Location {
    unsigned char first;
    unsigned char count;
    Location() {
    }
    Location(unsigned char first, unsigned char count) :
                    first(first), count(count) {
    }
};

/**
 * A set of locations
 */
typedef std::vector<Location> Locations;

/**
 * Extract all the numbers in a filename and their respective locations.
 * 'filename' is then set to the actual pattern with all its number replaced
 * by some #.
 * eg : file-20.1234.cr2
 * will produce the following locations : [5,2],[8,4],[15,1]
 * will produce the following number    : 20,1234,2
 * will produce the following pattern   : file-##.####.cr#
 */
SEQUENCEPARSER_LOCAL void extractPattern(std::string &filename, Locations &locations, Values &numbers);

/**
 * In a sorted container, count the number of different values
 */
template<typename ForwardIterator>
size_t count_different(ForwardIterator begin, const ForwardIterator end) {
    if (begin == end)
        return 0;
    size_t count = 1;
    for (typename ForwardIterator::value_type previous = *begin, current = previous; begin != end; previous = current, current = *begin) {
        ++begin;
        if (current != previous)
            ++count;
    }
    return count;
}

/**
 * A set of locations
 */
typedef std::vector<Range> Ranges;

/**
 * Given a sorted range, gives the consecutive ranges within begin and end as well
 * as the minimal step
 */
template<typename ForwardIterator>
Ranges getRangesAndStep(ForwardIterator begin, const ForwardIterator end, size_t &step) {
    step = 1;
    Ranges ranges;
    const size_t size = std::distance(begin, end);
    if (size == 0)
        return ranges;
    if (size == 1) {
        ranges.push_back(Range(*begin, *begin));
        return ranges;
    }
    typedef typename ForwardIterator::value_type value_type;
    typedef typename std::vector<value_type> Values;
    typedef typename Values::const_iterator CItr;
    Values derivative(size);
    std::adjacent_difference(begin, end, derivative.begin());
    const CItr d_begin = derivative.begin();
    const CItr d_end = derivative.end();
    step = std::max(value_type(1), *std::min_element(d_begin, d_end));
    for (CItr d_itr = d_begin; d_itr != d_end; ++d_itr, ++begin) {
        const value_type current = *begin;
        if (*d_itr == step && !ranges.empty())
            ranges.back().last = current;
        else
            ranges.push_back(Range(current, current));
    }
    return ranges;
}

typedef boost::container::flat_set<value_type> Set;

struct LocationData {
    LocationData(const LocationData &other) {
        allValues = other.allValues;
        location = other.location;
        sortedValues = other.sortedValues;
    }
    LocationData() {
        static size_t preAllocate = 64 * 1024;
        allValues.reserve(preAllocate);
    }
    inline void insert(value_type value) {
        allValues.push_back(value);
    }
    inline void reserve(size_t count) {
        allValues.reserve(count);
        sortedValues.reserve(count);
    }
    inline void computeDistinctValues() {
        sortedValues.insert(allValues.begin(), allValues.end());
    }
    inline bool dismiss(value_type &value) const {
        value = *sortedValues.begin();
        return sortedValues.size() == 1;
    }
    LocationData& operator=(const LocationData &other) {
        if (this != &other) {
            allValues = other.allValues;
            location = other.location;
            sortedValues = other.sortedValues;
        }
        return *this;
    }
    Location location;
    Values allValues;
    Set sortedValues;
    static bool less(const LocationData &a, const LocationData &b) {
        return a.sortedValues.size() < b.sortedValues.size();
    }
};

typedef std::vector<LocationData> LocationDatas;

static inline void overwrite(unsigned int value, std::string &inString, const Location &atLocation) {
    sequence::details::CharStack<> stack(value);
    while (stack.size() < atLocation.count)
        stack.push('0');
    char* ptr = &inString[atLocation.first];
    for (; !stack.empty(); stack.pop(), ++ptr)
        *ptr = stack.top();
}

struct Pattern {
    Pattern(const std::string &key, const Locations& locations) :
                    key(key), locationData(locations.size()) {
        for (size_t i = 0; i < locations.size(); ++i)
            locationData[i].location = locations[i];
        allValues.reserve(locations.size() * 8 * 1024);
    }

    inline void insert(const Values &values) {
        assert(values.size()==locationData.size());
        allValues.insert(allValues.end(), values.begin(), values.end());
    }

    void prepare() {
        const size_t elements = allValues.size() / locationData.size();
        const LocationDatas::iterator begin = locationData.begin();
        const LocationDatas::iterator end = locationData.end();
        std::for_each(begin, end, boost::bind(&LocationData::reserve, _1, elements));
        LocationDatas::iterator itr = begin;
        for (Values::const_iterator vItr = allValues.begin(), vEnd = allValues.end(); vItr != vEnd; ++vItr, ++itr) {
            if (itr == end)
                itr = begin;
            itr->allValues.push_back(*vItr);
        }
        std::for_each(begin, end, boost::bind(&LocationData::computeDistinctValues, _1));
        allValues.clear();
    }

    void bakeConstantLocations() {
        LocationDatas newLocations;
        newLocations.reserve(locationData.size());
        for (LocationDatas::const_iterator itr = locationData.begin(), end = locationData.end(); itr != end; ++itr) {
            const LocationData &current = *itr;
            value_type value = 0;
            if (current.dismiss(value))
                overwrite(value, key, current.location);
            else
                newLocations.push_back(current);
        }
        locationData.swap(newLocations);
    }

    std::string key;
    Values allValues;
    LocationDatas locationData;
};

typedef boost::unordered::unordered_map<std::string, Pattern> PatternsPerDir;

typedef boost::unordered::unordered_map<std::string, PatternsPerDir> AllPatterns;

struct TmpData {
    Values values;
    Locations locations;
};

// filling structures
void insert(TmpData&, PatternsPerDir &, std::string filename);
// filling structures
void insertPath(TmpData&, AllPatterns &, const std::string& absolutePath);

struct Splitter {
    template<typename Pair>
    struct second_t {
        typename Pair::second_type operator()(const Pair& p) const {
            return p.second;
        }
    };

    template<typename Map>
    second_t<typename Map::value_type> second(const Map& m) {
        return second_t<typename Map::value_type>();
    }

    Splitter(const Pattern& pattern) :
                    pattern(pattern), //
                    locations(pattern.locationData), //
                    begin(locations.begin()), //
                    end(locations.end()), //
                    pivot(std::min_element(begin, end, &LocationData::less)) {
        assert(locations.size()>1);

        // getting pointers to all other columns
        pColumns.reserve(locations.size());
        for (LocationDatas::const_iterator itr = begin; itr != end; ++itr)
            if (itr != pivot)
                pColumns.push_back(itr);

        const Values &dispatcher = pivot->allValues;
        size_t i = 0;
        Values tmp;
        for (Values::const_iterator pValue = dispatcher.begin(), pValueEnd = dispatcher.end(); pValue != pValueEnd; ++pValue, ++i) {
            tmp.clear();
            for (LocationsPtr::const_iterator itr = pColumns.begin(), end = pColumns.end(); itr != end; ++itr)
                tmp.push_back((*itr)->allValues[i]);
            getPattern(*pValue).insert(tmp);
        }
        std::transform(map.begin(), map.end(), std::back_inserter(patterns), second_t<Map::value_type>());
    }

    Pattern& getPattern(value_type value) {
        Map::iterator pFound = map.find(value);
        if (pFound == map.end()) {
            std::string key = pattern.key;
            overwrite(value, key, pivot->location);
            Locations newLocations;
            for (LocationsPtr::const_iterator itr = pColumns.begin(), end = pColumns.end(); itr != end; ++itr)
                newLocations.push_back((*itr)->location);
            Pattern newPattern(key, newLocations);
            pFound = map.insert(std::make_pair(value, newPattern)).first;
        }
        return pFound->second;
    }

    typedef LocationDatas::const_iterator LocationPtr;
    typedef std::vector<LocationPtr> LocationsPtr;
    typedef std::map<size_t, Pattern> Map;

    const Pattern& pattern;
    const LocationDatas &locations;
    const LocationPtr begin, end, pivot;
    LocationsPtr pColumns;
    Map map;
    std::vector<Pattern> patterns;
};

struct Parser {
    inline void insert(const std::string& absolutePath) {
        insertPath(tmp, allPatterns, absolutePath);
    }
    std::vector<sequence::BrowseItem> getResults() {
        if (!results.empty())
            return results;
        std::for_each(allPatterns.begin(), allPatterns.end(), boost::bind(&Parser::preparePath, this, _1));
        return results;
    }
    struct Functor {
        Functor(Parser *pParser) :
                        pParser(pParser) {
        }
        inline void operator()(const std::string& absolutePath) {
            pParser->insert(absolutePath);
        }
        Parser *pParser;
    };
    Functor functor() {
        return Functor(this);
    }
private:
    void addPattern(const std::string& path, const Pattern& pattern) {
        const LocationDatas &locations = pattern.locationData;
        if (locations.empty()) {
            results.push_back(create_file(boost::filesystem::path(path) / pattern.key));
            return;
        }
        assert(locations.size()==1);
        const LocationData &location = locations[0];
        const Set &set = location.sortedValues;
        value_type step = 0;
        const Ranges ranges = getRangesAndStep(set.begin(), set.end(), step);
        std::transform(ranges.begin(), //
                       ranges.end(), //
                       std::back_inserter(results), //
                       boost::bind(&Parser::createItem, this, boost::ref(path), boost::ref(pattern), _1, step));
    }
    BrowseItem createItem(const std::string &path, const Pattern& pattern, const Range range, const size_t step) {
        return create_sequence(path, parsePattern(pattern.key), range, step);
    }
    void preparePath(AllPatterns::value_type &pair) {
        std::vector<Pattern> ready;
        PatternsPerDir &patterns = pair.second;
        std::for_each(patterns.begin(), patterns.end(), boost::bind(&Parser::preparePattern, boost::ref(ready), _1));
        const std::string &path = pair.first;
        std::for_each(ready.begin(), ready.end(), boost::bind(&Parser::addPattern, this, boost::ref(path), _1));
    }
    static void preparePattern(std::vector<Pattern> &ready, PatternsPerDir::value_type &pair) {
        mutate(ready, pair.second);
    }
    static void mutate(std::vector<Pattern>& ready, Pattern& pattern) {
        pattern.prepare();
        pattern.bakeConstantLocations();
        if (pattern.locationData.size() < 2)
            ready.push_back(pattern);
        else {
            Splitter splitter(pattern);
            std::for_each(splitter.patterns.begin(), splitter.patterns.end(), boost::bind(&Parser::mutate, boost::ref(ready), _1));
        }
    }
    TmpData tmp;
    AllPatterns allPatterns;
    std::vector<sequence::BrowseItem> results;
};

} /* namespace details */
} /* namespace parser */
} /* namespace sequence */
#endif /* UTILS_H_ */
