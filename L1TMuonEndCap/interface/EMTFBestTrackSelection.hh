#ifndef L1TMuonEndCap_EMTFBestTrackSelection_hh
#define L1TMuonEndCap_EMTFBestTrackSelection_hh

#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFCommon.hh"


class EMTFBestTrackSelection {
public:
  void configure(
    int endcap, int sector, int bx,
    int maxRoadsPerZone, int maxTracks
  );

  void select(
      const std::vector<EMTFTrackExtraCollection>& zone_tracks,
      EMTFTrackExtraCollection& best_tracks
  ) const;

private:
  int endcap_, sector_, bx_;

  int maxRoadsPerZone_, maxTracks_;
};

#endif