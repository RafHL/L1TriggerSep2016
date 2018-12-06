#ifndef L1TMuonEndCap_PrimitiveSelection_h
#define L1TMuonEndCap_PrimitiveSelection_h

#include "L1Trigger/L1TMuonEndCap/interface/Common.h"


class PrimitiveSelection {
public:
  void configure(
      int verbose, int endcap, int sector, int bx,
      int bxShiftCSC, int bxShiftRPC, int bxShiftGEM,
      bool includeNeighbor, bool duplicateTheta,
      bool bugME11Dupes
  );

  template<typename T>
  void process(
      T tag,
      const TriggerPrimitiveCollection& muon_primitives,
      std::map<int, TriggerPrimitiveCollection>& selected_prim_map
  ) const;

  // Put the hits from DT, CSC, RPC, GEM, ME0 together in one collection
  void merge(
      const std::map<int, TriggerPrimitiveCollection>& selected_dt_map,
      const std::map<int, TriggerPrimitiveCollection>& selected_csc_map,
      const std::map<int, TriggerPrimitiveCollection>& selected_rpc_map,
      const std::map<int, TriggerPrimitiveCollection>& selected_gem_map,
      const std::map<int, TriggerPrimitiveCollection>& selected_me0_map,
      std::map<int, TriggerPrimitiveCollection>& selected_prim_map
  ) const;

  // Like merge(), but keep all the hits
  void merge_no_truncate(
      const std::map<int, TriggerPrimitiveCollection>& selected_dt_map,
      const std::map<int, TriggerPrimitiveCollection>& selected_csc_map,
      const std::map<int, TriggerPrimitiveCollection>& selected_rpc_map,
      const std::map<int, TriggerPrimitiveCollection>& selected_gem_map,
      const std::map<int, TriggerPrimitiveCollection>& selected_me0_map,
      std::map<int, TriggerPrimitiveCollection>& selected_prim_map
  ) const;

  // ___________________________________________________________________________
  // CSC functions
  // If selected, return an index 0-53, else return -1
  // The index 0-53 roughly corresponds to an input link. It maps to the
  // 2D index [station][chamber] used in the firmware, with size [5:0][8:0].
  // Station 5 = neighbor sector, all stations.
  int select_csc(const TriggerPrimitive& muon_primitive) const;

  bool is_in_sector_csc(int tp_endcap, int tp_sector) const;

  bool is_in_neighbor_sector_csc(int tp_endcap, int tp_sector, int tp_subsector, int tp_station, int tp_csc_ID) const;

  bool is_in_bx_csc(int tp_bx) const;

  int get_index_csc(int tp_subsector, int tp_station, int tp_csc_ID, bool is_neighbor) const;

  // RPC functions
  // If selected, return an index 0-41, else return -1
  // The index 0-41 corresponds to CPPF link x chamber. Each CPPF link corresponds
  // to a RPC subsector (6+1 incl. neighbor), and carries data from 6 RPC rings
  // (RE1/2, RE2/2, RE3/2, RE3/3, RE4/2, RE4/3). The index maps to the 2D index
  // [subsector][chamber] used in the firmware, with size [6:0][5:0].
  int select_rpc(const TriggerPrimitive& muon_primitive) const;

  bool is_in_sector_rpc(int tp_endcap, int tp_station, int tp_ring, int tp_sector, int tp_subsector) const;

  bool is_in_neighbor_sector_rpc(int tp_endcap, int tp_station, int tp_ring, int tp_sector, int tp_subsector) const;

  bool is_in_bx_rpc(int tp_bx) const;

  int get_index_rpc(int tp_station, int tp_ring, int tp_subsector, bool is_neighbor) const;

  // GEM functions
  int select_gem(const TriggerPrimitive& muon_primitive) const;

  bool is_in_sector_gem(int tp_endcap, int tp_sector) const;

  bool is_in_neighbor_sector_gem(int tp_endcap, int tp_sector, int tp_subsector, int tp_station, int tp_csc_ID) const;

  bool is_in_bx_gem(int tp_bx) const;

  int get_index_gem(int tp_subsector, int tp_station, int tp_csc_ID, bool is_neighbor) const;

  // ME0 functions
  int select_me0(const TriggerPrimitive& muon_primitive) const;

  bool is_in_sector_me0(int tp_endcap, int tp_sector) const;

  bool is_in_neighbor_sector_me0(int tp_endcap, int tp_sector, int tp_csc_ID) const;

  bool is_in_bx_me0(int tp_bx) const;

  int get_index_me0(int tp_station, int tp_csc_ID, bool is_neighbor) const;

  // DT functions
  int select_dt(const TriggerPrimitive& muon_primitive) const;

  bool is_in_sector_dt(int tp_endcap, int tp_sector) const;

  bool is_in_neighbor_sector_dt(int tp_endcap, int tp_sector, int tp_csc_ID) const;

  bool is_in_bx_dt(int tp_bx) const;

  int get_index_dt(int tp_station, int tp_csc_ID, bool is_neighbor) const;


private:
  int verbose_, endcap_, sector_, bx_;

  int bxShiftCSC_, bxShiftRPC_, bxShiftGEM_;

  bool includeNeighbor_, duplicateTheta_;

  bool bugME11Dupes_;
};

#endif
