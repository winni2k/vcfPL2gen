#include <algorithm>
#include <string>
#include <vector>
#include <iostream>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/split.hpp>
#include <htslib/hts.h>
#include <htslib/vcf.h>
#include <exception>

using namespace std;

static_assert(__cplusplus == 201103L, "C++11 compiler required");

int main(int ac, char *av[]) {

  if (ac != 2)
    throw std::runtime_error(
        "Please specify exactly one input file on command line");

  // expect vcf/bcf formatted file as input
  string inFile(av[1]);
  htsFile *fp = hts_open(av[1], "r");
  bcf_hdr_t *hdr = bcf_hdr_read(fp);
  bcf1_t *rec = bcf_init1();
  unsigned outLines(0);
  cout << std::fixed;
  cout.precision(5);
  while (bcf_read1(fp, hdr, rec) >= 0) {

    if ((outLines & (1024 - 1)) == 0)
      cerr << "\rNumber of Lines output: " << outLines;

    // get the first five and sample columns
    bcf_unpack(rec, BCF_UN_STR | BCF_UN_IND);

    // make sure the PL field exists in format
    assert(bcf_get_fmt(hdr, rec, "PL"));

    // print out position information
    // CHROM POS POS REF ALT
    cout << bcf_hdr_id2name(hdr, rec->rid) << " " << (rec->pos + 1) << " "
         << (rec->pos + 1) << " " << rec->d.allele[0] << " "
         << rec->d.allele[1];

    int npl_arr = 0, mpl_arr = 0;
    int32_t *pl_arr = NULL;
    npl_arr = bcf_get_format_int32(hdr, rec, "PL", &pl_arr, &mpl_arr);
    if (npl_arr > 0) {

      assert(npl_arr / 3 == bcf_hdr_nsamples(hdr));
      int nvals = npl_arr / bcf_hdr_nsamples(hdr);
      int32_t *ptr = pl_arr;
      assert(nvals == 3);
      for (int i = 0; i < bcf_hdr_nsamples(hdr); ++i) {

        vector<double> PLs(3);
        for (int j = 0; j < nvals; ++j) {
          if (ptr[j] == bcf_int32_vector_end)
            throw std::runtime_error("unexpected end of PLs");
          if (ptr[j] == bcf_int32_missing)
            throw std::runtime_error("missing PL value encountered");
          PLs[j] = pow(10, -static_cast<double>(ptr[j]) / 10);
        }
        ptr += nvals;

        for (auto gl : PLs) {
          assert(gl <= 1);
          assert(gl >= 0);
        }

        // now normalize PLs and print in gen format
        double sum = PLs[0];
        sum += PLs[1];
        sum += PLs[2];
        for (auto &gl : PLs)
          cout << " " << (gl / sum);
      }
      cout << "\n";
      ++outLines;

    } else {
      free(pl_arr);
      throw std::runtime_error("could not read record");
    }
    free(pl_arr);
  }

  return 0;
}
