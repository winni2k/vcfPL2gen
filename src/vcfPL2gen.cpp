#include <algorithm>
#include <string>
#include <vector>
#include <iostream>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/split.hpp>

#include <boost/spirit/include/qi.hpp>
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
      vector<string> sampFormat;
      boost::split(sampFormat, cols[colNum], boost::is_any_of(":"));
      vector<string> PLs;
      boost::split(PLs, sampFormat[PLColNum], boost::is_any_of(","));

      assert(PLs.size() == 3);
      vector<double> GLs;
      GLs.reserve(3);
      for (auto pl : PLs)
        GLs.push_back(pow(10, -stod(pl) / 10));

      for (auto gl : GLs) {
        assert(gl <= 1);
        assert(gl >= 0);
      }

      // now normalize PLs and print in gen format
      double sum = GLs[0] + GLs[1] + GLs[2];
      if (GLs[0] != 1 || GLs[1] != 1 || GLs[2] != 1)
        for (auto &gl : GLs)
          gl /= sum;
      else
        for (auto &gl : GLs)
          gl = 0;

      for (auto gl : GLs)
        cout << " " << gl;
    }
    cout << "\n";
    ++outLines;
  }

  return 0;
}
