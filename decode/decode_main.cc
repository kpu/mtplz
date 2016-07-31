#include "decode/system.hh"
#include "decode/chart.hh"
#include "decode/output.hh"
#include "decode/stacks.hh"
#include "decode/weights.hh"
#include "pt/query.hh"
#include "pt/statistics.hh"
#include "pt/access.hh"
#include "pt/create.hh"
#include "util/file_stream.hh"
#include "util/mutable_vocab.hh"
#include "util/usage.hh"

// features
#include "decode/distortion.hh"
#include "decode/word_insert.hh"
#include "decode/passthrough.hh"
#include "decode/lm.hh"
#include "decode/lexro.hh"

#include <boost/program_options.hpp>

#include <string>
#include <vector>

namespace decode {
void Decode(System &system, const pt::Table &table, const StringPiece in,
    ScoreHistoryMap &history_map, bool verbose, util::FileStream &out) {
  VocabMap vocab_map(system.GetObjective(), system.GetBaseVocab());
  Chart chart(table.Stats().max_source_phrase_length, vocab_map, system.GetObjective());
  chart.ReadSentence(in);
  chart.LoadPhrases(table);
  Stacks stacks(system, chart);
  const Hypothesis *hyp = stacks.End();
	
  history_map.clear();
	
  if (hyp) {
    Output(*hyp, vocab_map, history_map, out, system.GetObjective().GetFeatureInit(), verbose);
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

    pt::Table table(phrase_file.c_str(), util::READ);

    decode::Weights weights;
    weights.ReadFromFile(weights_file);
    decode::Distortion distortion;
    decode::WordInsertion word_insert;
    decode::Passthrough passthrough;
    decode::LM lm(lm_file.c_str());
    decode::LexicalizedReordering lexro;

    decode::System sys(config, table.Accessor(), weights, lm.Model());
    sys.GetObjective().AddFeature(distortion);
    sys.GetObjective().AddFeature(word_insert);
    sys.GetObjective().AddFeature(passthrough);
    sys.GetObjective().AddFeature(lm);
    sys.GetObjective().RegisterLanguageModel(lm);
    sys.GetObjective().AddFeature(lexro);

    sys.LoadVocab(table.Vocab(), table.Stats().vocab_size);
    sys.GetObjective().LoadWeights(weights);

    util::FilePiece f(0, NULL, &std::cerr);
    util::FileStream out(1);
    decode::ScoreHistoryMap map;
    while (true) {
      StringPiece line;
      try {
        line = f.ReadLine();
      } catch (const util::EndOfFileException &e) { break; }
      decode::Decode(sys, table, line, map, verbose, out);
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
