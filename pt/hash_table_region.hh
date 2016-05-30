#pragma once

#include "pt/format.hh"
#include "pt/types.hh"
#include "util/probing_hash_table.hh"
#include "util/layout.hh"
#include "util/murmur_hash.hh"

#include <cstddef>

namespace pt {

template <class Value> class HashTableRegion {
  private:
    static constexpr float kDefaultMultiply = 1.2;
  public:
    explicit HashTableRegion(FileFormat &format) : backing_(format.Attach()) {
      if (format.Writing()) {
        HugeRealloc(Table::Size(10, kDefaultMultiply), true, backing_);
      }
      table_ = Table(backing_.get(), backing_.size());
      insert_threshold_ = table_.Buckets() / kDefaultMultiply;
    }

    void Insert(uint64_t hash, const Value &value) {
      if (table_.SizeNoSerialization() > insert_threshold_) {
        HugeRealloc(table_.DoubleTo(), true, backing_);
        table_.Double(backing_.get(), false);
        insert_threshold_ = table_.Buckets() / kDefaultMultiply;
      }
      Entry entry;
      entry.key = hash;
      entry.value = value;
      table_.Insert(entry);
    }

    bool Find(uint64_t hash, const Value *&found) const {
      typename Table::ConstIterator i;
      if (table_.Find(hash, i)) {
        found = &i->value;
        return true;
      }
      return false;
    }

  private:
    struct Entry {
      Entry(uint64_t k, const Value &v) : key(k), value(v) {}
      Entry() {}
      typedef uint64_t Key;
      uint64_t key;
      uint64_t GetKey() const { return key; }
      void SetKey(uint64_t to) { key = to; }
      Value value;
    };

    typedef util::ProbingHashTable<Entry, util::IdentityHash, std::equal_to<uint64_t>, util::Power2Mod> Table;
    Table table_;

    std::size_t insert_threshold_;

    util::scoped_memory &backing_;
};
} // namespace pt
