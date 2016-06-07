#include "decode/system.hh"
#include "decode/chart.hh"
#include "decode/context.hh"
#include "decode/output.hh"
#include "decode/phrase_table.hh"
#include "decode/stacks.hh"
#include "util/file_stream.hh"
#include "util/usage.hh"

// features
#include "decode/distortion.hh"
#include "decode/lm.hh"

#include <boost/program_options.hpp>

#include <string>
#include <vector>

namespace decode {
void Decode(Context &context, const PhraseTable &table, const StringPiece in, ScoreHistoryMap &history_map, bool verbose, util::FileStream &out) {
  Chart chart(table, in, context.GetVocab(), context.GetScorer());
  Stacks stacks(context, chart);
  const Hypothesis *hyp = stacks.End();
	
	history_map.clear();
	
  if (hyp) {
    if(verbose) {
      OutputVerbose(*hyp, context.GetVocab(), history_map, out);
    } else {
	  	Output(*hyp, context.GetVocab(), out);
	  }
  }
  out << '\n';
}
} // namespace decode

int main(int argc, char *argv[]) {
  try {
    namespace po = boost::program_options;
    po::options_description options("Decoder options");
    std::string lm_file, phrase_file;
    std::string weights_file;
    decode::Config config;
    bool verbose = false;

    options.add_options()
      ("verbose,v", "Produce verbose output")
      ("lm,l", po::value<std::string>(&lm_file)->required(), "Language model file")
      ("phrase,p", po::value<std::string>(&phrase_file)->required(), "Phrase table")
      ("weights_file,W", po::value<std::string>(&weights_file)->required(), "Weights file")
      ("beam,K", po::value<unsigned int>(&config.pop_limit)->required(), "Beam size")
      ("reordering,R", po::value<std::size_t>(&config.reordering_limit)->required(), "Reordering limit");
    if (argc == 1) {
      std::cerr << options << std::endl;
      return 1;
    }
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, options), vm);
    po::notify(vm);

    if(vm.count("verbose")) {
        verbose = true;
    }
    decode::System sys(config, weights_file);
    decode::Context context(lm_file.c_str(), weights_file, sys.GetConfig(), sys.GetObjective());
    decode::Distortion distortion;
    sys.GetObjective().AddFeature(distortion);
    decode::LM lm(phrase_file.c_str(), context.GetVocab());
    sys.GetObjective().AddFeature(lm);
    decode::PhraseTable table(phrase_file.c_str(), context.GetVocab(), context.GetScorer());
    sys.GetObjective().lm_begin_sentence_state = &context.GetScorer().LanguageModel().BeginSentenceState();
    util::FilePiece f(0, NULL, &std::cerr);
    util::FileStream out(1);
    decode::ScoreHistoryMap map;
    while (true) {
      StringPiece line;
      try {
        line = f.ReadLine();
      } catch (const util::EndOfFileException &e) { break; }
      decode::Decode(context, table, line, map, verbose, out);
      out.flush();
      f.UpdateProgress();
    }
    util::PrintUsage(std::cerr);
  } catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }
  return 0;
}
