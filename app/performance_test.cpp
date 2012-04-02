/*
 * main.cpp
 *
 *  Created on: Feb 20, 2012
 *      Author: Guillaume Chatelet
 */

#include <sequence/parser/details/ParserUtils.h>
#include <sequence/DisplayUtils.h>

#include <boost/chrono/chrono.hpp>
#include <boost/chrono/chrono_io.hpp>

#include <vector>
#include <functional>
#include <algorithm>
#include <iterator>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <cstdio>

using namespace std;
using namespace sequence;
using namespace sequence::parser::details;
using namespace boost::chrono;
using namespace boost::filesystem;

vector<string> preparePaths(const string path, const vector<SequencePattern> &patterns, const Range range) {
    vector<string> paths;
    for (size_t i = 0; i < patterns.size(); ++i)
        for (size_t f = range.first; f <= range.last; ++f)
            paths.push_back(path + sequence::instanciatePattern(patterns[i], f));
    std::random_shuffle(paths.begin(), paths.end());
    return paths;
}

void test(const vector<string> &paths) {
    const high_resolution_clock::time_point start = high_resolution_clock::now();

    typedef vector<sequence::BrowseItem> Items;
    Parser parser;

    for_each(paths.begin(), paths.end(), parser.functor());
    const high_resolution_clock::time_point mid = high_resolution_clock::now();

    Items items = parser.getResults();
    const high_resolution_clock::time_point end = high_resolution_clock::now();

    ostringstream stream;
    stream << "Listing " << items.size() << " items took " << endl;
    stream << " - inserting : " << duration_cast<milliseconds>(mid - start) << endl;
    stream << " - results   : " << duration_cast<milliseconds>(end - mid) << endl;
    copy(items.begin(), items.end(), ostream_iterator<sequence::BrowseItem>(stream, "\n"));
    printf("%s\n", stream.str().c_str());
}

int main(int argc, char **argv) {
    try {
        vector<SequencePattern> patterns;
        for (int i = 1; i < 4; ++i) {
            ostringstream ss;
            ss << "LGT-prepaanimatic--shot01-v00" << i;
            patterns.push_back(parsePattern(ss.str() + ".####.exr"));
            patterns.push_back(parsePattern(ss.str() + "_directDiffuse.####.exr"));
            patterns.push_back(parsePattern(ss.str() + "_indirectDiffuse.####.exr"));
            patterns.push_back(parsePattern(ss.str() + "_z.####.exr"));
        }
        test(preparePaths("/s/prods/le_terrier/prepa/animatic/images/3d/wip/LGT-prepaanimatic-shot01/", patterns, Range(1, 400)));
        patterns.clear();
        patterns.push_back(parsePattern("file-0001.bad.#######.exr"));
        test(preparePaths("/s/", patterns, Range(0, 200000)));
        return EXIT_SUCCESS;
    } catch (exception& e) {
        cerr << "Unexpected error : " << e.what() << endl;
    }
    return EXIT_FAILURE;
}
