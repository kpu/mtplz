#include "alone/features.hh"
#include "util/file_piece.hh"
#include "util/tokenize_piece.hh"

#include <boost/lexical_cast.hpp>

int main(int argc, char *argv[]) {
  using namespace alone;
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " weights <nbest" << std::endl;
    return 1;
  }
  feature::Weights weights;
  weights.AppendFromFile(argv[1]);
  util::FilePiece f(0);
  while (true) {
    feature::Vector ignored;
    StringPiece str;
    try {
      str = f.ReadLine();
    } catch (const util::EndOfFileException &e) { break; }
    util::TokenIter<util::MultiCharacter, false> fields(str, "|||");
    ++fields; // Skip sentence number
    search::Score dot = weights.Parse(*++fields, ignored);
    search::Score given = boost::lexical_cast<search::Score>(*util::TokenIter<util::SingleCharacter, true>(*++fields, ' '));
    if (fabs(dot - given) > 0.005) {
      std::cout << "Fail: dot = " << dot << " while n-best says " << given << std::endl;
    }
  }
}
