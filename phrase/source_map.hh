#ifndef PT_SOURCE_MAP_H
#define PT_SOURCE_MAP_H

#include "phrase/types.hh"
#include "util/probing_hash_table.hh"
#include "util/layout.hh"
#include "util/murmur_hash.hh"

#include <cstddef>

namespace pt {
class SourceMap {
  public:
    SourceMap(util::scoped_memory &backing, bool writing) : backing_(backing) {
      if (writing) {
        HugeRealloc(Table::Size(100, 1.2), true, backing_);
      }
      table_ = Table(backing_.get(), backing_.size());
      insert_threshold_ = table_.Buckets() / 1.2;
    }

    void Insert(const WordIndex *src_begin, const WordIndex *src_end, uint64_t offset) {
      if (table.SizeNoSerialization() > insert_threshold_) {
        HugeRealloc(table_.DoubleTo(), true, backing_);
        table_.Double(backing_.get(), false);
        insert_threshold_ *= 2;
      }
      Entry entry;
      entry.src_hash = Hash(src_begin, src_end);
      entry.tgt_offset = offset;
      Table::MutableIterator it;
      UTIL_THROW_IF2(table.FindOrInsert(entry, it), "Duplicate source phrase or collision");
    }

    bool Lookup(const WordIndex *src_begin, const WordIndex *src_end, uint64_t &offset) const {
      Table::ConstIterator i;
      if (!table.Find(Hash(src_begin, src_end), i)) return false;
      offset = i->tgt_offset;
      return true;
    }

  private:
    static uint64_t Hash(const WordIndex *src_begin, const WordIndex *src_end) const {
      return util::MurmurHashNative(src_begin, (src_end - src_begin) * sizeof(WordIndex));
    }

    struct Entry {
      uint64_t src_hash;
      uint64_t tgt_offset;
      uint64_t GetKey() const { return src_hash; }
      void SetKey(uint64_t to) { src_hash = to; }
    };

    typedef util::ProbingHashTable<Entry, util::IdentityHash, std::equal_to<uint64_t>, util::Power2Mod> Table;
    Table table_;

    std::size_t insert_threshold_;

    util::scoped_memory &backing_;
};
} // namespace pt

#endif // PT_SOURCE_MAP_H
