//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// MuoMessenger.hh
// /muo/ コマンド。値は MuoConfig に書き込む (/run/initialize の前に実行すること)。
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifndef MuoMessenger_h
#define MuoMessenger_h 1

#include "G4UImessenger.hh"
#include "globals.hh"
#include <vector>

class G4UIcommand;
class G4UIdirectory;
class G4UIcmdWithAString;
class G4UIcmdWithABool;
class G4UIcmdWithAnInteger;
class G4UIcmdWithADouble;
class G4UIcmdWithADoubleAndUnit;

class MuoMessenger : public G4UImessenger {
public:
  MuoMessenger();
  ~MuoMessenger() override;
  void SetNewValue(G4UIcommand* cmd, G4String value) override;

private:
  G4UIcmdWithADoubleAndUnit* Quantity(const G4String& path, const G4String& guide,
                                      const G4String& category, const G4String& defUnit, G4double* target);
  G4UIcmdWithAString* Text(const G4String& path, const G4String& guide, G4String* target);
  G4UIcmdWithABool* Flag(const G4String& path, const G4String& guide, G4bool* target);
  G4UIcmdWithADouble* Number(const G4String& path, const G4String& guide, G4double* target);

  std::vector<G4UIdirectory*> fDirs;
  std::vector<G4UIcommand*> fAll;
  std::vector<std::pair<G4UIcmdWithADoubleAndUnit*, G4double*>> fQuantities;
  std::vector<std::pair<G4UIcmdWithAString*, G4String*>> fTexts;
  std::vector<std::pair<G4UIcmdWithABool*, G4bool*>> fFlags;
  std::vector<std::pair<G4UIcmdWithADouble*, G4double*>> fNumbers;

  G4UIcmdWithAString* fStageCmd = nullptr;
  G4UIcmdWithAnInteger* fSeedCmd = nullptr;
  G4UIcommand* fNxyCmd = nullptr;
  G4UIcommand* fAddLayerCmd = nullptr;
  G4UIcommand* fClearLayersCmd = nullptr;
  G4UIcommand* fCavityCmd = nullptr;
};

#endif
