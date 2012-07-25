#include <boost/python.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>

#include <sequence/parser/Browser.h>
#include <sequence/BrowseItem.h>

#include <vector>

using namespace boost::python;
using namespace std;
using namespace sequence;
using namespace sequence::parser;

string pathAsString(const BrowseItem& item) {
    return item.path.string();
}

namespace sequence {
namespace parser {

bool operator==(const BrowseItem &lhs, const BrowseItem &rhs) {
    return true;
}

}  // namespace parser
}  // namespace sequence


BOOST_PYTHON_MODULE(sequenceparser) {
    class_<Range>("Range")
        .def_readonly("first", &Range::first)
        .def_readonly("last", &Range::last)
        ;

    class_<SequencePattern>("SequencePattern")
        .def_readonly("prefix", &SequencePattern::prefix)
        .def_readonly("suffix", &SequencePattern::suffix)
        .def_readonly("padding", &SequencePattern::padding)
        ;

    class_<Sequence>("Sequence")
        .def_readonly("pattern", &Sequence::pattern)
        .def_readonly("range", &Sequence::range)
        .def_readonly("step", &Sequence::step)
        ;

    enum_<BrowseItemType>("BrowseItemType")
            .value("UNDEFINED", UNDEFINED)
            .value("FOLDER", FOLDER)
            .value("SEQUENCE", SEQUENCE)
            .value("UNITFILE", UNITFILE)
            ;

    class_<BrowseItem>("BrowseItem")
        .def_readonly("type", &BrowseItem::type)
        .add_property("path", pathAsString)
        .def_readonly("sequence", &BrowseItem::sequence)
        ;

    class_<BrowseItems>("BrowseItems")
        .def(vector_indexing_suite<BrowseItems>());

    def("browse", browse);
}
