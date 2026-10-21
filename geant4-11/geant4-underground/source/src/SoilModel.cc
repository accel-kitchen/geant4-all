//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// SoilModel.cc
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#include "SoilModel.hh"
#include "G4Material.hh"
#include "G4NistManager.hh"
#include "G4SystemOfUnits.hh"
#include <algorithm>
#include <cstdio>

namespace SoilModel {

//------------------------------------------------------------------------------
  G4Material* Get(const G4String& label, G4double rho, G4double thetaV)
//------------------------------------------------------------------------------
{
  char buf[96];
  std::snprintf(buf, sizeof(buf), "Soil_%s_r%.4f_w%.4f", label.c_str(), rho/(g/cm3), thetaV);
  const G4String name(buf);

  if (auto* existing = G4Material::GetMaterial(name, false)) return existing;

  auto* nist = G4NistManager::Instance();
  const G4double fw = std::clamp(thetaV*1.0*g/cm3/rho, 0.0, 0.6);
  auto* sio2 = nist->FindOrBuildMaterial("G4_SILICON_DIOXIDE");
  if (fw <= 0.0) {
    auto* mat = new G4Material(name, rho, 1);
    mat->AddMaterial(sio2, 1.0);
    return mat;
  }
  auto* water = nist->FindOrBuildMaterial("G4_WATER");
  auto* mat = new G4Material(name, rho, 2);
  mat->AddMaterial(sio2, 1.0 - fw);
  mat->AddMaterial(water, fw);
  return mat;
}

}  // namespace SoilModel
