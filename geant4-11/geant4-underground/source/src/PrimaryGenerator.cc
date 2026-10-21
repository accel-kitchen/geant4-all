//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// PrimaryGenerator.cc
//
// 重み w1 [Hz/イベント]:
//   w1 = I(E,θ) / q(E) · 2π(1 − cosθmax) · π r_t²
//   I: 微分強度 [cm^-2 s^-1 sr^-1 GeV^-1]、q(E): 発生に使ったエネルギー分布 (∝ E^-index)
//   cosθ と φ は一様 (dΩ = dcosθ dφ)。測定レート = Σ w1 / N_generated。
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#include "PrimaryGenerator.hh"
#include "MuoConfig.hh"
#include "MuoEventInfo.hh"
#include "MuoRun.hh"
#include "MuonFlux.hh"
#include "PhaseSpaceStore.hh"

#include "G4Event.hh"
#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "G4PhysicalConstants.hh"
#include "G4RunManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4ThreeVector.hh"
#include "Randomize.hh"
#include <cmath>

namespace {
  MuoRun* CurrentRun()
  {
    return static_cast<MuoRun*>(G4RunManager::GetRunManager()->GetNonConstCurrentRun());
  }
}

//------------------------------------------------------------------------------
  PrimaryGenerator::PrimaryGenerator()
  : fGun(new G4ParticleGun(1))
//------------------------------------------------------------------------------
{
  auto* table = G4ParticleTable::GetParticleTable();
  fMuMinus = table->FindParticle("mu-");
  fMuPlus  = table->FindParticle("mu+");
  fGun->SetParticleDefinition(fMuMinus);
}

//------------------------------------------------------------------------------
  PrimaryGenerator::~PrimaryGenerator()
//------------------------------------------------------------------------------
{
  delete fGun;
}

//------------------------------------------------------------------------------
  void PrimaryGenerator::GeneratePrimaries(G4Event* event)
//------------------------------------------------------------------------------
{
  if (MuoConfig::Get().stage == MuoStage::B) Replay(event);
  else Sample(event);
}

//------------------------------------------------------------------------------
  void PrimaryGenerator::Sample(G4Event* event)
//------------------------------------------------------------------------------
{
  const auto& c = MuoConfig::Get();

  // 方向: cosθ 一様、φ 一様。ミュオンは下向きに進む
  const G4double cosMax = std::cos(c.zenithMax);
  const G4double cth = cosMax + (1.0 - cosMax)*G4UniformRand();
  const G4double sth = std::sqrt(1.0 - cth*cth);
  const G4double ph = twopi*G4UniformRand();
  const G4ThreeVector dir(sth*std::cos(ph), sth*std::sin(ph), -cth);

  // エネルギー: q(E) ∝ E^-index (index=1 なら ln E 一様)
  const G4double eLo = c.eLow/GeV, eHi = c.eHigh/GeV;
  const G4double u = G4UniformRand();
  G4double E, qInv;
  if (std::fabs(c.energyIndex - 1.0) < 1.0e-6) {
    E = eLo*std::pow(eHi/eLo, u);
    qInv = E*std::log(eHi/eLo);
  } else {
    const G4double a1 = c.energyIndex - 1.0;
    const G4double s = std::pow(eLo/eHi, a1);
    E = eLo*std::pow(1.0 - u*(1.0 - s), -1.0/a1);
    qInv = (1.0 - s)*std::pow(E, c.energyIndex)/(a1*std::pow(eLo, a1));
  }

  const G4double rtCm = c.targetRadius/cm;
  const G4double w1 = MuonFlux::DiffIntensity(E, cth)*qInv
                    * twopi*(1.0 - cosMax) * pi*rtCm*rtCm;

  // 検出器基準点を中心とし、進行方向に垂直な円盤の上の点を選ぶ
  const G4ThreeVector e1(cth*std::cos(ph), cth*std::sin(ph), sth);
  const G4ThreeVector e2(-std::sin(ph), std::cos(ph), 0.0);
  const G4double r = c.targetRadius*std::sqrt(G4UniformRand());
  const G4double a = twopi*G4UniformRand();
  const G4ThreeVector onDisk = r*std::cos(a)*e1 + r*std::sin(a)*e2;

  // その点から進行方向をさかのぼり、z = SourceTopZ の面を出発点にする
  const G4double s = (c.SourceTopZ() - onDisk.z())/cth;
  const G4ThreeVector start = onDisk - s*dir;

  auto* pd = (G4UniformRand() < c.plusFraction) ? fMuPlus : fMuMinus;
  fGun->SetParticleDefinition(pd);
  fGun->SetParticleEnergy(E*GeV - pd->GetPDGMass());
  fGun->SetParticlePosition(start);
  fGun->SetParticleMomentumDirection(dir);
  fGun->SetParticleTime(0.0);
  fGun->GeneratePrimaryVertex(event);

  event->SetUserInformation(new MuoEventInfo(event->GetEventID(), w1, E, cth, ph));
  CurrentRun()->AddGenerated(w1);
}

//------------------------------------------------------------------------------
  void PrimaryGenerator::Replay(G4Event* event)
//------------------------------------------------------------------------------
{
  const auto& groups = PhaseSpaceStore::Get().Groups();
  if (groups.empty()) return;
  const auto& g = groups[static_cast<size_t>(event->GetEventID()) % groups.size()];

  auto* table = G4ParticleTable::GetParticleTable();
  for (const auto& r : g.rows) {
    auto* pd = table->FindParticle(r.pdg);
    if (!pd) continue;
    fGun->SetParticleDefinition(pd);
    fGun->SetParticleEnergy(r.ekin*MeV);
    fGun->SetParticlePosition(G4ThreeVector(r.x*mm, r.y*mm, r.z*mm));
    fGun->SetParticleMomentumDirection(G4ThreeVector(r.ux, r.uy, r.uz));
    fGun->SetParticleTime(r.t*ns);
    fGun->GeneratePrimaryVertex(event);
  }
  event->SetUserInformation(new MuoEventInfo(g.srcEvent, g.w1, g.E0, g.cth0, g.ph0));
  CurrentRun()->AddGenerated(g.w1);
}
