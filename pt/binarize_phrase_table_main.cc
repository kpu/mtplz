#include "pt/access.hh"
#include "pt/create.hh"
#include "util/exception.hh"
#include "util/file.hh"

#include <boost/program_options.hpp>

#include <iostream>
#include <string>
#include <vector>

namespace pt {
namespace {

template <class Value> bool Bind(const std::string &col, const char *name, std::size_t index, Value &field, std::size_t &out_index) {
  if (col != name) return false;
  UTIL_THROW_IF2(FieldConfig::Present(field), "Column " << col << " is already present.");
  field = 1;
  out_index = index;
  return true;
}

} // namespace
} // namespace pt

int main(int argc, char *argv[]) {
  try {
    namespace po = boost::program_options;
    po::options_description options("Phrase table binarization options");

    std::vector<std::string> default_columns = {
      "source",
      "target",
      "dense_features",
      "sparse_features",
      "lexical_reordering",
    };
    std::string default_columns_string;
    for (const std::string &col : default_columns) {
      default_columns_string += col;
      default_columns_string += ' ';
    }
    default_columns_string.resize(default_columns_string.size() - 1);

    options.add_options()
      ("help,h", po::bool_switch(), "Show this help message")
      ("columns,c", po::value<std::vector<std::string> >()->multitoken()->default_value(default_columns, default_columns_string), "Columns in the text phrase table.  Use `ignore' to skip a column.");
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, options), vm);

    bool stdout_is_seekable = true;
    try {
      util::SeekOrThrow(1, 0);
    } catch (const util::FDException &) {
      stdout_is_seekable = false;
    }

    if (vm["help"].as<bool>() || !stdout_is_seekable) {
      std::cerr << 
        "Converts a text phrase table to mtplz binary format.\n"
        "Usage: " << argv[0] << " <pt.text >pt.binary\n"
        "Where pt.binary must be a seekable file.\n"
        << options << std::endl;
      return 1;
    }
    po::notify(vm);

    using namespace pt;
    bool have_source = false;
    FieldConfig fields;
    fields.target = false;
    TextColumns columns;
    std::size_t index = 0;
    for (const std::string &col : vm["columns"].as<std::vector<std::string> >()) {
      UTIL_THROW_IF2(
          col != "ignore"
          && !Bind(col, "source", index, have_source, columns.source)
          && !Bind(col, "target", index, fields.target, columns.target)
          && !Bind(col, "dense_features", index, fields.dense_features, columns.dense_features)
          && !Bind(col, "sparse_features", index, fields.sparse_features, columns.sparse_features)
          && !Bind(col, "lexical_reordering", index, fields.lexical_reordering, columns.lexical_reordering),
        "Bad column name " << col);
      ++index;
    }
    UTIL_THROW_IF2(!have_source, "Source is a required column.");
    CreateTable(0, 1, columns, fields);
  } catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }
  return 0;
}
