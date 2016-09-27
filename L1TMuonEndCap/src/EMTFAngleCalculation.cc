#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFAngleCalculation.hh"

#include "helper.h"  // to_hex, to_binary

namespace {
  static const int bw_fph = 13;  // bit width of ph, full precision
  static const int bw_th = 7;    // bit width of th
}


void EMTFAngleCalculation::configure(
    int verbose, int endcap, int sector, int bx,
    int thetaWindow
) {
  verbose_ = verbose;
  endcap_  = endcap;
  sector_  = sector;
  bx_      = bx;

  thetaWindow_     = thetaWindow;
}

void EMTFAngleCalculation::process(
    std::vector<EMTFTrackExtraCollection>& zone_tracks
) const {

  for (int izone = 0; izone < NUM_ZONES; ++izone) {
    EMTFTrackExtraCollection& tracks = zone_tracks.at(izone);  // pass by reference
    const int ntracks = tracks.size();

    for (int itrack = 0; itrack < ntracks; ++itrack) {
      EMTFTrackExtra& track = tracks.at(itrack);  // pass by reference

      // Calculate deltas
      calculate_angles(track);
    }
  }

  if (verbose_ > 0) {  // debug
    for (const auto& tracks : zone_tracks) {
      for (const auto& track : tracks) {
        if (track.rank) {
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

}

void EMTFAngleCalculation::calculate_angles(EMTFTrackExtra& track) const {
  // Fold track.xhits, a vector of EMTFHits, into a vector of vector of EMTFHits
  // with index [station][num]
  std::vector<EMTFHitExtraCollection> st_conv_hits;

  for (int istation = 0; istation < NUM_STATIONS; ++istation) {
    st_conv_hits.push_back(EMTFHitExtraCollection());

    for (const auto& conv_hit : track.xhits) {
      if ((conv_hit.station-1) == istation)
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
  best_dphi_sign_arr.fill(0);
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

      for (const auto& conv_hitA : conv_hitsA) {
        for (const auto& conv_hitB : conv_hitsB) {
          // Calculate theta deltas
          int thA = conv_hitA.theta_fp;
          int thB = conv_hitB.theta_fp;
          int dth = (thA > thB) ? thA - thB : thB - thA;
          int dth_sign = (thA > thB);  // sign

          if (best_dtheta_arr.at(ipair) >= dth) {
            best_dtheta_arr.at(ipair) = dth;
            best_dtheta_sign_arr.at(ipair) = dth_sign;
            best_dtheta_valid_arr.at(ipair) = true;

            best_theta_arr.at(ist1) = thA;
            best_theta_arr.at(ist2) = thB;
            best_theta_valid_arr.at(ist1) = true;
            best_theta_valid_arr.at(ist2) = true;
          }

          // Calculate phi deltas
          int phA = conv_hitA.phi_fp;
          int phB = conv_hitB.phi_fp;
          int dph = (phA > phB) ? phA - phB : phB - phA;
          int dph_sign = (phA <= phB);  // sign reversed according to Matt's oral request 2016-04-27

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
  if (best_dtheta_arr.at(0) <= thetaWindow_) {
    vmask1 |= 0b0011;  // 12
    best_dtheta_valid_arr.at(0) = false;
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
  int vstat = vmask1;
  if ((vstat & vmask2) != 0 || vstat == 0)
    vstat |= vmask2;
  if ((vstat & vmask3) != 0 || vstat == 0)
    vstat |= vmask3;

  // remove some valid flags if th did not line up
  for (int istation = 0; istation < NUM_STATIONS; ++istation) {
    if ((vstat & (1<<istation)) == 0) {  // station bit not set
      st_conv_hits.at(istation).clear();
      best_theta_valid_arr.at(istation) = false;
    }
  }

  // assign precise phi and theta
  int phi_int = 0;
  int theta_int = 0;

  if ((vstat & (1<<1)) != 0) {          // ME2 present
    assert(best_theta_valid_arr.at(1));
    phi_int   = best_phi_arr.at(1);
    theta_int = best_theta_arr.at(1);

  } else if ((vstat & (1<<2)) != 0) {   // ME3 present
    assert(best_theta_valid_arr.at(2));
    phi_int   = best_phi_arr.at(2);
    theta_int = best_theta_arr.at(2);

  } else if ((vstat & (1<<3)) != 0) {   // ME4 present
    assert(best_theta_valid_arr.at(3));
    phi_int   = best_phi_arr.at(3);
    theta_int = best_theta_arr.at(3);
  }

  // update rank taking into account available stations after theta deltas
  // keep straightness as it was
  int rank = (track.xroad.quality_code << 1);  // output rank is one bit longer than input, to accomodate ME4 separately
  int rank2 = (
      (((rank>>6)  & 1) << 6) |  // straightness
      (((rank>>4)  & 1) << 4) |  // straightness
      (((rank>>2)  & 1) << 2) |  // straightness
      (((vstat>>0) & 1) << 5) |  // ME1
      (((vstat>>1) & 1) << 3) |  // ME2
      (((vstat>>2) & 1) << 1) |  // ME3
      (((vstat>>3) & 1) << 0)    // ME4
  );

  int mode = (
      (((vstat>>0) & 1) << 3) |  // ME1
      (((vstat>>1) & 1) << 2) |  // ME2
      (((vstat>>2) & 1) << 1) |  // ME3
      (((vstat>>3) & 1) << 0)    // ME4
  );

  int mode_inv = vstat;

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
      bool isEven = (chamber%2==0);
      // odd chambers are bolted to the iron, which faces
      // forward in 1&2, backward in 3&4, so...
      result = (station<3) ? isEven : !isEven;
    }
    return result;
  };

  // ___________________________________________________________________________
  // Output

  track.rank       = rank2;
  track.mode       = mode;
  track.mode_inv   = mode_inv;
  track.phi_int    = phi_int;
  track.theta_int  = theta_int;

  // Assign ptlut_data
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
    ptlut_data.bt_chamber[i] = v.empty() ? 0 : get_bt_chamber(v.front());
  }

  track.ptlut_data = ptlut_data;

  // Reduce converted hit collection
  track.num_xhits    = 0;
  uint16_t check_num_xhits = 0;

  // Apply erase-remove idiom
  // Erase the hits that are not the "best" phi and theta in each station
  for (int i = 0, j = 0; i < NUM_STATIONS; ++i) {
    const int nchits = track.xhits.size();

    if (best_theta_valid_arr.at(i)) {
      for (int ichit = 0; ichit < nchits; ++ichit) {
        const EMTFHitExtra& conv_hit = track.xhits.at(ichit);

        if (
            ((conv_hit.station-1) == i) &&
            (conv_hit.phi_fp      == best_phi_arr.at(i)) &&
            (conv_hit.theta_fp    == best_theta_arr.at(i))
        ) {
          track.num_xhits += 1;
          track.xhits        .at(j) = std::move(track.xhits        .at(ichit));
          track.xhits_ph_diff.at(j) = std::move(track.xhits_ph_diff.at(ichit));
          ++j;
          break;
        }
      }
    }
    check_num_xhits = j;
  }
  assert(track.num_xhits == check_num_xhits);

  track.xhits        .erase(track.xhits.begin()        +track.num_xhits, track.xhits.end());
  track.xhits_ph_diff.erase(track.xhits_ph_diff.begin()+track.num_xhits, track.xhits_ph_diff.end());
}

int EMTFAngleCalculation::get_bt_chamber(const EMTFHitExtra& conv_hit) const {
  int bt_station = (conv_hit.station == 1) ? (conv_hit.subsector-1) : conv_hit.station;
  int bt_chamber = (conv_hit.cscn_ID-1);
  if (bt_chamber >= 12)
    bt_chamber -= 3;
  return (bt_station * 12) + bt_chamber;
}
