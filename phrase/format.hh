#pragma once
/* File layout.  This is different from the Layout class because it has
 * different performance characteristics and purpose:
 * 1. There are a few big regions.  We can afford to store the size of all of
 *    them.
 * 2. While the data will be contiguous on disk, it doesn't have to be
 *    contiguous in RAM during building time.  Separately-malloced resizable
 *    locations are ok.
 * 3. Creation of a single object rather than a bunch.
 * In contrast, Layout is designed for lots of small objects contiguous in
 * memory from the beginning.
 */

#include "util/file.hh"
#include "util/mmap.hh"

#include <cassert>
#include <cstddef>
#include <deque>

namespace phrase {

class FileRegion {
  public:
    FileRegion() {}

    void Resize(std::size_t to, bool zero_new) {
      util::HugeRealloc(to, zero_new, mem_);
    }

    const void *get() const { return mem_.get(); }
    void *get() { return mem_.get(); }

  private:
    friend class FileFormat;
    util::scoped_memory mem_;
};

class FileFormat {
  public:
    // Takes ownership of file.
    FileFormat(int fd, const std::string &header, bool writing, util::LoadMethod load_method);

    FileRegion &Attach();

    // Two special regions:
    // 1. Target phrases at the beginning are written directly to the file.
    void DirectWriteTargetPhrases(void *data, std::size_t size) {
      util::WriteOrThrow(file_.get(), data, size);
      direct_write_size_ += size;
    }
    // 2. Vocab at the end is created in RAM but not read into RAM.
    void DirectReadVocab(void *data, std::size_t size) {
      util::ReadOrThrow(file_.get(), data, size);
    }

    void Write();

  private:
    util::scoped_fd file_;
    bool writing_;

    std::deque<FileRegion> regions_;

    // When the file is mapped as one giant region, this is the giant region.
    util::scoped_memory full_backing_;
    
    uint64_t direct_write_size_ = 0;

    std::size_t header_offset_;
};

} // namespace phrase
