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
#include "decode/phrase_count_feature.hh"
#include "decode/pt_features.hh"
#include "decode/lm.hh"
#include "decode/lexro.hh"

#include <boost/program_options.hpp>

#include <string>
#include <vector>

namespace decode {
void Decode(System &system, const pt::Table &table, Chart::VertexCache &cache,
    const StringPiece in,
    ScoreHistoryMap &history_map, bool verbose, util::FileStream &out) {
  Chart chart(table.Stats().max_source_phrase_length, system.GetBaseVocab(), system.GetObjective(), cache);
  chart.ReadSentence(in);
  chart.LoadPhrases(table);
  Stacks stacks(system, chart);
  const Hypothesis *hyp = stacks.End();
	
  history_map.clear();
	
  if (hyp) {
    Output(*hyp, chart.VocabMapping(), history_map, out, system.GetObjective().GetFeatureInit(), verbose);
    std::cerr << "score: " << hyp->GetScore() << std::endl;
  }
  out << '\n';

  if (verbose && hyp) {
    std::vector<float> feature_values(system.GetObjective().weights.size());
    while (hyp->Previous() && hyp->Target()) {
      std::size_t i = 0;
      for (float v : system.GetObjective().GetFeatureValues(*hyp)) {
        feature_values[i++] += v;
      }
      hyp = hyp->Previous();
    }
    std::cerr << "feature values: [ \n";
    std::size_t i = 0;
    for (auto value : feature_values) {
      std::cerr << system.GetObjective().FeatureDescription(i++) << ": " << value << std::endl;
    }
    std::cerr << "]\n";
  }
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
    decode::Passthrough passthrough;
    decode::WordInsertion word_insert;
    decode::PhraseCountFeature phrase_count_feature;
    decode::PhraseTableFeatures pt_features;
    decode::LM lm(lm_file.c_str());
    decode::LexicalizedReordering lexro;

    decode::System sys(config, table.Accessor(), weights, lm.Model());
    sys.GetObjective().AddFeature(distortion);
    sys.GetObjective().AddFeature(passthrough);
    sys.GetObjective().AddFeature(word_insert);
    sys.GetObjective().AddFeature(phrase_count_feature);
    sys.GetObjective().AddFeature(pt_features);
    sys.GetObjective().AddFeature(lm);
    sys.GetObjective().RegisterLanguageModel(lm);
    sys.GetObjective().AddFeature(lexro);

    sys.LoadVocab(table.Vocab(), table.Stats().vocab_size);
    sys.GetObjective().SetStoreFeatureValues(verbose);
    sys.GetObjective().LoadWeights(weights);

    util::FilePiece f(0, NULL, &std::cerr);
    util::FileStream out(1);
    decode::Chart::VertexCache cache(15000000); // TODO non-hardcode
    // TODO vocab map originally exists to avoid having a global dictionary.
    // it is now here because we need backing for cache, which only exists
    // to make speed comparable to the previous mtplz
    decode::ScoreHistoryMap history_map;
    std::size_t i = 0;
    while (true) {
      StringPiece line;
      try {
        line = f.ReadLine();
      } catch (const util::EndOfFileException &e) { break; }
      util::PrintUsage(std::cerr);
      std::cerr << "sentence " << i++ << std::endl;
      decode::Decode(sys, table, cache, line, history_map, verbose, out);
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
