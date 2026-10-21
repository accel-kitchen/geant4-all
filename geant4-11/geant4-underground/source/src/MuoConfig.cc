//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// MuoConfig.cc
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#include "MuoConfig.hh"
#include "G4Exception.hh"
#include "G4PhysicalConstants.hh"
#include <algorithm>
#include <cmath>
#include <sstream>

//------------------------------------------------------------------------------
  MuoConfig& MuoConfig::Get()
//------------------------------------------------------------------------------
{
  static MuoConfig instance;
  return instance;
}

//------------------------------------------------------------------------------
  G4double MuoConfig::PlaneHalfXY() const
//------------------------------------------------------------------------------
{
  return 0.5*std::max(nx, ny)*pitch + 0.5*m;
}

//------------------------------------------------------------------------------
  G4double MuoConfig::SourceTopZ() const
//------------------------------------------------------------------------------
{
  // 地表 (または土なしのとき検出器の 1 m 上) の少し上から出発させる
  return soilEnabled ? overburden + 0.2*m : PlaneZ() + 1.0*m;
}

//------------------------------------------------------------------------------
  G4double MuoConfig::SoilHalfWidth() const
//------------------------------------------------------------------------------
{
  if (halfWidth > 0) return halfWidth;
  // 天頂角の上限まで斜めに入る経路が土の外に出ない幅
  G4double w = overburden*std::tan(zenithMax) + targetRadius + 2.0*m;
  for (const auto& l : layers) w = std::max(w, std::max(std::fabs(l.xmin), std::fabs(l.xmax)));
  return w;
}

//------------------------------------------------------------------------------
  std::vector<SoilLayerSpec> MuoConfig::EffectiveLayers() const
//------------------------------------------------------------------------------
{
  if (!layers.empty()) return layers;
  const G4double w = SoilHalfWidth();
  return { {"sand", -w, 0.0, 1.4*g/cm3, 0.0}, {"clay", 0.0, w, 1.8*g/cm3, 0.0} };
}

//------------------------------------------------------------------------------
  G4String MuoConfig::StageName() const
//------------------------------------------------------------------------------
{
  switch (stage) {
    case MuoStage::A: return "A";
    case MuoStage::B: return "B";
    default:          return "full";
  }
}

//------------------------------------------------------------------------------
  void MuoConfig::Validate() const
//------------------------------------------------------------------------------
{
  auto fail = [](const char* msg) {
    G4Exception("MuoConfig::Validate", "Muo001", FatalException, msg);
  };
  if (nx < 1 || ny < 1 || nx > 99 || ny > 99) fail("/muo/det/nxy は 1〜99 (CopyNo の 2 桁に収まる範囲)");
  if (eLow < 0.12*GeV || eHigh <= eLow)        fail("/muo/source/eLow は 0.12 GeV 以上、eHigh は eLow より大きく");
  if (zenithMax <= 0 || zenithMax > 85.0*deg)  fail("/muo/source/zenithMax は 0〜85 deg");
  if (stage == MuoStage::B && phaseSpacePrefix.empty())
    fail("Stage B には /muo/out/phaseSpaceInput が必要");
  if (stage != MuoStage::B && soilEnabled) {
    if (!pipeEnabled && airGap <= PlaneZ() + 5.0*mm) fail("/muo/soil/airGap が小さすぎる (検出器と記録面が土に入る)");
    if (overburden <= (pipeEnabled ? PipeAxisZ() + PipeOuterRadius() : airGap))
      fail("/muo/soil/overburden が管または空隙より小さい");
  }
  if (pipeEnabled && stage != MuoStage::B && PipeAxisZ() + pipeInnerRadius < PlaneZ() + 5.0*cm)
    fail("管の内径が小さく、検出器が管に収まらない");
}

//------------------------------------------------------------------------------
  std::string MuoConfig::ToJson() const
//------------------------------------------------------------------------------
{
  std::ostringstream o;
  auto mm_ = [](G4double v) { return v/mm; };
  o << "  \"config\": {\n"
    << "    \"stage\": \"" << StageName() << "\",\n"
    << "    \"phase_space_input\": \"" << phaseSpacePrefix << "\",\n"
    << "    \"coincidence_only\": " << (coincidenceOnly ? "true" : "false") << ",\n"
    << "    \"soil_cut_mm\": " << mm_(soilCut) << ",\n"
    << "    \"det\": {\"pitch_mm\": " << mm_(pitch) << ", \"thickness_mm\": " << mm_(thickness)
    << ", \"gap_mm\": " << mm_(gap) << ", \"nx\": " << nx << ", \"ny\": " << ny
    << ", \"housing_mm\": " << mm_(housing) << ", \"threshold_MeV\": " << threshold/MeV << "},\n"
    << "    \"soil\": {\"enabled\": " << (soilEnabled ? "true" : "false")
    << ", \"overburden_mm\": " << mm_(overburden) << ", \"air_gap_mm\": " << mm_(airGap)
    << ", \"half_width_mm\": " << mm_(SoilHalfWidth()) << ", \"layers\": [";
  const auto ls = EffectiveLayers();
  for (size_t i = 0; i < ls.size(); ++i) {
    o << (i ? ", " : "") << "{\"name\": \"" << ls[i].name << "\", \"xmin_mm\": " << mm_(ls[i].xmin)
      << ", \"xmax_mm\": " << mm_(ls[i].xmax) << ", \"rho_g_cm3\": " << ls[i].rho/(g/cm3)
      << ", \"theta_v\": " << ls[i].theta << "}";
  }
  o << "], \"cavity\": {\"enabled\": " << (cavityEnabled ? "true" : "false")
    << ", \"center_mm\": [" << mm_(cavityCenter[0]) << ", " << mm_(cavityCenter[1]) << ", " << mm_(cavityCenter[2])
    << "], \"size_mm\": [" << mm_(cavitySize[0]) << ", " << mm_(cavitySize[1]) << ", " << mm_(cavitySize[2]) << "]}},\n"
    << "    \"pipe\": {\"enabled\": " << (pipeEnabled ? "true" : "false")
    << ", \"inner_radius_mm\": " << mm_(pipeInnerRadius) << ", \"wall_mm\": " << mm_(pipeWall)
    << ", \"water_depth_mm\": " << mm_(waterDepth) << "},\n"
    << "    \"source\": {\"zenith_max_deg\": " << zenithMax/deg << ", \"target_radius_mm\": " << mm_(targetRadius)
    << ", \"e_low_GeV\": " << eLow/GeV << ", \"e_high_GeV\": " << eHigh/GeV
    << ", \"energy_index\": " << energyIndex << ", \"plus_fraction\": " << plusFraction << "}\n"
    << "  }";
  return o.str();
}
