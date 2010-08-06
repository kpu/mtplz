#include "lm/filter_phrase.hh"
#include "lm/filter_format.hh"

#include <algorithm>
#include <functional>
#include <iostream>
#include <queue>
#include <string>
#include <vector>

#include <ctype.h>

namespace lm {
namespace phrase {

unsigned int ReadMultiple(std::istream &in, Substrings &out) {
  bool sentence_content = false;
  unsigned int sentence_id = 0;
  std::vector<Hash> phrase;
  std::string word;
  while (in) {
    char c;
    // Gather a word.
    while (!isspace(c = in.get()) && in) word += c;
    // Treat EOF like a newline.
    if (!in) c = '\n';
    // Add the word to the phrase.
    if (!word.empty()) {
      phrase.push_back(detail::StringHash(word));
      word.clear();
    }
    if (c == ' ') continue;
    // It's more than just a space.  Close out the phrase.  
    if (!phrase.empty()) {
      sentence_content = true;
      out.AddPhrase(sentence_id, phrase.begin(), phrase.end());
      phrase.clear();
    }
    if (c == '\t' || c == '\v') continue;
    // It's more than a space or tab: a newline.   
    if (sentence_content) {
      ++sentence_id;
      sentence_content = false;
    }
  }
  if (!in.eof()) in.exceptions(std::istream::failbit | std::istream::badbit);
  return sentence_id + sentence_content;
}

namespace detail { const StringPiece kEndSentence("</s>"); }

namespace {

typedef unsigned int Sentence;
typedef std::vector<Sentence> Sentences;

class Vertex;

class Arc {
  public:
    Arc() {}

    // For arcs from one vertex to another.  
    void SetPhrase(Vertex &from, Vertex &to, const Sentences &intersect) {
      Set(to, intersect);
      from_ = &from;
    }

    /* For arcs from before the n-gram begins to somewhere in the n-gram (right
     * aligned).  These have no from_ vertex; it implictly matches every
     * sentence.  This also handles when the n-gram is a substring of a phrase. 
     */
    void SetRight(Vertex &to, const Sentences &complete) {
      Set(to, complete);
      from_ = NULL;
    }

    Sentence Current() const {
      return *current_;
    }

    bool Empty() const {
      return current_ == last_;
    }

    /* When this function returns:
     * If Empty() then there's nothing left from this intersection.
     *
     * If Current() == to then to is part of the intersection. 
     *
     * Otherwise, Current() > to.  In this case, to is not part of the
     * intersection and neither is anything < Current().  To determine if
     * any value >= Current() is in the intersection, call LowerBound again
     * with the value.   
     */
    void LowerBound(const Sentence to);

  private:
    void Set(Vertex &to, const Sentences &sentences);

    const Sentence *current_;
    const Sentence *last_;
    Vertex *from_;
};

struct ArcGreater : public std::binary_function<const Arc *, const Arc *, bool> {
  bool operator()(const Arc *first, const Arc *second) const {
    return first->Current() > second->Current();
  }
};

class Vertex {
  public:
    Vertex() : current_(0) {}

    Sentence Current() const {
      return current_;
    }

    bool Empty() const {
      return incoming_.empty();
    }

    void LowerBound(const Sentence to);

  private:
    friend class Arc;

    void AddIncoming(Arc *arc) {
      if (!arc->Empty()) incoming_.push(arc);
    }

    unsigned int current_;
    std::priority_queue<Arc*, std::vector<Arc*>, ArcGreater> incoming_;
};

void Arc::LowerBound(const Sentence to) {
  current_ = std::lower_bound(current_, last_, to);
  // If *current_ > to, don't advance from_.  The intervening values of
  // from_ may be useful for another one of its outgoing arcs.
  if (!from_ || Empty() || (Current() > to)) return;
  assert(Current() == to);
  from_->LowerBound(to);
  if (from_->Empty()) {
    current_ = last_;
    return;
  }
  assert(from_->Current() >= to);
  if (from_->Current() > to) {
    current_ = std::lower_bound(current_ + 1, last_, from_->Current());
  }
}

void Arc::Set(Vertex &to, const Sentences &sentences) {
  current_ = &*sentences.begin();
  last_ = &*sentences.end();
  to.AddIncoming(this);
}

void Vertex::LowerBound(const Sentence to) {
  if (Empty()) return;
  // Union lower bound.  
  while (true) {
    Arc *top = incoming_.top();
    if (top->Current() > to) {
      current_ = top->Current();
      return;
    }
    // If top->Current() == to, we still need to verify that's an actual 
    // element and not just a bound.  
    incoming_.pop();
    top->LowerBound(to);
    if (!top->Empty()) {
      incoming_.push(top);
      if (top->Current() == to) {
        current_ = to;
        return;
      }
    } else if (Empty()) {
      return;
    }
  }
}

void BuildGraph(const Substrings &phrase, const std::vector<Hash> &hashes, Vertex *const vertices, Arc *free_arc) {
  assert(!hashes.empty());

  const Hash *const first_word = &*hashes.begin();
  const Hash *const last_word = &*hashes.end() - 1;

  Hash hash = 0;
  const Sentences *found;
  // Phrases starting at or before the first word in the n-gram.
  {
    Vertex *vertex = vertices;
    for (const Hash *word = first_word; ; ++word, ++vertex) {
      detail::CombineHash(hash, *word);
      // Now hash is [hashes.begin(), word].
      if (word == last_word) {
        if (phrase.FindSubstring(hash, found))
          (free_arc++)->SetRight(*vertex, *found);
        break;
      }
      if (!phrase.FindRight(hash, found)) break;
      (free_arc++)->SetRight(*vertex, *found);
    }
  }

  // Phrases starting at the second or later word in the n-gram.   
  Vertex *vertex_from = vertices;
  for (const Hash *word_from = first_word + 1; word_from != &*hashes.end(); ++word_from, ++vertex_from) {
    hash = 0;
    Vertex *vertex_to = vertex_from + 1;
    for (const Hash *word_to = word_from; ; ++word_to, ++vertex_to) {
      // Notice that word_to and vertex_to have the same index.  
      detail::CombineHash(hash, *word_to);
      // Now hash covers [word_from, word_to].
      if (word_to == last_word) {
        if (phrase.FindLeft(hash, found))
          (free_arc++)->SetPhrase(*vertex_from, *vertex_to, *found);
        break;
      }
      if (!phrase.FindPhrase(hash, found)) break;
      (free_arc++)->SetPhrase(*vertex_from, *vertex_to, *found);
    }
  }
}

} // namespace

namespace detail {

} // namespace detail

bool Union::Evaluate() {
  assert(!hashes_.empty());
  // Usually there are at most 6 words in an n-gram, so stack allocation is reasonable.  
  Vertex vertices[hashes_.size()];
  // One for every substring.  
  Arc arcs[((hashes_.size() + 1) * hashes_.size()) / 2];
  BuildGraph(substrings_, hashes_, vertices, arcs);
  Vertex &last_vertex = vertices[hashes_.size() - 1];

  unsigned int lower = 0;
  while (true) {
    last_vertex.LowerBound(lower);
    if (last_vertex.Empty()) return false;
    if (last_vertex.Current() == lower) return true;
    lower = last_vertex.Current();
  }
}

template <class OutputT> void Multiple<OutputT>::Evaluate(const std::string &line) {
  assert(!hashes_.empty());
  // Usually there are at most 6 words in an n-gram, so stack allocation is reasonable.  
  Vertex vertices[hashes_.size()];
  // One for every substring.  
  Arc arcs[((hashes_.size() + 1) * hashes_.size()) / 2];
  BuildGraph(substrings_, hashes_, vertices, arcs);
  Vertex &last_vertex = vertices[hashes_.size() - 1];

  unsigned int lower = 0;
  while (true) {
    last_vertex.LowerBound(lower);
    if (last_vertex.Empty()) return;
    if (last_vertex.Current() == lower) output_.SingleAddNGram(lower, line);
    lower = last_vertex.Current();
  }
}

template void Multiple<CountFormat::Multiple>::Evaluate(const std::string &line);
template void Multiple<ARPAFormat::Multiple>::Evaluate(const std::string &line);

} // namespace phrase
} // namespace lm