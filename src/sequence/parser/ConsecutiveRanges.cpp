/*
 * ConsecutiveRanges.cpp
 *
 *  Created on: 2 avr. 2012
 *      Author: Guillaume Chatelet
 */

#include "ConsecutiveRanges.h"
#include <sequence/Range.h>

#include <algorithm>
#include <numeric>

using namespace std;

namespace sequence {
namespace parser {

vector<Range> ConsecutiveRanges::getConsecutiveRanges() const {
    if (empty())
        return vector<Range>();
    vector<Range> ranges;
    if (size() == 1) {
        ranges.push_back(Range(*begin(), *begin()));
    } else {
        typedef vector<value_type> V;
        typedef V::const_iterator VItr;
        V derivative(size());
        adjacent_difference(begin(), end(), derivative.begin());
        const_iterator itr = begin();
        for (VItr d_itr = derivative.begin(), end = derivative.end(); d_itr != end; ++d_itr, ++itr) {
            if (*d_itr == 1 && !ranges.empty())
                ranges.back().last = *itr;
            else
                ranges.push_back(Range(*itr, *itr));
        }
    }
    return ranges;
}

} /* namespace parser */
} /* namespace sequence */
