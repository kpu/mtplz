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

void System::LoadVocab(const util::MutableVocab &vocab) {
  vocab_mapping_ = std::vector<VocabWord*>(vocab.Size());
  util::Layout &word_layout = objective_.GetFeatureInit().word_layout;
  for (ID i=0; i < vocab.Size(); ++i) {
    VocabWord *mapping = reinterpret_cast<VocabWord*>(word_layout.Allocate(word_pool_));
    objective_.GetFeatureInit().pt_id_field(mapping) = i;
    objective_.NewWord(vocab.String(i), mapping);
    vocab_mapping_[i] = mapping;
  }
}

} // namespace decode
