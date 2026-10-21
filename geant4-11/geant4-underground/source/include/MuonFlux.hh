//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// MuonFlux.hh
// 海面のミュオン微分強度。Gaisser 式に低エネルギー・大天頂角の補正を加えた形
// (Guan et al., arXiv:1509.06176)。analysis/feasibility.py と同じ式。
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifndef MuonFlux_h
#define MuonFlux_h 1

#include "globals.hh"
#include <cmath>

namespace MuonFlux {

inline G4double CosStar(G4double c)
{
  const G4double p1 = 0.102573, p2 = -0.068287, p3 = 0.958633, p4 = 0.0407253, p5 = 0.817285;
  return std::sqrt((c*c + p1*p1 + p2*std::pow(c, p3) + p4*std::pow(c, p5)) / (1.0 + p1*p1 + p2 + p4));
}

// E: ミュオンのエネルギー [GeV]、c: cos(天頂角)。戻り値 [cm^-2 s^-1 sr^-1 GeV^-1]
inline G4double DiffIntensity(G4double E, G4double c)
{
  const G4double cs = CosStar(c);
  const G4double Ee = E*(1.0 + 3.64/(E*std::pow(cs, 1.29)));
  return 0.14*std::pow(Ee, -2.7)*(1.0/(1.0 + 1.1*E*cs/115.0) + 0.054/(1.0 + 1.1*E*cs/850.0));
}

}  // namespace MuonFlux

#endif
