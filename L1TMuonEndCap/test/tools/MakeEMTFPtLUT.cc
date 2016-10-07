#include <memory>
#include <iostream>

#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDAnalyzer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFPtAssignmentEngine.hh"
#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFPtLUTWriter.hh"

#include "helper.hh"


class MakeEMTFPtLUT : public edm::EDAnalyzer {
public:
  explicit MakeEMTFPtLUT(const edm::ParameterSet&);
  virtual ~MakeEMTFPtLUT();

private:
  //virtual void beginJob();
  //virtual void endJob();

  //virtual void beginRun(const edm::Run&, const edm::EventSetup&);
  //virtual void endRun(const edm::Run&, const edm::EventSetup&);

  virtual void analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup);

  void makeLUT();

private:
  std::unique_ptr<EMTFPtAssignmentEngine> pt_assign_engine_;

  EMTFPtLUTWriter ptlut_writer_;

  const edm::ParameterSet config_;

  int verbose_;

  std::string outfile_;

  bool done_;
};

// _____________________________________________________________________________
#define PTLUT_SIZE (1<<30)

MakeEMTFPtLUT::MakeEMTFPtLUT(const edm::ParameterSet& iConfig) :
    pt_assign_engine_(new EMTFPtAssignmentEngine()),
    ptlut_writer_(),
    config_(iConfig),
    verbose_(iConfig.getUntrackedParameter<int>("verbosity")),
    outfile_(iConfig.getParameter<std::string>("outfile")),
    done_(false)
{
  const edm::ParameterSet spPAParams16 = config_.getParameter<edm::ParameterSet>("spPAParams16");
  auto bdtXMLDir          = spPAParams16.getParameter<std::string>("BDTXMLDir");
  auto readPtLUTFile      = spPAParams16.getParameter<bool>("ReadPtLUTFile");
  auto fixMode15HighPt    = spPAParams16.getParameter<bool>("FixMode15HighPt");
  auto fix9bDPhi          = spPAParams16.getParameter<bool>("Fix9bDPhi");

  pt_assign_engine_->read(bdtXMLDir);
  pt_assign_engine_->configure(
    verbose_,
    readPtLUTFile, fixMode15HighPt, fix9bDPhi
  );
}

MakeEMTFPtLUT::~MakeEMTFPtLUT() {}

void MakeEMTFPtLUT::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) {
  if (done_)  return;

  makeLUT();

  done_ = true;
  return;
}

void MakeEMTFPtLUT::makeLUT() {
  std::cout << "Calculating pT for " << PTLUT_SIZE << " addresses, please sit tight..." << std::endl;

  EMTFPtLUTWriter::address_t address = 0;
  EMTFPtLUTWriter::content_t pt_value = 0;

  float xmlpt = 0.;
  float pt = 0.;
  int gmt_pt = 0;

  for (; address<PTLUT_SIZE; ++address) {
    //int mode_inv = (address >> (30-4)) & ((1<<4)-1);

    show_progress_bar(address, PTLUT_SIZE);

    // floats
    xmlpt = pt_assign_engine_->calculate_pt_xml(address);
    pt    = xmlpt * 1.4;

    // integers
    gmt_pt = (pt * 2) + 1;
    gmt_pt = (gmt_pt > 511) ? 511 : gmt_pt;

    pt_value = gmt_pt;
    ptlut_writer_.push_back(pt_value);
  }

  ptlut_writer_.write(outfile_);
}

// DEFINE THIS AS A PLUG-IN
#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(MakeEMTFPtLUT);