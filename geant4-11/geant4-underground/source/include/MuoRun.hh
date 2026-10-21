//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// MuoRun.hh
// スレッドごとの集計をマスターで合算する Run。
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifndef MuoRun_h
#define MuoRun_h 1

#include "G4Run.hh"
#include "globals.hh"

class MuoRun : public G4Run {
public:
  MuoRun() = default;
  ~MuoRun() override = default;
  void Merge(const G4Run* run) override;

  void AddGenerated(G4double w1)   { sumGenW += w1; sumGenW2 += w1*w1; }
  void AddCoincidence(G4double w1) { ++nCoinc; sumCoincW += w1; sumCoincW2 += w1*w1; }
  void AddPlaneRow()               { ++nPlaneRows; }

  G4double sumGenW = 0, sumGenW2 = 0;
  G4long   nCoinc = 0;
  G4double sumCoincW = 0, sumCoincW2 = 0;
  G4long   nPlaneRows = 0;
};

#endif
