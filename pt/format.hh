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
#include "util/file_piece.hh"
#include "util/mmap.hh"

#include <cassert>
#include <cstddef>
#include <deque>

namespace pt {

extern const char kFileHeader[];

class VocabRange {
  public:
    typedef util::LineIterator Iterator;

    explicit VocabRange(int fd) : file_(util::DupOrThrow(fd)) {}

    util::LineIterator begin() { return util::LineIterator(file_, '\0'); }
    util::LineIterator end() const { return util::LineIterator(); }

  private:
    util::FilePiece file_;
};

class FileFormat {
  public:
    // Takes ownership of file.
    FileFormat(int fd, const std::string &header, bool writing, util::LoadMethod load_method);

    util::scoped_memory &Attach();

    // Two special regions:
    // 1. Target phrases at the beginning are written directly to the file.
    void DirectWriteTargetPhrases(void *data, std::size_t size) {
      util::WriteOrThrow(file_.get(), data, size);
      direct_write_size_ += size;
    }
    uint64_t DirectWriteSize() const { return direct_write_size_; }

    // Only for reading the vocab from a binary file.
    VocabRange Vocab() {
      util::SeekOrThrow(file_.get(), vocab_offset_);
      return VocabRange(file_.get());
    }

    void Write();

    bool Writing() const { return writing_; }
  private:
    util::scoped_fd file_;
    bool writing_;

    std::deque<util::scoped_memory> regions_;

    // When the file is mapped as one giant region, this is the giant region.
    util::scoped_memory full_backing_;
    
    uint64_t direct_write_size_ = 0;

    std::size_t header_offset_;

    uint64_t vocab_offset_;
};

} // namespace pt
