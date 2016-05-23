#pragma once

#include "phrase/types.hh"

#include <boost/optional.hpp> // Or C++17

/* Accessors to interpret a target phrase pointer */
namespace phrase {

template <class Field> class OptionalField {
  public:
    OptionalField() {}

    OptionalField &operator=(const Field &f) {
      field_ = f;
    }

    auto operator()(const Phrase *phr) const {
      return (*field_)(phr);
    }
    template <class Alloc> auto operator()(Phrase *phr, Alloc &alloc) const {
      return (*field_)(phr, alloc);
    }
    auto operator()(Phrase *phr) const {
      return (*field_)(phr);
    }
    operator bool() const { return field_; }

  private:
    boost::optional<Field> field_;
};

// Usage:
// To check if lexicalized reordering is in the phrase table:
//   if (access.lexicalized_reordering)
// Once you're sure it's there, access:
//   access.lexicalized_reordering(phrase);
class Access {
  public:
    Access() {}

    util::VectorField<WordIndex, VectorSize> words;
    util::ArrayField<float> dense_features;
    OptionalField<util::VectorField<SparseFeature> > sparse_features;
    OptionalField<util::ArrayField<float> > lexical_reordering;
    // TODO word alignment, properties?

    const Phrase *Next(const Phrase *phrase) const {
      // TODO optimize.
      const uint8_t *base = reinterpret_cast<const uint8_t*>(phrase);
      return phrase + layout_.OffsetsEnd() + 
        *(reinterpret_cast<const VectorSize*>(base + layouts_.OffsetsEnd()) - 1);
    }

  private:
    util::Layout layout_;
};

} // namespace phrase
