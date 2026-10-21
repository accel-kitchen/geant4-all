//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// DetectorConstruction.cc
// 座標系: z 上向き。原点は 2 層検出器の中点。管路の軸は X 方向。
//   stage full : 地盤 (+管路) + 検出器
//   stage A    : 地盤 (+管路) + 検出器直上の記録面 (検出器は置かない)
//   stage B    : 検出器だけ (記録面から再生した粒子を入れる)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#include "DetectorConstruction.hh"
#include "MuoConfig.hh"
#include "SoilModel.hh"
#include "ScintSD.hh"
#include "PlaneSD.hh"

#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4SubtractionSolid.hh"
#include "G4IntersectionSolid.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4NistManager.hh"
#include "G4Material.hh"
#include "G4Region.hh"
#include "G4ProductionCuts.hh"
#include "G4RotationMatrix.hh"
#include "G4SDManager.hh"
#include "G4ThreeVector.hh"
#include "G4SystemOfUnits.hh"
#include "G4PhysicalConstants.hh"
#include <algorithm>
#include <cmath>

//------------------------------------------------------------------------------
  G4VPhysicalVolume* DetectorConstruction::Construct()
//------------------------------------------------------------------------------
{
  const auto& c = MuoConfig::Get();
  c.Validate();

  auto* nist = G4NistManager::Instance();
  auto* air = nist->FindOrBuildMaterial("G4_AIR");

  // ワールドは z について対称な箱にする
  G4double hx, hy, hz;
  if (c.stage == MuoStage::B) {
    hx = hy = std::max(2.0*m, c.PlaneHalfXY() + 1.0*m);
    hz = 2.0*m;
  } else {
    const G4double w = c.soilEnabled ? c.SoilHalfWidth() : 2.0*m;
    hx = hy = w + 1.0*m;
    const G4double zMax = c.SourceTopZ() + 0.5*m;
    const G4double zMin = (c.soilEnabled && c.pipeEnabled)
                            ? c.PipeAxisZ() - c.PipeOuterRadius() - 2.0*m : -2.0*m;
    hz = std::max(std::fabs(zMax), std::fabs(zMin));
  }
  auto* worldS  = new G4Box("World", hx, hy, hz);
  auto* worldLV = new G4LogicalVolume(worldS, air, "WorldLV");
  auto* worldPV = new G4PVPlacement(nullptr, G4ThreeVector(), worldLV, "World", nullptr, false, 0, false);

  if (c.stage != MuoStage::B && c.soilEnabled) BuildGround(worldLV);
  if (c.stage != MuoStage::A) BuildDetector(worldLV);
  if (c.stage == MuoStage::A) BuildPlane(worldLV);
  return worldPV;
}

//------------------------------------------------------------------------------
  void DetectorConstruction::BuildGround(G4LogicalVolume* world)
//------------------------------------------------------------------------------
{
  const auto& c = MuoConfig::Get();
  auto* nist = G4NistManager::Instance();

  const G4double Wx = c.SoilHalfWidth();
  const G4double Wy = Wx;
  const G4double zTop = c.overburden;
  const G4double zAxis = c.PipeAxisZ();
  const G4double Rin = c.pipeInnerRadius;
  const G4double Rout = c.PipeOuterRadius();
  const G4double zBot = c.pipeEnabled ? zAxis - Rout - 1.0*m : c.airGap;
  const G4double zc = 0.5*(zTop + zBot);

  // 管の軸 (solid の z) を世界の X に向ける回転
  auto* rotY = new G4RotationMatrix;
  rotY->rotateY(90.0*deg);

  auto* soilRegion = new G4Region("SoilRegion");
  auto* cuts = new G4ProductionCuts;
  cuts->SetProductionCut(c.soilCut);
  soilRegion->SetProductionCuts(cuts);

  for (const auto& l : c.EffectiveLayers()) {
    const G4double xc = 0.5*(l.xmin + l.xmax);
    G4VSolid* solid = new G4Box("SoilBox_" + l.name, 0.5*(l.xmax - l.xmin), Wy, 0.5*(zTop - zBot));
    if (c.pipeEnabled) {
      auto* cyl = new G4Tubs("PipeCut_" + l.name, 0.0, Rout, Wx + 1.0*m, 0.0, twopi);
      solid = new G4SubtractionSolid("SoilP_" + l.name, solid, cyl, rotY, G4ThreeVector(-xc, 0.0, zAxis - zc));
    }
    if (c.cavityEnabled) {
      auto* cav = new G4Box("Cavity_" + l.name, 0.5*c.cavitySize[0], 0.5*c.cavitySize[1], 0.5*c.cavitySize[2]);
      solid = new G4SubtractionSolid("SoilC_" + l.name, solid, cav, nullptr,
                                     G4ThreeVector(c.cavityCenter[0] - xc, c.cavityCenter[1], c.cavityCenter[2] - zc));
    }
    auto* lv = new G4LogicalVolume(solid, SoilModel::Get(l.name, l.rho, l.theta), "SoilLV_" + l.name);
    lv->SetRegion(soilRegion);
    soilRegion->AddRootLogicalVolume(lv);
    new G4PVPlacement(nullptr, G4ThreeVector(xc, 0.0, zc), lv, "Soil_" + l.name, world, false, 0, true);
  }

  if (!c.pipeEnabled) return;

  // 管壁 (コンクリート)。内側は世界の空気がそのまま管内になる
  auto* wallS  = new G4Tubs("PipeWall", Rin, Rout, Wx, 0.0, twopi);
  auto* wallLV = new G4LogicalVolume(wallS, nist->FindOrBuildMaterial("G4_CONCRETE"), "PipeWallLV");
  new G4PVPlacement(rotY, G4ThreeVector(0.0, 0.0, zAxis), wallLV, "PipeWall", world, false, 0, true);

  // 水 (管の内側の下部)
  const G4double depth = std::min(c.waterDepth, 1.8*Rin);
  if (depth > 0.0) {
    const G4double zw = (zAxis - Rin) + 0.5*depth;
    auto* boxW = new G4Box("WaterBox", Wx, Rin, 0.5*depth);
    auto* cylI = new G4Tubs("WaterCyl", 0.0, Rin, Wx + 1.0*m, 0.0, twopi);
    auto* waterS = new G4IntersectionSolid("Water", boxW, cylI, rotY, G4ThreeVector(0.0, 0.0, zAxis - zw));
    auto* waterLV = new G4LogicalVolume(waterS, nist->FindOrBuildMaterial("G4_WATER"), "WaterLV");
    new G4PVPlacement(nullptr, G4ThreeVector(0.0, 0.0, zw), waterLV, "Water", world, false, 0, true);
  }
}

//------------------------------------------------------------------------------
  void DetectorConstruction::BuildDetector(G4LogicalVolume* world)
//------------------------------------------------------------------------------
{
  const auto& c = MuoConfig::Get();
  auto* nist = G4NistManager::Instance();

  auto* pixelS = new G4Box("Pixel", 0.5*c.pitch, 0.5*c.pitch, 0.5*c.thickness);
  fLogScint = new G4LogicalVolume(pixelS, nist->FindOrBuildMaterial("G4_PLASTIC_SC_VINYLTOLUENE"), "PixelLV");

  // CopyNo = layer*10000 + ix*100 + iy  (layer 1: 下段 z=-gap/2、layer 2: 上段 z=+gap/2)
  for (G4int layer = 1; layer <= 2; ++layer) {
    const G4double z = (layer == 1 ? -0.5 : 0.5)*c.gap;
    for (G4int ix = 0; ix < c.nx; ++ix) {
      for (G4int iy = 0; iy < c.ny; ++iy) {
        const G4double x = (ix - 0.5*(c.nx - 1))*c.pitch;
        const G4double y = (iy - 0.5*(c.ny - 1))*c.pitch;
        new G4PVPlacement(nullptr, G4ThreeVector(x, y, z), fLogScint, "Pixel", world,
                          false, layer*10000 + ix*100 + iy, false);
      }
    }
  }

  if (c.housing > 0.0) {
    auto* plateS = new G4Box("Housing", 0.5*c.nx*c.pitch, 0.5*c.ny*c.pitch, 0.5*c.housing);
    auto* plateLV = new G4LogicalVolume(plateS, nist->FindOrBuildMaterial("G4_Al"), "HousingLV");
    const G4double dz = 0.5*c.gap + 0.5*c.thickness + 0.5*c.housing;
    new G4PVPlacement(nullptr, G4ThreeVector(0, 0, +dz), plateLV, "HousingTop", world, false, 0, true);
    new G4PVPlacement(nullptr, G4ThreeVector(0, 0, -dz), plateLV, "HousingBottom", world, false, 1, true);
  }
}

//------------------------------------------------------------------------------
  void DetectorConstruction::BuildPlane(G4LogicalVolume* world)
//------------------------------------------------------------------------------
{
  const auto& c = MuoConfig::Get();
  auto* air = G4NistManager::Instance()->FindOrBuildMaterial("G4_AIR");
  auto* planeS = new G4Box("Plane", c.PlaneHalfXY(), c.PlaneHalfXY(), 0.5*mm);
  fLogPlane = new G4LogicalVolume(planeS, air, "PlaneLV");
  new G4PVPlacement(nullptr, G4ThreeVector(0, 0, c.PlaneZ()), fLogPlane, "Plane", world, false, 0, true);
}

//------------------------------------------------------------------------------
  void DetectorConstruction::ConstructSDandField()
//------------------------------------------------------------------------------
{
  // 論理体積への割り当てだけでは SDManager に登録されず、Initialize / EndOfEvent が呼ばれない
  auto* sdMan = G4SDManager::GetSDMpointer();
  if (fLogScint) {
    auto* sd = new ScintSD("ScintSD");
    sdMan->AddNewDetector(sd);
    SetSensitiveDetector(fLogScint, sd);
  }
  if (fLogPlane) {
    auto* sd = new PlaneSD("PlaneSD");
    sdMan->AddNewDetector(sd);
    SetSensitiveDetector(fLogPlane, sd);
  }
}
