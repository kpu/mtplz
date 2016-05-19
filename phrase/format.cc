#include "phrase/format.hh"

namespace phrase {

namespace {
struct SizeHeader {
  // Size of mapped region.
  uint64_t map;
  // Total expected size after header.
  uint64_t total;
};
} // namespace

FileFormat::FileFormat(int fd, const std::string &header, bool writing, util::LoadMethod load_method)
  : file_(fd), writing_(writing), header_offset_(header.size() + sizeof(SizeHeader)) {
  SizeHeader h;
  if (writing) {
    util::WriteOrThrow(fd, header.data(), header.size());
    // Write an invalid size header to indicate the file is incomplete.
    h.map = 1;
    h.total = 0;
    util::WriteOrThrow(fd, &h, sizeof(SizeHeader));
    // Note: seeked to direct write location.
  } else {
    std::string buf(header.size() , 0);
    util::ReadOrThrow(fd, &buf[0], buf.size());
    UTIL_THROW_IF_ARG(buf != header, FDException, (fd), "Header did not match reference value");
    util::ReadOrThrow(fd, &h, sizeof(Header));
    UTIL_THROW_IF_ARG(h.map > h.total, FDException, (fd), "Binary file is incomplete or corrupt");
    util::MapRead(load_method, fd, 0 /* header is unlikely to be more than a page */, CheckOverflow((uint64_t)header_offset_ + h.map), full_backing_);
    // Setup reading for vocab.  TODO: support reading from pipe.
    util::SeekOrThrow(fd, header_offset_ + h.map);
  }
}

FileRegion &FileFormat::Attach() {
  void *base = regions_.empty() ? 
  regions_.emplace_back();
  if (!writing_) {
    regions_.back().mem_.reset(
  }
}

void FileFormat::Write() {

  for (FileRegion &r : regions_) {
    
  }
}

} // namespace phrase
