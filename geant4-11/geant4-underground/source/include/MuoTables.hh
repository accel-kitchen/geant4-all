//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// MuoTables.hh
// 出力表 (CSV) の列定義。作成と書き込みの列順をここに集める。
//   phasespace: Stage A の出力。検出器直上の面を通った粒子 (長さ mm, エネルギー MeV, 時間 ns)
//   hits      : ピクセルのヒット (edep MeV, t ns)。E0 は発生時のエネルギー [GeV]
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifndef MuoTables_h
#define MuoTables_h 1

#include "G4AnalysisManager.hh"
#include "globals.hh"

class MuoEventInfo;

namespace MuoTables {

// ntuple の番号は 0 だけを使う (stage によって phasespace か hits のどちらか)
void Create(G4AnalysisManager* am, const G4String& tableName);

void FillHit(G4AnalysisManager* am, const MuoEventInfo& info,
             G4int layer, G4int ix, G4int iy, G4double edep, G4double t);

void FillPhaseSpace(G4AnalysisManager* am, const MuoEventInfo& info,
                    G4int pdg, G4double ekin, const G4ThreeVector& pos,
                    const G4ThreeVector& dir, G4double t, G4int trk, G4int parent);

}  // namespace MuoTables

#endif
