#ifndef DISPLAYUTILS_H_
#define DISPLAYUTILS_H_

#include "BrowseItem.h"

#include <ostream>

namespace sequence
{

struct Range;
struct SequencePattern;
struct Sequence;

SEQUENCEPARSER_API const char* toString(const BrowseItemType type);

SEQUENCEPARSER_API std::ostream& operator<<(std::ostream &stream, const Range &range);
SEQUENCEPARSER_API std::ostream& operator<<(std::ostream &stream, const SequencePattern &pattern);
SEQUENCEPARSER_API std::ostream& operator<<(std::ostream &stream, const Sequence &sequence);
SEQUENCEPARSER_API std::ostream& operator<<(std::ostream &stream, const BrowseItemType &type);
SEQUENCEPARSER_API std::ostream& operator<<(std::ostream &stream, const BrowseItem &item);

}
#endif
