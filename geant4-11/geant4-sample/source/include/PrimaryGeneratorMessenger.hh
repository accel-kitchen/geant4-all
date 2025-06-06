//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// PrimaryGeneratorMessenger.hh
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifndef PrimaryGeneratorMessenger_h
#define PrimaryGeneratorMessenger_h 1

#include "G4UImessenger.hh"

class PrimaryGenerator;
class G4UIdirectory;
class G4UIcmdWithABool;
class G4UIcmdWithADoubleAndUnit;

//------------------------------------------------------------------------------
class PrimaryGeneratorMessenger : public G4UImessenger
//------------------------------------------------------------------------------
{
  public:
    PrimaryGeneratorMessenger(PrimaryGenerator* primaryGenerator);
   ~PrimaryGeneratorMessenger() override;

  public:
    void SetNewValue(G4UIcommand* command, G4String newValue) override;

  private:
    PrimaryGenerator* fPrimaryGenerator;
    
    G4UIdirectory* fPrimaryGeneratorDir;
    G4UIcmdWithABool* fUseZenithDistributionCmd;
    G4UIcmdWithADoubleAndUnit* fMaxZenithAngleCmd;
    G4UIcmdWithADoubleAndUnit* fSphereRadiusCmd;
    G4UIcmdWithADoubleAndUnit* fCircleRadiusCmd;
};

#endif
