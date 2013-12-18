#include "alone/features.hh"

#include "util/double-conversion/double-conversion.h"
#include "util/file_piece.hh"
#include "util/tokenize_piece.hh"

#include <limits>

#include <math.h>
#include <string.h>

namespace alone {
namespace feature {

void Adder::Finish(Vector &out_wrap) {
  typedef Vector::Entry Entry;
  std::vector<Entry> &out = out_wrap.values_;
  out.clear();

  ID previous = std::numeric_limits<ID>::max();
  while (!queue_.empty()) {
    Part top(queue_.top());
    queue_.pop();

    if (previous == top.cur->id) {
      out.back().score += top.cur->score;
    } else {
      out.push_back(*top.cur);
      previous = top.cur->id;
    }

    ++top.cur;
    if (top.cur != top.end) {
      queue_.push(top);
    }
  }
}

namespace {

struct ParseRet {
  StringPiece name;
  search::Score score;
};

double_conversion::StringToDoubleConverter converter(double_conversion::StringToDoubleConverter::NO_FLAGS, NAN, NAN, "inf", "nan");

// Parser allows = or space between feature name and value.  
ParseRet RawParse(util::TokenIter<util::AnyCharacter, true> &spaces) {
  ParseRet ret;
  StringPiece::size_type s = spaces->find('=');
  StringPiece value;
  if (s == StringPiece::npos) {
    ret.name = *spaces;
    value = *++spaces;
  } else {
    ret.name = StringPiece(spaces->data(), s);
    value = StringPiece(spaces->data() + s + 1, spaces->size() - s - 1);
  }
  ++spaces;

  int processed;
  ret.score = converter.StringToFloat(value.data(), value.length(), &processed);
  UTIL_THROW_IF(isnan(ret.score), WeightParseException, "Failed to parse weight" << value);
  return ret;
}

} // namespace

Weights::Weights() {}

void Weights::AppendFromFile(const char *name) {
  util::FilePiece f(name);
  try {
    while (true) {
      AppendFromString(f.ReadLine());
    }
  } catch (const util::EndOfFileException &e) {}
}

void Weights::AppendFromString(StringPiece from) {
  for (util::TokenIter<util::AnyCharacter, true> spaces(from, " \n\t"); spaces;) {
    ParseRet res(RawParse(spaces));
    Add(res.name, res.score);
  }
}

// copy id_ and str_ but assume pool_ is still alive from the parent.  
Weights::Weights(Weights::ForThread, const Weights &copy_from) 
  : id_(copy_from.id_), str_(copy_from.str_) {}

search::Score Weights::Parse(StringPiece from, Vector &to) {
  to.values_.clear();
  search::Score dot = 0.0;
  Vector::Entry to_add;

  for (util::TokenIter<util::AnyCharacter, true> spaces(from, " \n\t"); spaces;) {
    ParseRet res(RawParse(spaces));
    to_add.score = res.score;
    Map::iterator i(str_.find(res.name));
    if (i != str_.end()) {
      dot += i->second.weight * res.score;
      to_add.id = i->second.id;
    } else {
      to_add.id = Add(res.name, 0.0);
    }
    to.values_.push_back(to_add);
  }
  std::sort(to.values_.begin(), to.values_.end());
  return dot;
}

search::Score Weights::Lookup(StringPiece name, std::ostream *complain) const {
  Map::const_iterator i = str_.find(name);
  if (i == str_.end()) {
    if (complain) *complain << "Warning: hard-coded feature " << name << " was not given a weight." << std::endl;
    return 0.0;
  }
  return i->second.weight;
}

ID Weights::Add(StringPiece str, search::Score weight) {
  char *copied_mem = static_cast<char*>(pool_.Allocate(str.size()));
  memcpy(copied_mem, str.data(), str.size());
  std::pair<StringPiece, Value> to_ins;
  to_ins.first = StringPiece(copied_mem, str.size());
  UTIL_THROW_IF(id_.size() > std::numeric_limits<ID>::max(), WeightParseException, "Too many features.  Change the typedef for ID to uint64_t.");
  to_ins.second.id = static_cast<ID>(id_.size());
  to_ins.second.weight = weight;
  std::pair<Map::iterator, bool> ret(str_.insert(to_ins));
  UTIL_THROW_IF(!ret.second, WeightParseException, "Duplicate weight name " << str);
  id_.push_back(to_ins.first);
  return to_ins.second.id;
}

std::ostream &Weights::Write(std::ostream &to, const Vector &from) const {
  std::vector<Vector::Entry>::const_iterator i(from.values_.begin());
  if (i == from.values_.end()) return to;
  to << id_[i->id] << '=' << i->score;
  for (++i; i != from.values_.end(); ++i) 
    to << ' ' << id_[i->id] << '=' << i->score;
  return to;
}

} // namespace feature
} // namespace alone
