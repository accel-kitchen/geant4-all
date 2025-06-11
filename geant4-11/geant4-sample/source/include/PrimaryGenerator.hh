//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// PrimaryGenerator.hh
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifndef PrimaryGenerator_h
#define PrimaryGenerator_h 1

#include "G4VUserPrimaryGeneratorAction.hh"
#include "G4ThreeVector.hh"
#include <vector>
class G4Event;
class G4GeneralParticleSource;
class G4ParticleGun;
class PrimaryGeneratorMessenger;

//------------------------------------------------------------------------------
  class PrimaryGenerator : public G4VUserPrimaryGeneratorAction
//------------------------------------------------------------------------------
{
  public:
    PrimaryGenerator();
   ~PrimaryGenerator() override;

  public:
    void GeneratePrimaries( G4Event* ) override;
    
    // 天頂角度分布設定
    void SetUseZenithDistribution(G4bool flag) { fUseZenithDistribution = flag; }
    void SetMaxZenithAngle(G4double angle) { fMaxZenithAngle = angle; }
    void SetSphereRadius(G4double radius) { fSphereRadius = radius; }
    void SetCircleRadius(G4double radius) { fCircleRadius = radius; }

  private:
    // 高速化のための初期化とプリコンピュート
    void InitializeLookupTables();
    
    // cos²θ分布に従った天頂角をサンプリング（高速化版）
    G4double SampleZenithAngleWithCos2Distribution();
    // 方位角をサンプリング（等方）
    G4double SampleAzimuthAngle();
    // Z=10m平面上のランダムな点をサンプリング
    G4ThreeVector SamplePointOnPlane();
    // (0,0,-1)方向を基準としたcos²θ分布の方向ベクトルを計算
    G4ThreeVector SampleDirectionWithCos2Distribution();
    
    // 地上宇宙線ミュオンのエネルギー分布サンプリング（べき乗則 E^-2.7）
    G4double SampleCosmicRayMuonEnergy();

  private:
    G4GeneralParticleSource* fpParticleGPS;
    G4ParticleGun* fpParticleGun;
    PrimaryGeneratorMessenger* fpMessenger;
    
    // 天頂角度分布の設定
    G4bool fUseZenithDistribution;
    G4double fMaxZenithAngle;
    G4double fSphereRadius;      // 球の半径 (20m)
    G4double fCircleRadius;      // 接円の半径 (1m)
    
    // イベントカウンター（1000発ごとの出力用）
    static G4int fEventCounter;
    
    // 高速化のためのルックアップテーブル
    static std::vector<G4double> fCosZenithLookup;
    static std::vector<G4double> fEnergyLookup;
    static G4bool fTablesInitialized;
    static const G4int fTableSize = 10000;  // ルックアップテーブルサイズ
    
    // 地上宇宙線ミュオンエネルギースペクトルパラメータ
    static const G4double fE0;         // ピボットエネルギー (357 MeV)
    static const G4double fMinEnergy;  // 最小エネルギー (100 MeV)
    static const G4double fMaxEnergy;  // 最大エネルギー (10 GeV)
    static const G4double fA_low;      // 低エネ側正規化係数
    static const G4double fAlpha;      // 低エネ側指数
    static const G4double fA_high;     // 高エネ側正規化係数
    static const G4double fBeta;       // 高エネ側指数
    static G4double fC_low;            // 低エネ側積分値
    static G4double fC_high;           // 高エネ側積分値
    static G4double fC_tot;            // 全積分値
};
#endif
