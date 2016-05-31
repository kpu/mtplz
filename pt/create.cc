#include "pt/create.hh"

#include "pt/access.hh"
#include "pt/format.hh"
#include "pt/hash.hh"
#include "pt/hash_table_region.hh"
#include "pt/record_writer.hh"
#include "pt/word_array.hh"

#include "util/mutable_vocab.hh"

#include "util/double-conversion/double-conversion.h"
#include "util/file_piece.hh"
#include "util/murmur_hash.hh"
#include "util/tokenize_piece.hh"

#include <cmath>
#include <algorithm>

namespace pt {

namespace {

void CountColumns(StringPiece part, std::size_t &count) {
  if (!FieldConfig::Present(count)) return;
  count = 0;
  for (util::TokenIter<util::BoolCharacter, true> i(part); i; ++i, ++count) {}
}

class SourceHasher {
  public:
    explicit SourceHasher(util::GrowableVocab<WordArray> &vocab) : vocab_(vocab) {}

    uint64_t operator()(StringPiece source) {
      for (util::TokenIter<util::BoolCharacter, true> i(source); i; ++i) {
        reuse_.push_back(vocab_.FindOrInsert(*i));
      }
      uint64_t ret = HashSource(&*reuse_.begin(), &*reuse_.begin() + reuse_.size());
      reuse_.clear();
      return ret;
    }

  private:
    util::GrowableVocab<WordArray> &vocab_;
    std::vector<WordIndex> reuse_;
};

void ExtractLine(StringPiece from, std::vector<StringPiece> &out) {
  util::TokenIter<util::MultiCharacter> pipes(from, "|||");
  for (std::vector<StringPiece>::iterator i = out.begin(); i != out.end(); ++i, ++pipes) {
    *i = *pipes; // This does throw already if pipes is broken.
  }
  UTIL_THROW_IF2(pipes, "Too many fields in line " << from);
}

static const double_conversion::StringToDoubleConverter kConverter(0, std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN(), "inf", "NaN");

void ParseFloats(StringPiece from, OptionalField<util::ArrayField<float> > &field, Row *row) {
  if (!field) return;
  util::TokenIter<util::BoolCharacter, true> token(from);
  float *to = field(row).begin();
  float *const end = field(row).end();
  for (; to != end; ++to, ++token) {
    int processed;
    *to = kConverter.StringToFloat(token->data(), token->size(), &processed);
    using namespace std;
    UTIL_THROW_IF2(isnan(*to), "Bad floating point number " << *token << " in section: " << from);
    UTIL_THROW_IF2(processed != token->size(), "Did not process full float for " << *token);
  }
  UTIL_THROW_IF2(token, "More than " << field(row).size() << " floats in " << from);
}

} // namespace

void CreateTable(int from, int to, const TextColumns columns, FieldConfig &config) {
  util::FilePiece f(from, NULL, &std::cerr);
  util::LineIterator line = f.begin();
  UTIL_THROW_IF2(!line, "Empty phrase table file");

  std::vector<StringPiece> parsed_line;
  std::copy(
      util::TokenIter<util::MultiCharacter>(*line, "|||"),
      util::TokenIter<util::MultiCharacter>::end(),
      std::back_insert_iterator<std::vector<StringPiece> >(parsed_line));

  const std::size_t have_fields = parsed_line.size();

  UTIL_THROW_IF2(columns.source >= have_fields, "Text file has " << have_fields << " columns, but the source is supposed to be in column " << columns.source);
  StringPiece &source = parsed_line[columns.source];
#define BIND_COLUMN(name) \
  UTIL_THROW_IF2(FieldConfig::Present(config.name) && columns.name >= have_fields, "Text file has " << have_fields << " columns, but " #name  " is supposed to be in column " << columns.name); \
  StringPiece &name = parsed_line[FieldConfig::Present(config.name) ? columns.name : 0];

  BIND_COLUMN(target);
  BIND_COLUMN(dense_features);
  BIND_COLUMN(sparse_features);
  BIND_COLUMN(lexical_reordering);

  // Compute number of feature columns from first row.
  CountColumns(dense_features, config.dense_features);
  CountColumns(lexical_reordering, config.lexical_reordering);
  // Now we have a fully-configured set of columns.

  FileFormat file(to, kFileHeader, true, util::POPULATE_OR_READ /* does not matter since this is the reading method */);
  TargetWriter target_write(file);
  config.Save(file.Attach());
  HashTableRegion<uint64_t> offsets(file);
  util::GrowableVocab<WordArray> vocab(100, file);
  SourceHasher source_hasher(vocab);
  Access access(config);

  uint64_t source_hash = source_hasher(source), new_source_hash;
  while (line) {
    offsets.Insert(source_hash, target_write.Offset());
    TargetBundleWriter bundle(target_write);
    do {
      Row *row = access.Allocate(bundle);

      // Fill target.
      util::VectorField<WordIndex, VectorSize>::FakeVector<TargetBundleWriter> vec = access.target(row, bundle);
      for (util::TokenIter<util::BoolCharacter, true> t(target); t; ++t) {
        vec.push_back(vocab.FindOrInsert(*t));
      }
      ParseFloats(dense_features, access.dense_features, row);
      ParseFloats(lexical_reordering, access.lexical_reordering, row);
      // TODO sparse features
 
      if (!++line) break;
      ExtractLine(*line, parsed_line);
    } while ((new_source_hash = source_hasher(source)) == source_hash);
    source_hash = new_source_hash;
  }

  vocab.Action().Finish();
  file.Write();
}

} // namespace pt
