#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFPtLUTReader.hh"

#include <fstream>
#include <iostream>
#include <stdexcept>

#define PTLUT_SIZE (1<<30)

EMTFPtLUTReader::EMTFPtLUTReader() :
    ptlut_(),
    version_(4),
    ok_(false)
{

}

EMTFPtLUTReader::~EMTFPtLUTReader() {

}

void EMTFPtLUTReader::read(const std::string& lut_full_path) {
  if (ok_)  return;

  std::cout << "Loading LUT, this might take a while..." << std::endl;

  std::ifstream infile(lut_full_path, std::ios::binary);
  if (!infile.good()) {
    char what[256];
    snprintf(what, sizeof(what), "Fail to open %s", lut_full_path.c_str());
    throw std::invalid_argument(what);
  }

  ptlut_.reserve(PTLUT_SIZE);

  typedef uint64_t full_word_t;
  full_word_t full_word;
  full_word_t sub_word[4] = {0, 0, 0, 0};

  while (infile.read(reinterpret_cast<char*>(&full_word), sizeof(full_word_t))) {
    sub_word[0] = (full_word>>0)      & 0x1FF;  // 9-bit
    sub_word[1] = (full_word>>9)      & 0x1FF;
    sub_word[2] = (full_word>>32)     & 0x1FF;
    sub_word[3] = (full_word>>(32+9)) & 0x1FF;

    ptlut_.push_back(sub_word[0]);
    ptlut_.push_back(sub_word[1]);
    ptlut_.push_back(sub_word[2]);
    ptlut_.push_back(sub_word[3]);
  }
  infile.close();

  if (ptlut_.size() != PTLUT_SIZE) {
    char what[256];
    snprintf(what, sizeof(what), "ptlut_.size() is %lu != %i", ptlut_.size(), PTLUT_SIZE);
    throw std::invalid_argument(what);
  }

  version_ = ptlut_.at(0);  // address 0 is the pT LUT version number
  ok_ = true;
  return;
}

EMTFPtLUTReader::content_t EMTFPtLUTReader::lookup(const address_t& address) const {
  return ptlut_.at(address);
}
