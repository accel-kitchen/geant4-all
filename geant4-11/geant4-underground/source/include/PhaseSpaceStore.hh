//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// PhaseSpaceStore.hh
// Stage B が読み込む位相空間ファイル。Stage A が出力した
// <prefix>_nt_phasespace_t*.csv をすべて読み、event ごとの組にまとめる。
// 初回の Get() で一度だけ読み込み、以後は全スレッドで共有する (読み取り専用)。
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifndef PhaseSpaceStore_h
#define PhaseSpaceStore_h 1

#include "globals.hh"
#include <vector>

struct PsRow {
  G4int    pdg;
  G4double ekin;           // MeV
  G4double x, y, z;        // mm
  G4double ux, uy, uz;
  G4double t;              // ns
};

struct PsGroup {
  G4int    srcEvent;
  G4double w1, E0, cth0, ph0;
  std::vector<PsRow> rows;
};

class PhaseSpaceStore {
public:
  static const PhaseSpaceStore& Get();
  const std::vector<PsGroup>& Groups() const { return fGroups; }
  G4long SrcGenerated() const { return fSrcGenerated; }  // Stage A で発生させたイベント数

private:
  PhaseSpaceStore();
  std::vector<PsGroup> fGroups;
  G4long fSrcGenerated = 0;
};

#endif
