//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// MuoMessenger.cc
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#include "MuoMessenger.hh"
#include "MuoConfig.hh"

#include "G4UIdirectory.hh"
#include "G4UIcommand.hh"
#include "G4UIparameter.hh"
#include "G4UIcmdWithAString.hh"
#include "G4UIcmdWithABool.hh"
#include "G4UIcmdWithAnInteger.hh"
#include "G4UIcmdWithADouble.hh"
#include "G4UIcmdWithADoubleAndUnit.hh"
#include "G4SystemOfUnits.hh"
#include "Randomize.hh"
#include <sstream>

//------------------------------------------------------------------------------
  MuoMessenger::MuoMessenger()
//------------------------------------------------------------------------------
{
  auto& c = MuoConfig::Get();
  auto dir = [this](const char* path, const char* guide) {
    auto* d = new G4UIdirectory(path);
    d->SetGuidance(guide);
    fDirs.push_back(d);
  };
  dir("/muo/", "ミュオグラフィ検出器シミュレーションの設定");
  dir("/muo/run/", "実行");
  dir("/muo/out/", "出力");
  dir("/muo/det/", "検出器");
  dir("/muo/soil/", "地盤");
  dir("/muo/pipe/", "管路");
  dir("/muo/source/", "一次粒子");

  // ---- 実行 ----
  fStageCmd = new G4UIcmdWithAString("/muo/run/stage", this);
  fStageCmd->SetGuidance("full: 地表から検出器まで / A: 地盤輸送 / B: 検出器応答 (位相空間を再生)");
  fStageCmd->SetParameterName("stage", false);
  fStageCmd->SetCandidates("full A B");
  fAll.push_back(fStageCmd);

  fSeedCmd = new G4UIcmdWithAnInteger("/muo/run/seed", this);
  fSeedCmd->SetGuidance("乱数のシード (meta.json に記録される)");
  fSeedCmd->SetParameterName("seed", false);
  fAll.push_back(fSeedCmd);

  Quantity("/muo/run/soilCut", "土の領域の生成カット", "Length", "cm", &c.soilCut);

  // ---- 出力 ----
  Text("/muo/out/dir", "出力ディレクトリ", &c.outDir);
  Text("/muo/out/prefix", "出力ファイルの接頭辞", &c.prefix);
  Text("/muo/out/phaseSpaceInput", "Stage B の入力 (Stage A の <dir>/<prefix>)", &c.phaseSpacePrefix);
  Flag("/muo/out/coincidenceOnly", "2 層で同時計数したイベントだけをヒット表に書く", &c.coincidenceOnly);

  // ---- 検出器 ----
  Quantity("/muo/det/pitch", "ピクセル幅 (= ピッチ)", "Length", "cm", &c.pitch);
  Quantity("/muo/det/thickness", "シンチレータの厚み", "Length", "cm", &c.thickness);
  Quantity("/muo/det/gap", "2 層の中心間距離", "Length", "cm", &c.gap);
  Quantity("/muo/det/housing", "上下の Al 板の厚み (0 で無し)", "Length", "mm", &c.housing);
  Quantity("/muo/det/threshold", "L0 のヒットしきい値 (ピクセルのエネルギー付与)", "Energy", "MeV", &c.threshold);

  fNxyCmd = new G4UIcommand("/muo/det/nxy", this);
  fNxyCmd->SetGuidance("1 層あたりのピクセル数 nx ny (各 1〜99)");
  fNxyCmd->SetParameter(new G4UIparameter("nx", 'i', false));
  fNxyCmd->SetParameter(new G4UIparameter("ny", 'i', false));
  fAll.push_back(fNxyCmd);

  // ---- 地盤 ----
  Flag("/muo/soil/enable", "地盤を置くか (false なら空気だけ)", &c.soilEnabled);
  Quantity("/muo/soil/overburden", "検出器の基準点から地表までの高さ", "Length", "m", &c.overburden);
  Quantity("/muo/soil/airGap", "管路なしのときの土の下面の高さ", "Length", "m", &c.airGap);
  Quantity("/muo/soil/halfWidth", "土の横幅の半分 (未指定なら天頂角の上限から自動)", "Length", "m", &c.halfWidth);

  fAddLayerCmd = new G4UIcommand("/muo/soil/addLayer", this);
  fAddLayerCmd->SetGuidance("X 方向の層を追加: name xmin[m] xmax[m] rho[g/cm3] thetaV(体積含水率)");
  fAddLayerCmd->SetParameter(new G4UIparameter("name", 's', false));
  for (const char* n : {"xmin", "xmax", "rho", "theta"}) fAddLayerCmd->SetParameter(new G4UIparameter(n, 'd', false));
  fAll.push_back(fAddLayerCmd);

  fClearLayersCmd = new G4UIcommand("/muo/soil/clearLayers", this);
  fClearLayersCmd->SetGuidance("層の指定を消す (既定の 砂 1.4 / 粘土 1.8 に戻る)");
  fAll.push_back(fClearLayersCmd);

  fCavityCmd = new G4UIcommand("/muo/soil/cavity", this);
  fCavityCmd->SetGuidance("空洞 (空気の箱): x y z sx sy sz [m]。中心は検出器基準点から");
  for (const char* n : {"x", "y", "z", "sx", "sy", "sz"}) fCavityCmd->SetParameter(new G4UIparameter(n, 'd', false));
  fAll.push_back(fCavityCmd);

  // ---- 管路 ----
  Flag("/muo/pipe/enable", "管路 (コンクリート壁 + 水) を置くか", &c.pipeEnabled);
  Quantity("/muo/pipe/innerRadius", "管の内半径", "Length", "m", &c.pipeInnerRadius);
  Quantity("/muo/pipe/wall", "管壁の厚み", "Length", "m", &c.pipeWall);
  Quantity("/muo/pipe/waterDepth", "水深", "Length", "cm", &c.waterDepth);

  // ---- 一次粒子 ----
  Quantity("/muo/source/zenithMax", "天頂角の上限", "Angle", "deg", &c.zenithMax);
  Quantity("/muo/source/targetRadius", "検出器を中心とするターゲット円盤の半径", "Length", "m", &c.targetRadius);
  Quantity("/muo/source/eLow", "発生エネルギーの下限", "Energy", "GeV", &c.eLow);
  Quantity("/muo/source/eHigh", "発生エネルギーの上限", "Energy", "GeV", &c.eHigh);
  Number("/muo/source/energyIndex", "発生エネルギー分布 ∝ E^-index (1 で ln E 一様)", &c.energyIndex);
  Number("/muo/source/plusFraction", "mu+ の割合", &c.plusFraction);
}

//------------------------------------------------------------------------------
  MuoMessenger::~MuoMessenger()
//------------------------------------------------------------------------------
{
  for (auto* cmd : fAll) delete cmd;
  for (auto* d : fDirs) delete d;
}

//------------------------------------------------------------------------------
  G4UIcmdWithADoubleAndUnit* MuoMessenger::Quantity(const G4String& path, const G4String& guide,
      const G4String& category, const G4String& defUnit, G4double* target)
//------------------------------------------------------------------------------
{
  auto* cmd = new G4UIcmdWithADoubleAndUnit(path, this);
  cmd->SetGuidance(guide);
  cmd->SetParameterName("value", false);
  cmd->SetUnitCategory(category);
  cmd->SetDefaultUnit(defUnit);
  fAll.push_back(cmd);
  fQuantities.emplace_back(cmd, target);
  return cmd;
}

//------------------------------------------------------------------------------
  G4UIcmdWithAString* MuoMessenger::Text(const G4String& path, const G4String& guide, G4String* target)
//------------------------------------------------------------------------------
{
  auto* cmd = new G4UIcmdWithAString(path, this);
  cmd->SetGuidance(guide);
  cmd->SetParameterName("text", false);
  fAll.push_back(cmd);
  fTexts.emplace_back(cmd, target);
  return cmd;
}

//------------------------------------------------------------------------------
  G4UIcmdWithABool* MuoMessenger::Flag(const G4String& path, const G4String& guide, G4bool* target)
//------------------------------------------------------------------------------
{
  auto* cmd = new G4UIcmdWithABool(path, this);
  cmd->SetGuidance(guide);
  cmd->SetParameterName("flag", false);
  fAll.push_back(cmd);
  fFlags.emplace_back(cmd, target);
  return cmd;
}

//------------------------------------------------------------------------------
  G4UIcmdWithADouble* MuoMessenger::Number(const G4String& path, const G4String& guide, G4double* target)
//------------------------------------------------------------------------------
{
  auto* cmd = new G4UIcmdWithADouble(path, this);
  cmd->SetGuidance(guide);
  cmd->SetParameterName("value", false);
  fAll.push_back(cmd);
  fNumbers.emplace_back(cmd, target);
  return cmd;
}

//------------------------------------------------------------------------------
  void MuoMessenger::SetNewValue(G4UIcommand* cmd, G4String value)
//------------------------------------------------------------------------------
{
  auto& c = MuoConfig::Get();
  for (auto& p : fQuantities) if (cmd == p.first) { *p.second = p.first->GetNewDoubleValue(value); return; }
  for (auto& p : fTexts)      if (cmd == p.first) { *p.second = value; return; }
  for (auto& p : fFlags)      if (cmd == p.first) { *p.second = p.first->GetNewBoolValue(value); return; }
  for (auto& p : fNumbers)    if (cmd == p.first) { *p.second = p.first->GetNewDoubleValue(value); return; }

  if (cmd == fStageCmd) {
    c.stage = (value == "A") ? MuoStage::A : (value == "B") ? MuoStage::B : MuoStage::Full;
  } else if (cmd == fSeedCmd) {
    c.seed = fSeedCmd->GetNewIntValue(value);
    G4Random::setTheSeed(c.seed);
  } else if (cmd == fNxyCmd) {
    std::istringstream is(value);
    is >> c.nx >> c.ny;
  } else if (cmd == fAddLayerCmd) {
    std::istringstream is(value);
    SoilLayerSpec l;
    G4double xmin, xmax, rho, theta;
    is >> l.name >> xmin >> xmax >> rho >> theta;
    l.xmin = xmin*m;
    l.xmax = xmax*m;
    l.rho = rho*g/cm3;
    l.theta = theta;
    c.layers.push_back(l);
  } else if (cmd == fClearLayersCmd) {
    c.layers.clear();
  } else if (cmd == fCavityCmd) {
    std::istringstream is(value);
    G4double v[6];
    for (auto& x : v) is >> x;
    for (int i = 0; i < 3; ++i) { c.cavityCenter[i] = v[i]*m; c.cavitySize[i] = v[3 + i]*m; }
    c.cavityEnabled = true;
  }
}
