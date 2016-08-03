#include "decode/system.hh"

namespace decode {
  
System::System(const Config config, const pt::Access &phrase_access,
    const Weights &weights, const lm::ngram::Model &lm)
  : config_(config), weights_(weights),
  objective_(phrase_access, lm.BeginSentenceState()),
  search_context_(search::Config(
        weights.LMWeight(),
        config.pop_limit,
        search::NBestConfig(1)), lm) {}

void System::LoadWeights() {
  objective_.LoadWeights(weights_);
}

void System::LoadVocab(pt::VocabRange vocab_range, std::size_t vocab_size) {
  base_vocab_.map = std::vector<VocabWord*>(vocab_size);
  auto word = vocab_range.begin();
  InsertNewWord(0); // <UNK> is not found in vocab, insert manually
  for (++word; word != vocab_range.end(); ++word) {
    InsertNewWord(base_vocab_.vocab.FindOrInsert(*word));
  }
}

void System::InsertNewWord(const ID id) {
  util::Layout &word_layout = objective_.GetFeatureInit().word_layout;
  VocabWord *mapping = reinterpret_cast<VocabWord*>(word_layout.Allocate(base_vocab_.pool));
  objective_.GetFeatureInit().pt_id_field(mapping) = id;
  objective_.NewWord(base_vocab_.vocab.String(id), mapping);
  base_vocab_.map[id] = mapping;
}

} // namespace decode
