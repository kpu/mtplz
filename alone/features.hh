// Handling for individual feature values.  
// Feature vectors are represented sparsely as a vector pairing id and value.
#ifndef ALONE_FEATURES__
#define ALONE_FEATURES__

#include "search/types.hh"
#include "util/exception.hh"
#include "util/murmur_hash.hh"
#include "util/pool.hh"
#include "util/string_piece.hh"

#include <boost/noncopyable.hpp>
#include <boost/unordered_map.hpp>

#include <cstddef>
#include <functional>
#include <iosfwd>
#include <queue>
#include <string>
#include <vector>

#include <stdint.h>

namespace alone {
namespace feature {

typedef uint32_t ID;

class Vector {
  public:
    Vector() {}

  private:
    friend class Adder;
    friend class Weights;
    friend class Test;

    struct Entry {
      ID id;
      search::Score score;

      // sorting order inside values_.  
      bool operator<(const Entry &other) const {
        return id < other.id;
      }
    };

    std::vector<Entry> values_;
};

// Sum a bunch of Vector at once.  
class Adder {
  public:
    void Add(const Vector &vec) {
      if (vec.values_.empty()) return;
      Part part;
      part.cur = &*vec.values_.begin();
      part.end = part.cur + vec.values_.size();
      queue_.push(part);
    }

    void Finish(Vector &out);

  private:
    struct Part {
      const Vector::Entry *cur, *end;
      bool operator>(const Part &other) const {
        return cur->id > other.cur->id;
      }
    };

    std::priority_queue<Part, std::vector<Part>, std::greater<Part> > queue_;
};

class WeightParseException : public util::Exception {
  public:
    WeightParseException() {}
    ~WeightParseException() throw() {}
};

// Integrated weight and string identification.  Why?  One hash table lookup
// for parsing.  
class Weights : boost::noncopyable {
  public:
    Weights();

    // Makes a partly shallow copy for use by threads.  
    // This does copy a lot though; if we're going to have a lot of features,
    // then this should be replaced with locking or freezing the feature
    // vector to make it immutable.  
    // Discriminated from copy constructor to make sure user is aware.  
    struct ForThread {};
    Weights(ForThread, const Weights &copy_from);

    // Initialization: add weights from these sources.
    void AppendFromFile(const char *name);
    void AppendFromString(StringPiece str);

    // Parse a feature vector, returning the dot product with weights.
    search::Score Parse(StringPiece from, Vector &to);

    std::ostream &Write(std::ostream &to, const Vector &from) const;

    // Lookup a feature by name and complain if it wasn't found.  This is
    // intended to setup weights for hard-coded features,  Callers should cache
    // the result.  
    search::Score Lookup(StringPiece name, std::ostream *complain = NULL) const;

  private:
    ID Add(StringPiece str, search::Score weight);

    // owns strings behind the StringPieces.  
    util::Pool pool_;
    // id to string lookup
    std::vector<StringPiece> id_;

    // id and weight for a feature name.  
    struct Value {
      ID id;
      search::Score weight;
    };

    struct HashMurmur : public std::unary_function<const StringPiece &, std::size_t> {
      std::size_t operator()(const StringPiece &str) const {
        return util::MurmurHashNative(str.data(), str.size());
      }
    };

    typedef boost::unordered_map<StringPiece, Value, HashMurmur> Map;
    Map str_;
};

} // namespace feature
} // namespace alone

#endif // ALONE_FEATURES__
