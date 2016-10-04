#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFAngleCalculation.hh"

#include "helper.h"  // to_hex, to_binary

namespace {
  static const int bw_fph = 13;  // bit width of ph, full precision
  static const int bw_th = 7;    // bit width of th
}


void EMTFAngleCalculation::configure(
    int verbose, int endcap, int sector, int bx,
    int bxWindow, int thetaWindow
) {
  verbose_ = verbose;
  endcap_  = endcap;
  sector_  = sector;
  bx_      = bx;

  bxWindow_    = bxWindow;
  thetaWindow_ = thetaWindow;
}

void EMTFAngleCalculation::process(
    std::vector<EMTFTrackExtraCollection>& zone_tracks
) const {

  for (int izone = 0; izone < NUM_ZONES; ++izone) {
    EMTFTrackExtraCollection& tracks = zone_tracks.at(izone);  // pass by reference

    for (unsigned itrack = 0; itrack < tracks.size(); ++itrack) {
      // Calculate deltas
      EMTFTrackExtra& track = tracks.at(itrack);  // pass by reference
      calculate_angles(track);
    }

    // Erase tracks with rank = 0, and hits that fail dTheta window (or are not "best" dTheta)
    erase_tracks(tracks);

    for (unsigned itrack = 0; itrack < tracks.size(); ++itrack) {
      // Calculate bx
      EMTFTrackExtra& track = tracks.at(itrack);  // pass by reference
      calculate_bx(track);
    }
  }

  if (verbose_ > 0) {  // debug
    for (const auto& tracks : zone_tracks) {
      for (const auto& track : tracks) {
        std::cout << "deltas: z: " << track.xroad.zone << " pat: " << track.xroad.winner << " rank: " << to_hex(track.rank)
            << " delta_ph: " << array_as_string(track.ptlut_data.delta_ph)
            << " delta_th: " << array_as_string(track.ptlut_data.delta_th)
            << " sign_ph: " << array_as_string(track.ptlut_data.sign_ph)
            << " sign_th: " << array_as_string(track.ptlut_data.sign_th)
            << " phi: " << track.phi_int << " theta: " << track.theta_int
            << std::endl;
      }
    }
  }

}

void EMTFAngleCalculation::calculate_angles(EMTFTrackExtra& track) const {
  // Fold track.xhits, a vector of EMTFHits, into a vector of vector of EMTFHits
  // with index [station][num]
  std::vector<EMTFHitExtraCollection> st_conv_hits;

  for (int istation = 0; istation < NUM_STATIONS; ++istation) {
    st_conv_hits.push_back(EMTFHitExtraCollection());

    for (const auto& conv_hit : track.xhits) {
      if ((conv_hit.station - 1) == istation)
        st_conv_hits.back().push_back(conv_hit);
    }
  }
  assert(st_conv_hits.size() == NUM_STATIONS);

  const int invalid_dtheta = (1<<bw_th) - 1;  // = 127
  const int invalid_dphi = (1<<bw_fph) - 1;   // = 8191

  // Best theta deltas and phi deltas
  // from 0 to 5: dphi12, dphi13, dphi14, dphi23, dphi24, dphi34
  std::array<int,  NUM_STATION_PAIRS> best_dtheta_arr;
  std::array<int,  NUM_STATION_PAIRS> best_dtheta_sign_arr;
  std::array<int,  NUM_STATION_PAIRS> best_dphi_arr;
  std::array<int,  NUM_STATION_PAIRS> best_dphi_sign_arr;
  std::array<bool, NUM_STATION_PAIRS> best_dtheta_valid_arr;

  best_dtheta_arr.fill(invalid_dtheta);
  best_dtheta_sign_arr.fill(0);
  best_dphi_arr.fill(invalid_dphi);
  best_dphi_sign_arr.fill(1);  // dphi sign reversed compared to dtheta
  best_dtheta_valid_arr.fill(false);

  // For phi and theta assignment
  std::array<int,  NUM_STATIONS> best_theta_arr;
  std::array<int,  NUM_STATIONS> best_phi_arr;
  std::array<bool, NUM_STATIONS> best_theta_valid_arr;

  best_phi_arr.fill(0);
  best_theta_arr.fill(0);
  best_theta_valid_arr.fill(false);

  // Calculate angles
  int ipair = 0;

  for (int ist1 = 0; ist1 < NUM_STATIONS-1; ++ist1) {  // station A
    for (int ist2 = ist1+1; ist2 < NUM_STATIONS; ++ist2) {  // station B
      const EMTFHitExtraCollection& conv_hitsA = st_conv_hits.at(ist1);
      const EMTFHitExtraCollection& conv_hitsB = st_conv_hits.at(ist2);

      for (const auto& conv_hitA : conv_hitsA) { // When would we have more than 1 hit per station? - AWB 03.10.16
        for (const auto& conv_hitB : conv_hitsB) {
          // Calculate theta deltas
          int thA = conv_hitA.theta_fp;
          int thB = conv_hitB.theta_fp;
          int dth = (thA > thB) ? thA - thB : thB - thA;
          int dth_sign = (thA > thB);  // sign
          assert(dth < invalid_dtheta);

          if (best_dtheta_arr.at(ipair) >= dth) {
            best_dtheta_arr.at(ipair) = dth;
            best_dtheta_sign_arr.at(ipair) = dth_sign;
            best_dtheta_valid_arr.at(ipair) = true; // When is this condition not fulfilled? - AWB 03.10.16

            best_theta_arr.at(ist1) = thA; // What if the station 2 theta for the "best" 1-2 pair does not match the best 2-3 pair? - AWB 03.10.16
            best_theta_arr.at(ist2) = thB;
            best_theta_valid_arr.at(ist1) = true;
            best_theta_valid_arr.at(ist2) = true;
          }

          // Calculate phi deltas
          int phA = conv_hitA.phi_fp;
          int phB = conv_hitB.phi_fp;
          int dph = (phA > phB) ? phA - phB : phB - phA;
          int dph_sign = (phA <= phB);  // sign reversed according to Matt's oral request 2016-04-27 (Why??? - AWB 03.10.16)

          if (best_dtheta_valid_arr.at(ipair)) {
            best_dphi_arr.at(ipair) = dph;
            best_dphi_sign_arr.at(ipair) = dph_sign;

            best_phi_arr.at(ist1) = phA;
            best_phi_arr.at(ist2) = phB;
          }
        }  // end loop over conv_hits in station B
      }  // end loop over conv_hits in station A
      ++ipair;
    }  // end loop over station B
  }  // end loop over station A


  // Find valid segments
  int vmask1 = 0;
  int vmask2 = 0;
  int vmask3 = 0;

  // vmask contains valid station mask = {ME4,ME3,ME2,ME1}
  // Why does vmask contain "0b0000"? - AWB 03.10.16
  if (best_dtheta_arr.at(0) <= thetaWindow_) {
    vmask1 |= 0b0011;  // 12
    best_dtheta_valid_arr.at(0) = false; // Why does a match make this IN-valid? - AWB 03.10.16
  }
  if (best_dtheta_arr.at(1) <= thetaWindow_) {
    vmask1 |= 0b0101;  // 13
    best_dtheta_valid_arr.at(1) = false;
  }
  if (best_dtheta_arr.at(2) <= thetaWindow_) {
    vmask1 |= 0b1001;  // 14
    best_dtheta_valid_arr.at(2) = false;
  }
  if (best_dtheta_arr.at(3) <= thetaWindow_) {
    vmask2 |= 0b0110;  // 23
    best_dtheta_valid_arr.at(3) = false;
  }
  if (best_dtheta_arr.at(4) <= thetaWindow_) {
    vmask2 |= 0b1010;  // 24
    best_dtheta_valid_arr.at(4) = false;
  }
  if (best_dtheta_arr.at(5) <= thetaWindow_) {
    vmask3 |= 0b1100;  // 34
    best_dtheta_valid_arr.at(5) = false;
  }

  // merge station masks only if they share bits
  // (How can they not?  All vmasks contin "0b0000" - AWB 03.10.16)
  int vstat = vmask1;
  if ((vstat & vmask2) != 0 || vstat == 0)
    vstat |= vmask2;
  if ((vstat & vmask3) != 0 || vstat == 0)
    vstat |= vmask3;

  // remove valid flag for station if hit has no dTheta values within the window
  for (int istation = 0; istation < NUM_STATIONS; ++istation) {
    if ((vstat & (1 << istation)) == 0) {  // station bit not set
      st_conv_hits.at(istation).clear();
      best_theta_valid_arr.at(istation) = false;
    }
  }

  // assign precise phi and theta for the track
  int phi_int = 0;
  int theta_int = 0;

  if ((vstat & (1 << 1)) != 0) {          // ME2 present
    assert(best_theta_valid_arr.at(1));
    phi_int   = best_phi_arr.at(1);
    theta_int = best_theta_arr.at(1);

  } else if ((vstat & (1 << 2)) != 0) {   // ME3 present
    assert(best_theta_valid_arr.at(2));
    phi_int   = best_phi_arr.at(2);
    theta_int = best_theta_arr.at(2);

  } else if ((vstat & (1 << 3)) != 0) {   // ME4 present
    assert(best_theta_valid_arr.at(3));
    phi_int   = best_phi_arr.at(3);
    theta_int = best_theta_arr.at(3);
  }

  // update rank taking into account available stations after theta deltas
  // keep straightness as it was
  int rank = (track.xroad.quality_code << 1);  // output rank is one bit longer than input, to accomodate ME4 separately
  int rank2 = (
      (((rank >> 6)  & 1) << 6) |  // straightness
      (((rank >> 4)  & 1) << 4) |  // straightness
      (((rank >> 2)  & 1) << 2) |  // straightness
      (((vstat >> 0) & 1) << 5) |  // ME1
      (((vstat >> 1) & 1) << 3) |  // ME2
      (((vstat >> 2) & 1) << 1) |  // ME3
      (((vstat >> 3) & 1) << 0)    // ME4
  );

  int mode = (
      (((vstat >> 0) & 1) << 3) |  // ME1
      (((vstat >> 1) & 1) << 2) |  // ME2
      (((vstat >> 2) & 1) << 1) |  // ME3
      (((vstat >> 3) & 1) << 0)    // ME4
  );

  int mode_inv = vstat; // With the "0b0000" componenent? - AWB 03.10.16

  // if less than 2 segments, kill rank
  if (vstat == 0b0001 || vstat == 0b0010 || vstat == 0b0100 || vstat == 0b1000 || vstat == 0)
    rank2 = 0;

  // from RecoMuon/DetLayers/src/MuonCSCDetLayerGeometryBuilder.cc
  auto isFront = [](int station, int ring, int chamber) {
    bool result = false;
    bool isOverlapping = !(station == 1 && ring == 3);
    // not overlapping means back
    if(isOverlapping)
    {
      bool isEven = (chamber % 2 == 0);
      // odd chambers are bolted to the iron, which faces
      // forward in 1&2, backward in 3&4, so...
      result = (station < 3) ? isEven : !isEven;
    }
    return result;
  };

  // Fill ptlut_data
  EMTFPtLUTData ptlut_data;
  for (int i = 0; i < NUM_STATION_PAIRS; ++i) {
    ptlut_data.delta_ph[i] = best_dphi_arr.at(i);
    ptlut_data.delta_th[i] = best_dtheta_arr.at(i);
    ptlut_data.sign_ph[i]  = best_dphi_sign_arr.at(i);
    ptlut_data.sign_th[i]  = best_dtheta_sign_arr.at(i);
  }
  for (int i = 0; i < NUM_STATIONS; ++i) {
    const auto& v = st_conv_hits.at(i);
    ptlut_data.cpattern[i]   = v.empty() ? 0 : v.front().pattern;
    ptlut_data.fr[i]         = v.empty() ? 0 : isFront(v.front().station, v.front().ring, v.front().chamber);
    ptlut_data.ph[i]         = best_phi_arr.at(i);
    ptlut_data.th[i]         = best_theta_arr.at(i);
    ptlut_data.bt_chamber[i] = v.empty() ? 0 : get_bt_chamber(v.front()); // What is this variable for? - AWB 03.10.16
  }

  // ___________________________________________________________________________
  // Output

  track.rank       = rank2;
  track.mode       = mode;
  track.mode_inv   = mode_inv;
  track.phi_int    = phi_int;
  track.theta_int  = theta_int;
  track.ptlut_data = ptlut_data;
}

void EMTFAngleCalculation::calculate_bx(EMTFTrackExtra& track) const {

  int delayBX = bxWindow_ - 1;
  assert(delayBX > 0);
  int hbx[delayBX];
  for (int i = delayBX; i >= 0; i--) 
    hbx[i] = 0;

  for (const auto& conv_hit : track.xhits) {
    for (int i = delayBX; i >= 0; i--) {
      if (conv_hit.bx <= bx_ - i)
	hbx[i] += 1;  // Count stubs delayed by i BX or more
    }
  }

  int first_bx = bx_ - delayBX; // Is this always true? - AWB 04.10.16

  int second_bx = 99;
  for (int i = delayBX; i >= 0; i--) {
    if (hbx[i] >= 2) { // If 2 or more stubs are delayed by i BX or more
      second_bx = bx_ - i; // if i == delayBX, analyze immediately
      break;
    }
  }
  assert(second_bx != 99);

  // ___________________________________________________________________________
  // Output

  track.first_bx  = first_bx;
  track.second_bx = second_bx;
}

void EMTFAngleCalculation::erase_tracks(EMTFTrackExtraCollection& tracks) const {
  // Erase tracks with rank == 0
  // using erase-remove idiom
  struct {
    typedef EMTFTrackExtra value_type;
    constexpr bool operator()(const value_type& x) {
      return (x.rank == 0);
    }
  } rank_zero_pred;

  tracks.erase(std::remove_if(tracks.begin(), tracks.end(), rank_zero_pred), tracks.end());

  // Erase hits that are not selected as the best phi and theta in each station
  // using erase-remove idiom
  struct {
    typedef EMTFHitExtra value_type;
    bool operator()(const value_type& x) {
      int istation = (x.station-1);
      bool match = (
        (stations.at(istation) == true) &&
        (x.pattern  == ptlut_data.cpattern[istation]) &&
        (x.phi_fp   == ptlut_data.ph[istation]) &&
        (x.theta_fp == ptlut_data.th[istation])
      );
      if (match)
        stations.at(istation) = false;
      return match;
    }
    EMTFPtLUTData ptlut_data;
    std::array<bool, NUM_STATIONS> stations;  // keep track of which station is empty
  } selected_hit_pred;

  for (unsigned itrack = 0; itrack < tracks.size(); ++itrack) {
    EMTFTrackExtra& track = tracks.at(itrack);  // pass by reference
    selected_hit_pred.ptlut_data = track.ptlut_data;  // capture
    selected_hit_pred.stations.fill(true);

    // begin mocked std::remove_if() to remove pairs of objects
    typedef std::vector<EMTFHitExtra>::iterator hit_iter_t;
    typedef std::vector<uint16_t>::iterator     uint_iter_t;

    hit_iter_t  first   = track.xhits.begin();
    hit_iter_t  last    = track.xhits.end();
    hit_iter_t  result  = first;
    uint_iter_t first2  = track.xhits_ph_diff.begin();
    uint_iter_t result2 = first2;

    for (; first != last; ++first, ++first2) {
      if (selected_hit_pred(*first)) {
        *result = std::move(*first);
        ++result;
        *result2 = std::move(*first2);
        ++result2;
      }
    }
    // end mocked std::remove_if()

    track.xhits.erase(result, track.xhits.end());
    track.xhits_ph_diff.erase(result2, track.xhits_ph_diff.end());

    track.num_xhits = track.xhits.size();
    assert(track.num_xhits <= NUM_STATIONS);
  }  // end loop over tracks

}

int EMTFAngleCalculation::get_bt_chamber(const EMTFHitExtra& conv_hit) const {
  int bt_station = (conv_hit.station == 1) ? (conv_hit.subsector-1) : conv_hit.station;
  int bt_chamber = (conv_hit.cscn_ID-1);
  if (bt_chamber >= 12)
    bt_chamber -= 3;
  return (bt_station * 12) + bt_chamber;
}
