#ifndef UTIL_MUTABLE_VOCAB_H
#define UTIL_MUTABLE_VOCAB_H

/* A vocabulary mapping class that's mutable at runtime.  The kenlm code has
 * a specialized immutable vocabulary.
 */

#include "util/murmur_hash.hh"
#include "util/pool.hh"
#include "util/probing_hash_table.hh"
#include "util/string_piece.hh"

#include <stdint.h>

namespace util {

inline uint64_t HashForVocab(const char *str, std::size_t len) {
  return util::MurmurHash64A(str, len, 0);
}
inline uint64_t HashForVocab(const StringPiece &str) {
  return HashForVocab(str.data(), str.length());
}

#pragma pack(push)
#pragma pack(4)
struct MutableVocabInternal {
  typedef uint64_t Key;
  uint64_t GetKey() const { return key; }
  void SetKey(uint64_t to) { key = to; }

  uint64_t key;
  uint32_t id;
};
#pragma pack(pop)
 
class MutableVocab {
  public:
    typedef uint32_t ID;

    static const ID kUNK = 0;

    MutableVocab();
    
    uint32_t Find(const StringPiece &str) const;

    ID FindOrInsert(const StringPiece &str);

    StringPiece String(ID id) const {
      return strings_[id];
    }

    // Includes kUNK.
    std::size_t Size() const { return strings_.size(); }
    
  private:
    util::Pool piece_backing_;

    typedef util::AutoProbing<MutableVocabInternal, util::IdentityHash> Map;
    Map map_;

    std::vector<StringPiece> strings_;
};

#pragma pack(push)
#pragma pack(4)
struct ProbingVocabularyEntry {
  typedef uint32_t WordIndex;
  uint64_t key;
  WordIndex value;

  typedef uint64_t Key;
  uint64_t GetKey() const { return key; }
  void SetKey(uint64_t to) { key = to; }

  static ProbingVocabularyEntry Make(uint64_t key, WordIndex value) {
    ProbingVocabularyEntry ret;
    ret.key = key;
    ret.value = value;
    return ret;
  }
};
#pragma pack(pop)

class NoOpUniqueWords {
  public:
    NoOpUniqueWords() {}
    void operator()(const StringPiece &word) {}
};

template <class NewWordAction = NoOpUniqueWords> class GrowableVocab {
  public:
    typedef uint32_t WordIndex;

    static std::size_t MemUsage(WordIndex content) {
      return Lookup::MemUsage(content > 2 ? content : 2);
    }

    template <class NewWordConstruct> GrowableVocab(WordIndex initial_size, NewWordConstruct &new_word_construct = NewWordAction())
      : lookup_(initial_size), new_word_(new_word_construct) {
      FindOrInsert("<unk>"); // Force 0
      FindOrInsert("<s>"); // Force 1
      FindOrInsert("</s>"); // Force 2
    }

    WordIndex Index(const StringPiece &str) const {
      Lookup::ConstIterator i;
      return lookup_.Find(HashForVocab(str), i) ? i->value : 0;
    }

    WordIndex FindOrInsert(const StringPiece &word) {
      ProbingVocabularyEntry entry = ProbingVocabularyEntry::Make(util::MurmurHashNative(word.data(), word.size()), Size());
      Lookup::MutableIterator it;
      if (!lookup_.FindOrInsert(entry, it)) {
        new_word_(word);
        UTIL_THROW_IF2(Size() >= std::numeric_limits<WordIndex>::max(), "Too many vocabulary words.  Change WordIndex to uint64_t");
      }
      return it->value;
    }

    WordIndex Size() const { return lookup_.Size(); }

    NewWordAction &Action() { return new_word_; }
    const NewWordAction &Action() const { return new_word_; }

  private:
    typedef util::AutoProbing<ProbingVocabularyEntry, util::IdentityHash> Lookup;

    Lookup lookup_;

    NewWordAction new_word_;
};

} // namespace util
#endif // UTIL_MUTABLE_VOCAB_H
