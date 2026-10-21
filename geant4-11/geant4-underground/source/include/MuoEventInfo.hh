//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// MuoEventInfo.hh
// イベントに付ける情報。重み w1 は「1 イベントあたりのレート [Hz]」で、
// 測定レート = Σ w1 / N_generated。
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifndef MuoEventInfo_h
#define MuoEventInfo_h 1

#include "G4VUserEventInformation.hh"
#include "globals.hh"

class MuoEventInfo : public G4VUserEventInformation {
public:
  MuoEventInfo(G4int srcEvent, G4double w, G4double e0GeV, G4double cth, G4double ph)
    : srcEvent(srcEvent), w1(w), E0(e0GeV), cth0(cth), ph0(ph) {}
  void Print() const override {}

  G4int    srcEvent;  // 出力表の event 列 (Stage B では Stage A のイベント番号)
  G4double w1;        // [Hz]
  G4double E0;        // 発生時のエネルギー [GeV]
  G4double cth0;      // cos(天頂角)
  G4double ph0;       // 方位角 [rad]
};

#endif
