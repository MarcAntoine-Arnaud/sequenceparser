/*
 * ConsecutiveRanges.h
 *
 *  Created on: 2 avr. 2012
 *      Author: Guillaume Chatelet
 */

#ifndef CONSECUTIVERANGES_H_
#define CONSECUTIVERANGES_H_

#include <sequence/Config.h>
#include <vector>
#include <set>
namespace sequence { struct Range; }

namespace sequence {
namespace parser {

struct SEQUENCEPARSER_API ConsecutiveRanges : public std::set<unsigned int> {
    std::vector<Range> getConsecutiveRanges() const;
private:
    ConsecutiveRanges(const ConsecutiveRanges&);
    ConsecutiveRanges& operator=(const ConsecutiveRanges&);
};

} /* namespace parser */
} /* namespace sequence */
#endif /* CONSECUTIVERANGES_H_ */
