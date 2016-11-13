#ifndef L1TMuonEndCap_EMTFTrackTools_hh
#define L1TMuonEndCap_EMTFTrackTools_hh

#include <cmath>

#include "DataFormatsSep2016/L1TMuon/interface/EMTFTrack.h"
#include "DataFormatsSep2016/L1TMuon/interface/EMTFTrackExtra.h"

namespace L1TMuonEndCap {

  // Please refers to DN-2015/017 for uGMT conventions

  int calc_ring(int station, int csc_ID, int strip);

  int calc_chamber(int station, int sector, int subsector, int ring, int csc_ID);

  int calc_uGMT_chamber(int csc_ID, int subsector, int neighbor, int station);

  inline double deg_to_rad(double deg) {
    constexpr double factor = M_PI/180.;
    return deg * factor;
  }

  inline double rad_to_deg(double rad) {
    constexpr double factor = 180./M_PI;
    return rad * factor;
  }

  inline double calc_theta_deg_from_int(int theta_int) {
    double theta = static_cast<double>(theta_int);
    theta = theta * (45.0-8.5)/128. + 8.5;
    return theta;
  }

  inline double calc_theta_rad_from_int(int theta_int) {
    return deg_to_rad(calc_theta_deg_from_int(theta_int));
  }

  inline double calc_phi_glob_deg(double loc, int sector) {  // sector [1-6]
    double glob = loc + 15. + (60. * (sector-1));
    glob = (glob < 180.) ? glob : glob - 360.;
    return glob;
  }

  inline double calc_phi_glob_rad(double loc, int sector) {  // sector [1-6]
    return deg_to_rad(calc_phi_glob_deg(loc, sector));
  }

  inline double calc_pt(int bits) {
    double pt = static_cast<double>(bits);
    pt = 0.5 * (pt - 1);
    return pt;
  }

  inline int    calc_pt_GMT(double val) {
    val = (val * 2) + 1;
    int gmt_pt = static_cast<int>(val);
    gmt_pt = (gmt_pt > 511) ? 511 : gmt_pt;
    return gmt_pt;
  }

  inline double calc_eta(int bits) {
    double eta = static_cast<double>(bits);
    eta *= 0.010875;
    return eta;
  }

  //inline double calc_eta_corr(int bits, int endcap) {  // endcap [1-2]
  //  bits = (endcap == 2) ? bits+1 : bits;
  //  double eta = static_cast<double>(bits);
  //  eta *= 0.010875;
  //  return eta;
  //}

  inline int    calc_eta_GMT(double val) {
    val /= 0.010875;
    int eta = static_cast<int>(val);
    return eta;
  }

  inline double calc_eta_from_theta_rad(double theta_rad) {
    double eta = -1. * std::log(std::tan(theta_rad/2.));
    return eta;
  }

  inline double calc_theta_rad(double eta) {
    double theta_rad = 2. * std::atan(std::exp(-1.*eta));
    return theta_rad;
  }

  inline double calc_theta_deg(double eta) {
    return rad_to_deg(calc_theta_rad(eta));
  }

  inline double calc_phi_loc_deg(int bits) {
    double loc = static_cast<double>(bits);
    loc = (loc/60.) - 22.;
    return loc;
  }

  inline double calc_phi_loc_rad(int bits) {
    return deg_to_rad(calc_phi_loc_deg(bits));
  }

  //inline double calc_phi_loc_deg_corr(int bits, int endcap) {  // endcap [1-2]
  //  double loc = static_cast<double>(bits);
  //  loc = (loc/60.) - 22.;
  //  loc = (endcap == 2) ? loc - (36./60.) : loc - (28./60.);
  //  return loc;
  //}

  //inline double calc_phi_loc_rad_corr(int bits, int endcap) {  // endcap [1-2]
  //  return deg_to_rad(calc_phi_loc_deg_corr(bits, endcap));
  //}

  inline int    calc_phi_loc_int(double val) {
    val = (val + 22.) * 60.;
    int phi_int = static_cast<int>(val);
    return phi_int;
  }

  inline double calc_phi_GMT_deg(int bits) {
    double phi = static_cast<double>(bits);
    phi = (phi * 360./576.) + (180./576.);
    return phi;
  }

  inline double calc_phi_GMT_deg_corr(int bits) {  // AWB mod 09.02.16
    return (bits * 0.625 * 1.0208) + 0.3125 * 1.0208 + 0.552;
  }

  inline double calc_phi_GMT_rad(int bits) {
    return deg_to_rad(calc_phi_GMT_deg(bits));
  }

  inline int    calc_phi_GMT_int(double val) {
    val = (val - 180./576.) / (360./576.);
    int phi_int = static_cast<int>(val);
    return phi_int;
  }

}  // namespace L1TMuonEndCap

#endif
