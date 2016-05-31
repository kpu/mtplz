#include "pt/query.hh"

#include "util/file.hh"

namespace pt {

namespace {
FieldConfig LoadFieldConfig(FileFormat &format) {
  FieldConfig ret;
  ret.Restore(format.Attach());
  return ret;
}
} // namespace

Table::Table(int fd, util::LoadMethod load_method)
  : file_(fd, kFileHeader, false, load_method),
    rows_(file_.Attach()),
    access_(LoadFieldConfig(file_)),
    offsets_(file_) {}

Table::Table(const char *file, util::LoadMethod load_method)
  : Table(util::OpenReadOrThrow(file), load_method) {}

} // namespace pt
