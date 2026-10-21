//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// PrimaryGenerator.hh
// 検出器から見た方向を先に選び、地表までさかのぼって発生させる重み付きの一次粒子生成
// (設計判断 D2)。Stage B では位相空間ファイルから粒子を再生する。
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifndef PrimaryGenerator_h
#define PrimaryGenerator_h 1

#include "G4VUserPrimaryGeneratorAction.hh"

class G4ParticleGun;
class G4ParticleDefinition;
class G4Event;

class PrimaryGenerator : public G4VUserPrimaryGeneratorAction {
public:
  PrimaryGenerator();
  ~PrimaryGenerator() override;
  void GeneratePrimaries(G4Event*) override;

private:
  void Sample(G4Event*);
  void Replay(G4Event*);

  G4ParticleGun* fGun;
  G4ParticleDefinition* fMuMinus;
  G4ParticleDefinition* fMuPlus;
};

#endif
