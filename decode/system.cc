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

void System::LoadVocab(pt::VocabRange vocab_range) {
  util::Layout &word_layout = objective_.GetFeatureInit().word_layout;
  assert(vocab_.String(0) == *vocab_range.begin());
  for (auto word = vocab_range.begin(); word != vocab_range.end(); ++word) {
    VocabWord *mapping = reinterpret_cast<VocabWord*>(word_layout.Allocate(word_pool_));
    std::size_t i = vocab_.FindOrInsert(*word);
    objective_.GetFeatureInit().pt_id_field(mapping) = i;
    objective_.NewWord(vocab_.String(i), mapping);
    if (i >= vocab_mapping_.size()) {
      vocab_mapping_.resize(i+1);
    }
    vocab_mapping_[i] = mapping;
  }
}

} // namespace decode
