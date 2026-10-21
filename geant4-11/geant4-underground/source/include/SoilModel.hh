//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// SoilModel.hh
// かさ密度と体積含水率から土 (SiO2 + 水) の G4Material を作る (設計判断 D5)。
// 水の質量分率 f_w = theta_v * 1 g/cm3 / rho_bulk。空気の質量は無視する。
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifndef SoilModel_h
#define SoilModel_h 1

#include "globals.hh"

class G4Material;

namespace SoilModel {
  // 同じ (rho, theta) の呼び出しには同じ材料を返す
  G4Material* Get(const G4String& label, G4double rho, G4double thetaV);
}

#endif
