#ifndef SEARCH_HEADER__
#define SEARCH_HEADER__

// Header consisting of Score, Arity, and Note

#include "search/types.hh"

#include <stdint.h>
#include <iostream>

namespace search {

// Copying is shallow.  
class Header {
  public:
    bool Valid() const { return base_; }

    Score GetScore() const {
      return *reinterpret_cast<const float*>(base_);
    }
    void SetScore(Score to) {
      *reinterpret_cast<float*>(base_) = to;
    }
    bool operator<(const Header &other) const {
      return GetScore() < other.GetScore();
    }
    bool operator>(const Header &other) const {
      return GetScore() > other.GetScore();
    }

    Arity GetArity() const {
      return *reinterpret_cast<const Arity*>(base_ + sizeof(Score));
    }

    void *GetData() const {
      return *reinterpret_cast<void* const *>(base_ + sizeof(Score) + sizeof(Arity));
    }
    void SetData(void *to) {
      *reinterpret_cast<void**>(base_ + sizeof(Score) + sizeof(Arity)) = to;
    }

    Note GetNote() const {
      return *reinterpret_cast<const Note*>(base_ + sizeof(Score) + sizeof(Arity) + sizeof(void*));
    }
    void SetNote(Note to) {
      *reinterpret_cast<Note*>(base_ + sizeof(Score) + sizeof(Arity) + sizeof(void*)) = to;
    }

    uint8_t *Base() { return base_; }
    const uint8_t *Base() const { return base_; }

  protected:
    Header() : base_(NULL) {}

    explicit Header(void *base) : base_(static_cast<uint8_t*>(base)) {
      *reinterpret_cast<void**>(base_ + sizeof(Score) + sizeof(Arity)) = NULL;
    }

    Header(void *base, Arity arity) : base_(static_cast<uint8_t*>(base)) {
      *reinterpret_cast<Arity*>(base_ + sizeof(Score)) = arity;
      *reinterpret_cast<void**>(base_ + sizeof(Score) + sizeof(Arity)) = NULL;
    }

    static const std::size_t kHeaderSize = sizeof(Score) + sizeof(Arity) + sizeof(void*) + sizeof(Note);

    uint8_t *After() { return base_ + kHeaderSize; }
    const uint8_t *After() const { return base_ + kHeaderSize; }

  private:
    // Data elements occur in the following order: Score, Arity, Data, Note
    uint8_t *base_;
};

} // namespace search

#endif // SEARCH_HEADER__
