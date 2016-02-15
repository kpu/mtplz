#include <sys/stat.h>
#include "line_splitter.hh"
#include "storing.hh"
#include "StoreTarget.h"

#include "util/file_piece.hh"

namespace ProbingPT {

void WriteTableEntry(Table &table, StringPiece source, uint64_t targetInd) {
  //Create an entry for the previous source phrase:
  Entry pesho;
  //The key is the sum of hashes of individual words bitshifted by their position in the phrase.
  //Probably not entirerly correct, but fast and seems to work fine in practise.
  pesho.key = 1;
  for (util::TokenIter<util::SingleCharacter> it(source, util::SingleCharacter(' ')); it; ++it) {
    pesho.key = ChainHash(pesho.key, HashWord(*it));
  }
  pesho.value = targetInd;
  //Put into table
  table.Insert(pesho);
}

void createProbingPT(
    const std::string &phrasetable_path,
    const std::string &basepath,
    int num_scores,
    int num_lex_scores,
    bool log_prob,
    int max_cache_size) {
  std::cerr << "Starting..." << std::endl;

  //Get basepath and create directory if missing
  mkdir(basepath.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

  StoreTarget storeTarget(basepath);

  //Get uniq lines:
  unsigned long uniq_entries = countUniqueSource(phrasetable_path);

  //Source phrase vocabids
  std::map<uint64_t, std::string> source_vocabids;

  //Read the file
  util::FilePiece filein(phrasetable_path.c_str());

  //Init the probing hash table
  util::scoped_memory mem(Table::Size(uniq_entries, 1.2), true);
  Table table(mem.get(), mem.size());

  std::priority_queue<CacheItem*, std::vector<CacheItem*>, CacheItemOrderer> cache;
  float totalSourceCount = 0;

  //Keep track of the size of each group of target phrases
  size_t line_num = 0;

  //Read everything and processs
  std::string prevSource;
  while(true) {
    try {
      //Process line read
      line_text line;
      line = splitLine(filein.ReadLine());

      ++line_num;
      if (line_num % 1000000 == 0) {
        std::cerr << line_num << " " << std::flush;
      }

      //Add source phrases to vocabularyIDs
      add_to_map(&source_vocabids, line.source_phrase);

      if (prevSource.empty()) {
        // 1st line
        prevSource = line.source_phrase.as_string();
        storeTarget.Append(line, log_prob);
      } else if (prevSource == line.source_phrase) {
        //If we still have the same line, just append to it:
        storeTarget.Append(line, log_prob);
      } else {
        assert(prevSource != line.source_phrase);

        //Create a new entry even

        // save
        uint64_t targetInd = storeTarget.Save();

        // next line
        storeTarget.Append(line, log_prob);

        WriteTableEntry(table, prevSource, targetInd);

        // update cache
        if (max_cache_size) {
          StringPiece countStr(util::Trim(line.counts));
          if (!countStr.empty()) {
            std::vector<float> toks = Tokenize<float>(countStr);

            if (toks.size() >= 2) {
              totalSourceCount += toks[1];
              CacheItem *item = new CacheItem(util::Trim(line.source_phrase), toks[1]);
              cache.push(item);

              if (max_cache_size > 0 && cache.size() > max_cache_size) {
                cache.pop();
              }
            }
          }
        }

        //Set prevLine
        prevSource = line.source_phrase.as_string();

      }

    } catch (util::EndOfFileException e) {
      std::cerr << "Reading phrase table finished, writing remaining files to disk." << std::endl;

      //After the final entry is constructed we need to add it to the phrase_table
      //Create an entry for the previous source phrase:
      uint64_t targetInd = storeTarget.Save();

      WriteTableEntry(table, prevSource, targetInd);
      break;
    }
  }

  serialize_table(mem, size, (basepath + "/probing_hash.dat"));

  serialize_map(&source_vocabids, (basepath + "/source_vocabids"));

  serialize_cache(cache, (basepath + "/cache"), totalSourceCount);

  //Write configfile
  std::ofstream configfile;
  configfile.open((basepath + "/config").c_str());
  configfile << API_VERSION << '\n';
  configfile << uniq_entries << '\n';
  configfile << num_scores << '\n';
  configfile << num_lex_scores << '\n';
  configfile << log_prob << '\n';
  configfile.close();
}

// How many unique source phrases are in the text phrase table?
std::size_t countUniqueSource(const char *path) {
  std::string previous_source;
  StringPiece line;
  size_t ret = 0;
  for (util::FilePiece f(path); f.ReadLineOrEOF(line);) {
    StringPiece source(line.substr(0, line.find("|||")));
    if (previous_source != source)
      ++ret;
    previous_source.assign(source.data(), source.size());
  }
  return ret;
}

void serialize_cache(std::priority_queue<CacheItem*, std::vector<CacheItem*>, CacheItemOrderer> &cache,
    const std::string &path,
    float totalSourceCount)
{
  std::vector<const CacheItem*> vec(cache.size());

  size_t ind = cache.size() - 1;
  while (!cache.empty()) {
    const CacheItem *item = cache.top();
    vec[ind] = item;
    cache.pop();
    --ind;
  }

  std::ofstream os (path.c_str());

  os << totalSourceCount << std::endl;
  for (size_t i = 0; i < vec.size(); ++i) {
    const CacheItem *item = vec[i];
    os << item->count << "\t" << item->source << std::endl;
    delete item;
  }

  os.close();
}

}

