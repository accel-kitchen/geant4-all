//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// PrimaryGenerator.cc
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#include "PrimaryGenerator.hh"
#include "PrimaryGeneratorMessenger.hh"
#include "G4GeneralParticleSource.hh"
#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "G4ParticleDefinition.hh"
#include "G4SystemOfUnits.hh"
#include "G4PhysicalConstants.hh"
#include "Randomize.hh"
#include <cmath>
#include <iostream>

// staticメンバー変数の初期化
G4int PrimaryGenerator::fEventCounter = 0;
std::vector<G4double> PrimaryGenerator::fCosZenithLookup;
std::vector<G4double> PrimaryGenerator::fEnergyLookup;
G4bool PrimaryGenerator::fTablesInitialized = false;

// 地上宇宙線ミュオンエネルギースペクトルパラメータの定義
const G4double PrimaryGenerator::fE0 = 357.0 * MeV;               // ピボットエネルギー
const G4double PrimaryGenerator::fMinEnergy = 100.0 * MeV;        // 最小エネルギー
const G4double PrimaryGenerator::fMaxEnergy = 100.0 * GeV;         // 最大エネルギー
const G4double PrimaryGenerator::fA_low = 3.2909335e-08;          // 低エネ側正規化
const G4double PrimaryGenerator::fAlpha = 0.71342502;             // 低エネ側指数
const G4double PrimaryGenerator::fA_high = 9.2025072e-04;         // 高エネ側正規化
const G4double PrimaryGenerator::fBeta = 1.02851191;              // 高エネ側指数

// 積分値（初期化で計算）
G4double PrimaryGenerator::fC_low = 0.0;
G4double PrimaryGenerator::fC_high = 0.0;
G4double PrimaryGenerator::fC_tot = 0.0;

//------------------------------------------------------------------------------
  PrimaryGenerator::PrimaryGenerator()
  : fpParticleGPS{nullptr},
    fpParticleGun{nullptr},
    fpMessenger{nullptr},
    fUseZenithDistribution{true},
    fMaxZenithAngle{20.0*deg},
    fSphereRadius{200.0*m},
    fCircleRadius{10.0*m}
//------------------------------------------------------------------------------
{
  fpParticleGPS = new G4GeneralParticleSource{};
  fpParticleGun = new G4ParticleGun{1};
  fpMessenger = new PrimaryGeneratorMessenger{this};
  
  // デフォルト設定（ミュオン）
  G4ParticleTable* particleTable = G4ParticleTable::GetParticleTable();
  G4ParticleDefinition* muon = particleTable->FindParticle("mu-");
  fpParticleGun->SetParticleDefinition(muon);
  
  // ルックアップテーブルの初期化（高速化）
  if (!fTablesInitialized) {
    InitializeLookupTables();
  }
}

//------------------------------------------------------------------------------
  PrimaryGenerator::~PrimaryGenerator()
//------------------------------------------------------------------------------
{
  delete fpParticleGPS;
  delete fpParticleGun;
  delete fpMessenger;
}

//------------------------------------------------------------------------------
  void PrimaryGenerator::GeneratePrimaries( G4Event* anEvent )
//------------------------------------------------------------------------------
{
  // イベントカウンターを増加
  fEventCounter++;
  
  // 1000発ごとに進捗を出力
  if (fEventCounter % 1000 == 0) {
    std::cout << "=== 進捗報告 ===  " << fEventCounter << " 発のミュオンを照射しました" << std::endl;
  }
  
  if (fUseZenithDistribution) {
    // 1. Z=10m平面上のランダムな点を選択
    G4ThreeVector sourcePosition = SamplePointOnPlane();
    
    // 2. (0,0,-1)方向を基準としたcos²θ分布の方向ベクトルを計算
    G4ThreeVector direction = SampleDirectionWithCos2Distribution();
    
    // 3. 地上宇宙線ミュオンのエネルギー分布に従ってエネルギーをサンプリング
    G4double energy = SampleCosmicRayMuonEnergy();
    
    // 4. 粒子銃の設定
    fpParticleGun->SetParticlePosition(sourcePosition);
    fpParticleGun->SetParticleMomentumDirection(direction);
    fpParticleGun->SetParticleEnergy(energy);
    fpParticleGun->GeneratePrimaryVertex(anEvent);
  } else {
    // GPSを使用
    fpParticleGPS->GeneratePrimaryVertex(anEvent);
  }
}

//------------------------------------------------------------------------------
  G4double PrimaryGenerator::SampleZenithAngleWithCos2Distribution()
//------------------------------------------------------------------------------
{
  // cos²θ分布に従った天頂角をサンプリング
  // 確率密度関数: f(θ) ∝ cos²(θ) * sin(θ) (0 ≤ θ ≤ θ_max)
  // 累積分布関数の逆関数を用いてサンプリング
  
  G4double cosTheta_max = std::cos(fMaxZenithAngle);
  G4double u = G4UniformRand(); // [0,1]の一様乱数
  
  // cos²θ分布の累積分布関数の逆関数
  // F(θ) = (1 - cos³(θ)) / (1 - cos³(θ_max))
  // F⁻¹(u) = arccos((1 - u*(1 - cos³(θ_max)))^(1/3))
  G4double cosTheta_max_cubed = cosTheta_max * cosTheta_max * cosTheta_max;
  G4double cosTheta_cubed = 1.0 - u * (1.0 - cosTheta_max_cubed);
  G4double cosTheta = std::pow(cosTheta_cubed, 1.0/3.0);
  
  return std::acos(cosTheta);
}

//------------------------------------------------------------------------------
  G4double PrimaryGenerator::SampleAzimuthAngle()
//------------------------------------------------------------------------------
{
  // 方位角を等方的にサンプリング
  return 2.0 * pi * G4UniformRand();
}

//------------------------------------------------------------------------------
  G4ThreeVector PrimaryGenerator::SamplePointOnPlane()
//------------------------------------------------------------------------------
{
  // Z=15m平面上で X∈[-15,15]m、Y∈[-10,10]m の範囲内でランダムな点を選択
  G4double x = -15.0*m + 30.0*m * G4UniformRand(); // [-10, 10]m
  G4double y = -10.0*m + 15.0*m * G4UniformRand();   // [-5, 5]m
  G4double z = 15.0*m;                               // 固定値
  
  return G4ThreeVector(x, y, z);
}

//------------------------------------------------------------------------------
  void PrimaryGenerator::InitializeLookupTables()
//------------------------------------------------------------------------------
{
  std::cout << "=== ルックアップテーブル初期化開始 ===" << std::endl;
  
  // cos²θ分布のルックアップテーブル作成
  fCosZenithLookup.clear();
  fCosZenithLookup.reserve(fTableSize);
  
  G4double cosTheta_max = std::cos(20.0*deg); // デフォルト最大天頂角
  G4double cosTheta_max_cubed = cosTheta_max * cosTheta_max * cosTheta_max;
  
  for (G4int i = 0; i < fTableSize; ++i) {
    G4double u = static_cast<G4double>(i) / static_cast<G4double>(fTableSize - 1);
    G4double cosTheta_cubed = 1.0 - u * (1.0 - cosTheta_max_cubed);
    G4double cosTheta = std::pow(cosTheta_cubed, 1.0/3.0);
    fCosZenithLookup.push_back(cosTheta);
  }
  
  // 地上宇宙線ミュオンエネルギースペクトルの積分値計算
  G4double gamma_low = fAlpha + 1.0;
  fC_low = fA_low / gamma_low * (std::pow(fE0, gamma_low) - std::pow(fMinEnergy, gamma_low));
  
  G4double gamma_high = -fBeta + 1.0;
  fC_high = fA_high / gamma_high * (std::pow(fMaxEnergy, gamma_high) - std::pow(fE0, gamma_high));
  
  fC_tot = fC_low + fC_high;
  
  // エネルギー分布のルックアップテーブル作成（二重べき乗則分布）
  fEnergyLookup.clear();
  fEnergyLookup.reserve(fTableSize);
  
  for (G4int i = 0; i < fTableSize; ++i) {
    G4double u = static_cast<G4double>(i) / static_cast<G4double>(fTableSize - 1);
    G4double target = u * fC_tot;
    G4double energy;
    
    if (target <= fC_low) {
      // 低エネルギー側 (E_min <= E <= E0)
      G4double u_low = target / fC_low;
      G4double term = u_low * gamma_low * fC_low / fA_low + std::pow(fMinEnergy, gamma_low);
      energy = std::pow(term, 1.0/gamma_low);
    } else {
      // 高エネルギー側 (E0 < E <= E_max)
      G4double u_high = (target - fC_low) / fC_high;
      G4double term = u_high * gamma_high * fC_high / fA_high + std::pow(fE0, gamma_high);
      energy = std::pow(term, 1.0/gamma_high);
    }
    fEnergyLookup.push_back(energy);
  }
  
  fTablesInitialized = true;
  std::cout << "=== ルックアップテーブル初期化完了 ===" << std::endl;
  std::cout << "    天頂角テーブルサイズ: " << fCosZenithLookup.size() << std::endl;
  std::cout << "    エネルギーテーブルサイズ: " << fEnergyLookup.size() << std::endl;
  std::cout << "    エネルギー範囲: " << fMinEnergy/MeV << " MeV - " << fMaxEnergy/MeV << " MeV" << std::endl;
  std::cout << "    ピボットエネルギー: " << fE0/MeV << " MeV" << std::endl;
  std::cout << "    低エネ側指数: " << fAlpha << ", 高エネ側指数: -" << fBeta << std::endl;
  std::cout << "    積分値比 C_low:C_high = " << fC_low/fC_tot*100 << "%:" << fC_high/fC_tot*100 << "%" << std::endl;
}

//------------------------------------------------------------------------------
  G4double PrimaryGenerator::SampleCosmicRayMuonEnergy()
//------------------------------------------------------------------------------
{
  // ルックアップテーブルを使用した高速エネルギーサンプリング
  // 二重べき乗則分布に従う地上宇宙線ミュオンエネルギー
  // 低エネ側: dN/dE ∝ E^α (α=0.71), 高エネ側: dN/dE ∝ E^-β (β=1.03)
  // ピボットエネルギー: 357 MeV
  
  G4double u = G4UniformRand();
  G4int index = static_cast<G4int>(u * (fTableSize - 1));
  
  // 線形補間による精度向上
  if (index < fTableSize - 1) {
    G4double fraction = u * (fTableSize - 1) - index;
    G4double energy1 = fEnergyLookup[index];
    G4double energy2 = fEnergyLookup[index + 1];
    return energy1 + fraction * (energy2 - energy1);
  } else {
    return fEnergyLookup[fTableSize - 1];
  }
}

//------------------------------------------------------------------------------
  G4ThreeVector PrimaryGenerator::SampleDirectionWithCos2Distribution()
//------------------------------------------------------------------------------
{
  // (0,0,-1)方向を基準としたcos²θ分布の方向ベクトルを計算
  G4double theta = SampleZenithAngleWithCos2Distribution();
  G4double phi = SampleAzimuthAngle();
  
  // 球面座標から直交座標への変換
  // 基準方向が(0,0,-1)なので、Z軸の負の方向を基準とする
  G4double sinTheta = std::sin(theta);
  G4double cosTheta = std::cos(theta);
  G4double sinPhi = std::sin(phi);
  G4double cosPhi = std::cos(phi);
  
  // (0,0,-1)を基準とした座標系での方向ベクトル
  G4double x = sinTheta * cosPhi;
  G4double y = sinTheta * sinPhi;
  G4double z = -cosTheta;  // Z軸の負の方向が基準
  
  return G4ThreeVector(x, y, z);
}
