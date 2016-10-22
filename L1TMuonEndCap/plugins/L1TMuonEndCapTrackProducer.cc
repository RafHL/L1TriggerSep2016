#include "L1TMuonEndCapTrackProducer.h"


L1TMuonEndCapTrackProducerSep2016::L1TMuonEndCapTrackProducerSep2016(const edm::ParameterSet& iConfig) :
    track_finder_(new EMTFTrackFinder(iConfig, consumesCollector())),
    track_adaptor_(new EMTFTrackAdaptor()),
    uGMT_converter_(new EMTFMicroGMTConverter()),
    config_(iConfig)
{
  // Make output products
  produces<EMTFHitExtraCollection>           ("");      // Same as EMTFHit, but with extra emulator-only variables
  produces<EMTFTrackExtraCollection>         ("");      // Same as EMTFTrack, but with extra emulator-only variables
  produces<l1t::RegionalMuonCandBxCollection>("EMTF");  // EMTF tracks output to uGMT

  produces<EMTFHitCollection>                ("");      // All CSC LCTs and RPC clusters received by EMTF

  produces<EMTFTrackCollection>              ("");      // All output EMTF tracks, in same format as unpacked data
}

L1TMuonEndCapTrackProducerSep2016::~L1TMuonEndCapTrackProducerSep2016() {

}

void L1TMuonEndCapTrackProducerSep2016::produce(edm::Event& iEvent, const edm::EventSetup& iSetup) {
  // Create pointers to output products
  auto out_xhits   = std::make_unique<EMTFHitExtraCollection>();
  auto out_xtracks = std::make_unique<EMTFTrackExtraCollection>();
  auto out_cands   = std::make_unique<l1t::RegionalMuonCandBxCollection>();

  auto out_hits    = std::make_unique<EMTFHitCollection>();
  auto out_tracks  = std::make_unique<EMTFTrackCollection>();

  // Main EMTF emulator process, produces tracks from hits in each sector in each event
  track_finder_->process(iEvent, iSetup, *out_xhits, *out_xtracks);

  // Convert into unpacker EMTFHit, EMTFTrack formats
  track_adaptor_->convert_all(*out_xhits, *out_xtracks, *out_hits, *out_tracks);

  // Convert into uGMT format
  uGMT_converter_->convert_all(*out_xtracks, *out_cands);

  // Fill the output products
  iEvent.put(std::move(out_xhits)  , "");
  iEvent.put(std::move(out_xtracks), "");
  iEvent.put(std::move(out_cands)  , "EMTF");

  iEvent.put(std::move(out_hits)   , "");
  iEvent.put(std::move(out_tracks) , "");
}

void L1TMuonEndCapTrackProducerSep2016::beginJob() {

}

void L1TMuonEndCapTrackProducerSep2016::endJob() {

}

// Fill 'descriptions' with the allowed parameters
void L1TMuonEndCapTrackProducerSep2016::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  // The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

// Define this as a plug-in
DEFINE_FWK_MODULE(L1TMuonEndCapTrackProducerSep2016);
