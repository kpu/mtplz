#include "decode/chart.hh"
#include "decode/context.hh"
#include "decode/output.hh"
#include "decode/phrase_table.hh"
#include "decode/stacks.hh"
#include "util/fake_ofstream.hh"

#include <boost/program_options.hpp>

#include <string>
#include <vector>

namespace decode {
void Decode(Context &context, const PhraseTable &table, const StringPiece in, util::FakeOFStream &out) {
  Chart chart(table, in, context.GetVocab(), context.GetScorer());
  Stacks stacks(context, chart);
  const Hypothesis *hyp = stacks.End();
  if (!hyp) {
    out << '\n';
  } else {
    Output(*hyp, context.GetVocab(), out);
    out << '\n';
  }
}
} // namespace decode

int main(int argc, char *argv[]) {
  try {
    namespace po = boost::program_options;
    po::options_description options("Decoder options");
    std::string lm, phrase;
    std::string weights;
    decode::Config config;
    options.add_options()
      ("lm,l", po::value<std::string>(&lm)->required(), "Language model file")
      ("phrase,p", po::value<std::string>(&phrase)->required(), "Phrase table")
      ("weights,W", po::value<std::string>(&weights), "Weights")
      ("beam,K", po::value<unsigned int>(&config.pop_limit)->required(), "Beam size")
      ("reordering,R", po::value<std::size_t>(&config.reordering_limit)->required(), "Reordering limit");
    if (argc == 1) {
      std::cerr << options << std::endl;
      return 1;
    }
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, options), vm);
    po::notify(vm);

    decode::Context context(lm.c_str(), weights, config);
    decode::PhraseTable table(phrase.c_str(), context.GetVocab(), context.GetScorer());
    util::FilePiece f(0);
    util::FakeOFStream out(1);
    while (true) {
      StringPiece line;
      try {
        line = f.ReadLine();
      } catch (const util::EndOfFileException &e) { break; }
      decode::Decode(context, table, line, out);
    }
  } catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }
  return 0;
}
