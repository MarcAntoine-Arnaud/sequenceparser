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
    Location location;
    Values allValues;
    Set sortedValues;
};

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

    void generateLocationData() {
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
                overwrite(value, current.location);
            else
                newLocations.push_back(current);
        }
        locationData.swap(newLocations);
    }

    std::string key;
    Values allValues;
    typedef std::vector<LocationData> LocationDatas;
    LocationDatas locationData;
private:
    void overwrite(value_type value, const Location &atLocation) {
        sequence::details::CharStack<> stack(value);
        while (stack.size() < atLocation.count)
            stack.push('0');
        char* ptr = &key[atLocation.first];
        for (; !stack.empty(); stack.pop(), ++ptr)
            *ptr = stack.top();
    }
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

struct Parser {
    inline void insert(const std::string& absolutePath) {
        insertPath(tmp, allPatterns, absolutePath);
    }
    void results() {
        std::for_each(allPatterns.begin(), allPatterns.end(), &Parser::preparePath);
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
    static void preparePath(AllPatterns::value_type &pair) {
        std::vector<Pattern> ready;
        std::for_each(pair.second.begin(), pair.second.end(), boost::bind(&Parser::preparePattern, boost::ref(ready), _1));
    }
    static void preparePattern(std::vector<Pattern> &ready, PatternsPerDir::value_type &pair) {
        pair.second.generateLocationData();
        mutate(ready, pair.second);
    }
    static void mutate(std::vector<Pattern>& ready, Pattern& pattern) {
        pattern.bakeConstantLocations();
    }

    TmpData tmp;
    AllPatterns allPatterns;
};

} /* namespace details */
} /* namespace parser */
} /* namespace sequence */
#endif /* UTILS_H_ */
