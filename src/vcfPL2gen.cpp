#include <algorithm>
#include <string>
#include <vector>
#include <iostream>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>

namespace pls {
namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;
namespace phoenix = boost::phoenix;

///////////////////////////////////////////////////////////////////////////
//  Our number list compiler
///////////////////////////////////////////////////////////////////////////
//[tutorial_numlist2
template <typename Iterator>
bool parse_numbers(Iterator first, Iterator last, std::vector<double> &v) {
  using qi::double_;
  using qi::_1;
  using ascii::space;
  using phoenix::push_back;

  bool r = qi::parse(first, last,

                        //  Begin grammar
                        (double_[push_back(phoenix::ref(v), _1)] >>
                         *(',' >> double_[push_back(phoenix::ref(v), _1)]))
                        //  End grammar
                        );

  if (first != last) // fail if we did not get a full match
    return false;
  return r;
}
//]
}

using namespace std;

static_assert(__cplusplus == 201103L, "C++11 compiler required");

int main() { // int ac, char **av

  // expect vcf formatted file as input
  string input;

  // pass on header
  while (getline(cin, input)) {
    if (input[0] == '#' && input[1] == 'C')
      break;
  }

  // count number of columns in chrom line
  size_t numCols = std::count(input.begin(), input.end(), '\t') + 1;
  if (numCols < 7) {
    cerr << "Too few columns in VCF CHROM line (less than 7 columns)" << endl;
    cerr << "Malformed line: " << input << endl;
    exit(1);
  }

  // parse lines
  unsigned outLines(0);
  bool PLColNumDefined = false;
  size_t PLColNum = 0; // needs to be made >= 0 later
  while (getline(cin, input)) {
    if (outLines % 1000 == 0)
      cerr << "\rNumber of Lines output: " << outLines;

    assert(input[0] != '#');

    // otherwise parse the line and do some splitting!

    // data to hold results of parsing
    // parsedFirstFewCols will hold columns 1-9, but excluding 2, which will be
    // in genomic_pos

    // #CHROM  POS     ID      REF     ALT     QUAL    FILTER  INFO    FORMAT
    // [SAMPLE1 .. SAMPLEN]
    vector<string> cols;
    boost::split(cols, input, boost::is_any_of("\t"));
    // figure out which column the PL format tag is in
    if (!PLColNumDefined) {
      vector<string> format;
      boost::split(format, cols[8], boost::is_any_of(":"));
      for (unsigned colNum = 0; colNum != format.size(); ++colNum)
        if (format[colNum] == "PL") {
          PLColNum = colNum;
          PLColNumDefined = true;
          break;
        }
    }

    // print out position information
    cout << cols[0] << " " << cols[1] << " " << cols[1] << " " << cols[3] << " "
         << cols[4];

    for (size_t colNum = 9; colNum < cols.size(); ++colNum) {

      // find index of first char of the PL format
      size_t sepCharIdx = 0;
      size_t firstPLCharIdx = 0;
      for (size_t formatField = 0; formatField < PLColNum; ++formatField) {
        sepCharIdx = cols[colNum].find_first_of(":", firstPLCharIdx);
        firstPLCharIdx = sepCharIdx + 1;
      }

      // find iter to one past last char
      sepCharIdx = cols[colNum].find_first_of(":\t\n", firstPLCharIdx);
      auto sepCharIter = cols[colNum].end();
      if(sepCharIdx != string::npos)
          sepCharIter = cols[colNum].begin() + sepCharIdx;
      
      vector<double> PLs;
      PLs.reserve(3);
      if (!pls::parse_numbers(cols[colNum].begin() + firstPLCharIdx,
                              sepCharIter, PLs))
        throw std::runtime_error(
            "Could not parse string: \"" +
            cols[colNum].substr(firstPLCharIdx, sepCharIdx - firstPLCharIdx) + '"');

      assert(PLs.size() == 3);
      for (auto &pl : PLs)
        pl = pow(10, -pl / 10);

      for (auto gl : PLs) {
        assert(gl <= 1);
        assert(gl >= 0);
      }

      // now normalize PLs and print in gen format
      double sum = PLs[0] + PLs[1] + PLs[2];
      if (PLs[0] != 1 || PLs[1] != 1 || PLs[2] != 1)
        for (auto &gl : PLs)
          gl /= sum;
      else
        for (auto &gl : PLs)
          gl = 0;

      for (auto gl : PLs)
        cout << " " << gl;
    }
    cout << "\n";
    ++outLines;
  }

  return 0;
}
