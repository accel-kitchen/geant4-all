//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// MuoConfig.hh
// 実行時設定。/muo/ コマンド (MuoMessenger) が /run/initialize の前に書き込み、
// 初期化後は全スレッドが読み取り専用で参照する。
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifndef MuoConfig_h
#define MuoConfig_h 1

#include "globals.hh"
#include "G4SystemOfUnits.hh"
#include <string>
#include <vector>

// full: 地表から検出器まで一度に通す / A: 地盤輸送 (検出器直上の面で記録) / B: 検出器応答
enum class MuoStage { Full, A, B };

struct SoilLayerSpec {
  G4String name;
  G4double xmin, xmax;  // 層の X 範囲 [length]
  G4double rho;         // かさ密度 [mass/volume]
  G4double theta;       // 体積含水率 (無次元)
};

class MuoConfig {
public:
  static MuoConfig& Get();

  // ---- 実行 ----
  MuoStage stage = MuoStage::Full;
  G4String outDir = "runs";
  G4String prefix = "run";
  G4String macroName = "";
  G4String phaseSpacePrefix = "";      // Stage B の入力 (Stage A の出力 prefix)
  G4bool   coincidenceOnly = true;
  G4int    seed = 12345;
  G4double soilCut = 1.0*cm;           // 土の領域の生成カット

  // ---- 検出器 (原点 = 2 層の中点) ----
  G4double pitch = 2.0*cm;             // ピクセル幅 = ピッチ
  G4double thickness = 1.0*cm;
  G4double gap = 30.0*cm;              // 層の中心間距離
  G4int    nx = 30, ny = 30;
  G4double housing = 0.0;              // >0 なら Al 板を上下に置く
  G4double threshold = 0.5*MeV;        // L0 のヒットしきい値

  // ---- 地盤 ----
  G4bool   soilEnabled = true;
  G4double overburden = 10.0*m;        // 検出器基準点から地表までの高さ
  G4double airGap = 0.5*m;             // 管路なしのとき、土の下面の高さ
  G4double halfWidth = -1.0;           // <0 なら自動
  std::vector<SoilLayerSpec> layers;   // 空なら 砂 1.4 / 粘土 1.8 の 2 層
  G4bool   cavityEnabled = false;
  G4double cavityCenter[3] = {0, 0, 0};
  G4double cavitySize[3] = {1.0*m, 1.0*m, 1.0*m};

  // ---- 管路 ----
  G4bool   pipeEnabled = false;
  G4double pipeInnerRadius = 1.5*m;
  G4double pipeWall = 0.3*m;
  G4double waterDepth = 0.3*m;
  G4double waterSurfaceBelow = 0.15*m; // 検出器基準点から水面までの距離

  // ---- 一次粒子 ----
  G4double zenithMax = 60.0*deg;
  G4double targetRadius = 1.0*m;
  G4double eLow = 1.0*GeV;
  G4double eHigh = 500.0*GeV;
  G4double energyIndex = 1.0;          // 1 なら ln E 一様
  G4double plusFraction = 0.559;       // mu+ の割合 (電荷比 1.27)

  // ---- 派生量 ----
  G4double PlaneZ() const { return 0.5*gap + 0.5*thickness + housing + 5.0*cm; }
  G4double PlaneHalfXY() const;
  G4double SourceTopZ() const;
  G4double SoilHalfWidth() const;
  G4double PipeOuterRadius() const { return pipeInnerRadius + pipeWall; }
  G4double PipeAxisZ() const { return -waterSurfaceBelow - waterDepth + pipeInnerRadius; }
  std::vector<SoilLayerSpec> EffectiveLayers() const;
  G4String StageName() const;
  void Validate() const;
  std::string ToJson() const;

private:
  MuoConfig() = default;
};

#endif
