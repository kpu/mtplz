#ifndef ALONE_THREADING__
#define ALONE_THREADING__

#include "alone/feature_computer.hh"

#ifdef WITH_THREADS
#include "util/pcqueue.hh"
#include "util/thread_pool.hh"
#endif

#include <iosfwd>
#include <queue>
#include <string>

namespace util {
class FilePiece;
} // namespace util

namespace search {
class Config;
} // namespace search

namespace alone {

#ifdef WITH_THREADS
struct SentenceID {
  unsigned int sentence_id;
  bool operator==(const SentenceID &other) const {
    return sentence_id == other.sentence_id;
  }
};

struct Input : public SentenceID {
  util::FilePiece *file;
  static Input Poison() {
    Input ret;
    ret.sentence_id = static_cast<unsigned int>(-1);
    ret.file = NULL;
    return ret;
  }
};

struct Output : public SentenceID {
  std::string *str;
  static Output Poison() {
    Output ret;
    ret.sentence_id = static_cast<unsigned int>(-1);
    ret.str = NULL;
    return ret;
  }
};

template <class Model> class DecodeHandler {
  public:
    typedef Input Request;

    DecodeHandler(const search::Config &config, const Model &model, const feature::Weights &weights, util::PCQueue<Output> &out) :
      config_(config), model_(model), features_(weights), out_(out) {}

    void operator()(Input message);

  private:
    void Produce(unsigned int sentence_id, const std::string &str);

    const search::Config &config_;

    const Model &model_;

    feature::Computer features_;
    
    util::PCQueue<Output> &out_;
};

class PrintHandler {
  public:
    typedef Output Request;

    explicit PrintHandler(std::ostream &o) : out_(o), done_(0) {}

    void operator()(Output message);

  private:
    std::ostream &out_;
    std::deque<std::string*> waiting_;
    unsigned int done_;
};

template <class Model> class Controller {
  public:
    // This config must remain valid.   
    explicit Controller(const search::Config &config, const Model &model, const feature::Weights &weights, size_t decode_workers, std::ostream &to);

    // Takes ownership of in.    
    void Add(util::FilePiece *in) {
      Input input;
      input.sentence_id = sentence_id_++;
      input.file = in;
      decoder_.Produce(input);
    }

  private:
    unsigned int sentence_id_;

    util::ThreadPool<PrintHandler> printer_;

    util::ThreadPool<DecodeHandler<Model> > decoder_;
};
#endif

// Same API as controller.  
template <class Model> class InThread {
  public:
    InThread(const search::Config &config, const Model &model, const feature::Weights &weights, std::ostream &to) :
      config_(config), model_(model), features_(weights), to_(to), sentence_id_(0) {}

    // Takes ownership of in.  
    void Add(util::FilePiece *in);

  private:
    const search::Config &config_;

    const Model &model_;

    feature::Computer features_;

    std::ostream &to_;

    unsigned int sentence_id_;
};

} // namespace alone
#endif // ALONE_THREADING__
