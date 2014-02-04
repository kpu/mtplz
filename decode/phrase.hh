#ifndef DECODE_PHRASE__
#define DECODE_PHRASE__

#include "decode/id.hh"
#include "util/string_piece.hh"

namespace util { class Pool; class MutableVocab; }

namespace decode {

// First ID* is the length of the array.
// Rest is array of words.
class Phrase {
  public:
    explicit Phrase(util::Pool &pool, util::MutableVocab &vocab, const StringPiece &tokens);

    // For passthroughs.
    explicit Phrase(util::Pool &pool, ID word);

    // Use to reconstruct from a void* pointer.
    explicit Phrase(const void *already_initialized)
      : base_(reinterpret_cast<const ID*>(already_initialized)) {}
    
    ID size() const { return *base_; }

    typedef const ID *const_iterator;
    const ID *begin() const { return base_ + 1; }
    const ID *end() const { return begin() + size(); }

    const void *Base() const { return base_; }

    bool Valid() const { return base_ != NULL; }

  private:
    // Points to length.
    const ID *base_;
};

} // namespace decode

#endif // DECODE_PHRASE__
