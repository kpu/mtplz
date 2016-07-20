#pragma once

#include "pt/access.hh"
#include "pt/format.hh"
#include "pt/hash.hh"
#include "pt/hash_table_region.hh"

#include <cassert>
#include <iterator>

namespace pt {

class Statistics;

/*class CurriedRow {
  public:
    CurriedRow(const Row *row, const Access *access) : row_(row), access_(access) {}

    boost::iterator_range<const WordIndex*> Target() const { return access_->target(row_); }
    boost::iterator_range<const float*> DenseFeatures() const { return access_->dense_features(row_); }
    boost::iterator_range<const SparseFeature*> SparseFeatures() const { return access_->sparse_features(row_); }
    boost::iterator_range<const float*> LexicalReordering() const { return access_->lexical_reordering(row_); }

    const Access &Accessor() const { return *access_; }

  private:
    const Row *row_;
    const Access *access_;
};*/

class RowIterator : public std::iterator<std::forward_iterator_tag, const Row> {
  public:
    RowIterator() {}

    RowIterator(const Row *row, const Access *access, RowCount remaining) : row_(row), access_(access), remaining_(remaining) {}

    RowIterator &operator++() {
      row_ = access_->End(row_);
      --remaining_;
      return *this;
    }

    const Row &operator*() { return *row_; }
    const Row *operator->() { return row_; }
    operator const Row *() const { return row_; }

    // This is really only used to test for end of sequence.
    // The C++ standard only requires that equality and inequality be defined
    // for the same underlying sequence.
    bool operator==(const RowIterator &other) const {
      return remaining_ == other.remaining_;
    }

    bool operator!=(const RowIterator &other) const { return !(*this == other); }

    operator bool() const { return remaining_; }

    const Access &Accessor() const { return *access_; }

    // Remaining goes down.
    std::ptrdiff_t operator-(const RowIterator &other) const {
      return other.remaining_ - remaining_;
    }

  private:
    const Row *row_;
    const Access *access_;
    RowCount remaining_;
};

class Table {
  public:
    // Takes ownership of fd.
    Table(int fd, util::LoadMethod load_method);

    Table(const char *file, util::LoadMethod load_method);

    boost::iterator_range<RowIterator> Lookup(const WordIndex *source_begin, const WordIndex *source_end) const {
      return boost::iterator_range<RowIterator>(Begin(source_begin, source_end), RowIterator(nullptr, &access_, 0));
    }

    const Access &Accessor() { return access_; }

    const Statistics &Stats() const { return stats_; }

    VocabRange Vocab() { return file_.Vocab(); }

  private:
    RowIterator Begin(const WordIndex *source_begin, const WordIndex *source_end) const {
      const uint64_t *found;
      if (!offsets_.Find(HashSource(source_begin, source_end), found))
        return RowIterator(nullptr, &access_, 0);
      const char *base = rows_.begin() + *found;
      RowCount count = *reinterpret_cast<const RowCount*>(base);
      return RowIterator(reinterpret_cast<const Row*>(base + sizeof(RowCount)), &access_, count);
    }

    FileFormat file_;
    util::scoped_memory &rows_;
    const Statistics &stats_;
    Access access_;
    HashTableRegion<uint64_t> offsets_;
};

} // namespace pt
