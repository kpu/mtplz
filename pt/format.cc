#include "pt/format.hh"

namespace pt {

namespace {
struct SizeHeader {
  // Size of mapped region.
  uint64_t map;
  // Total expected size after header.
  uint64_t total;
};
} // namespace

const char kFileHeader[] = "mtplz phrase table version 0";

FileFormat::FileFormat(int fd, const std::string &header, bool writing, util::LoadMethod load_method)
  : file_(fd), writing_(writing), header_offset_(header.size() + sizeof(SizeHeader)) {
  SizeHeader h;
  if (writing) {
    util::WriteOrThrow(fd, header.data(), header.size());
    // Write an invalid size header to indicate the file is incomplete.
    h.map = 1;
    h.total = 0;
    util::WriteOrThrow(fd, &h, sizeof(SizeHeader));
    // Leave space for the direct write.
    uint64_t direct_write_size = 0;
    util::WriteOrThrow(fd, &direct_write_size, sizeof(direct_write_size));
    // Note: seeked to direct write location.
  } else {
    std::string buf(header.size() , 0);
    util::ReadOrThrow(fd, &buf[0], buf.size());
    UTIL_THROW_IF_ARG(buf != header, util::FDException, (fd), "Header did not match reference value");
    util::ReadOrThrow(fd, &h, sizeof(SizeHeader));
    UTIL_THROW_IF_ARG(h.map > h.total, util::FDException, (fd), "Binary file is incomplete or corrupt");
    util::MapRead(load_method, fd, 0 /* header is unlikely to be more than a page */, util::CheckOverflow((uint64_t)header_offset_ + h.map), full_backing_);
    // Setup reading for vocab.  TODO: support reading from pipe.
    util::SeekOrThrow(fd, header_offset_ + h.map);
  }
}

util::scoped_memory &FileFormat::Attach() {
  if (writing_) {
    regions_.emplace_back();
  } else {
    char *base = regions_.empty() ? full_backing_.begin() : regions_.back().end();
    const uint64_t &size = *reinterpret_cast<const uint64_t*>(base);
    UTIL_THROW_IF2(base + sizeof(uint64_t) + size > full_backing_.end(), "File size header " << size << " goes off end of file.");
    regions_.emplace_back(base + sizeof(uint64_t), size, util::scoped_memory::NONE_ALLOCATED);
  }
  return regions_.back();
}

void FileFormat::Write() {
  SizeHeader head;
  head.total = direct_write_size_;
  for (util::scoped_memory &r : regions_) {
    head.total += r.size();
  }
  // Exclude the vocabulary from the mapped region.
  assert(!regions_.empty());
  head.map = head.total - regions_.back().size();
  // Write the total file size header.
  util::SeekOrThrow(file_.get(), header_offset_ - sizeof(SizeHeader));
  util::WriteOrThrow(file_.get(), &head, sizeof(SizeHeader));
  // Write the size of the direct-write region (its header), for which we had earlier reserved space.
  util::WriteOrThrow(file_.get(), &direct_write_size_, sizeof(uint64_t));
  // Write the regions with their own size headers.  This includes vocab.
  for (util::scoped_memory &r : regions_) {
    uint64_t size = r.size();
    util::WriteOrThrow(file_.get(), &size, sizeof(uint64_t));
    util::WriteOrThrow(file_.get(), r.get(), r.size());
  }
}

} // namespace pt
