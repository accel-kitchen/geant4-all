//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// PrimaryGeneratorMessenger.cc
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#include "PrimaryGeneratorMessenger.hh"
#include "PrimaryGenerator.hh"
#include "G4UIdirectory.hh"
#include "G4UIcmdWithABool.hh"
#include "G4UIcmdWithADoubleAndUnit.hh"
#include "G4SystemOfUnits.hh"

//------------------------------------------------------------------------------
PrimaryGeneratorMessenger::PrimaryGeneratorMessenger(PrimaryGenerator* primaryGenerator)
: G4UImessenger(),
  fPrimaryGenerator(primaryGenerator),
  fPrimaryGeneratorDir(nullptr),
  fUseZenithDistributionCmd(nullptr),
  fMaxZenithAngleCmd(nullptr),
  fSphereRadiusCmd(nullptr),
  fCircleRadiusCmd(nullptr)
//------------------------------------------------------------------------------
{
  // UIディレクトリを作成
  fPrimaryGeneratorDir = new G4UIdirectory("/primaryGenerator/");
  fPrimaryGeneratorDir->SetGuidance("Primary generator control commands");

  // useZenithDistributionコマンド
  fUseZenithDistributionCmd = new G4UIcmdWithABool("/primaryGenerator/useZenithDistribution", this);
  fUseZenithDistributionCmd->SetGuidance("Enable/disable zenith distribution");
  fUseZenithDistributionCmd->SetParameterName("flag", false);
  fUseZenithDistributionCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

  // maxZenithAngleコマンド
  fMaxZenithAngleCmd = new G4UIcmdWithADoubleAndUnit("/primaryGenerator/maxZenithAngle", this);
  fMaxZenithAngleCmd->SetGuidance("Set maximum zenith angle");
  fMaxZenithAngleCmd->SetParameterName("angle", false);
  fMaxZenithAngleCmd->SetUnitCategory("Angle");
  fMaxZenithAngleCmd->SetDefaultUnit("deg");
  fMaxZenithAngleCmd->SetRange("angle>=0 && angle<=90");
  fMaxZenithAngleCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

  // sphereRadiusコマンド
  fSphereRadiusCmd = new G4UIcmdWithADoubleAndUnit("/primaryGenerator/sphereRadius", this);
  fSphereRadiusCmd->SetGuidance("Set sphere radius");
  fSphereRadiusCmd->SetParameterName("radius", false);
  fSphereRadiusCmd->SetUnitCategory("Length");
  fSphereRadiusCmd->SetDefaultUnit("m");
  fSphereRadiusCmd->SetRange("radius>0");
  fSphereRadiusCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

  // circleRadiusコマンド
  fCircleRadiusCmd = new G4UIcmdWithADoubleAndUnit("/primaryGenerator/circleRadius", this);
  fCircleRadiusCmd->SetGuidance("Set tangent circle radius");
  fCircleRadiusCmd->SetParameterName("radius", false);
  fCircleRadiusCmd->SetUnitCategory("Length");
  fCircleRadiusCmd->SetDefaultUnit("m");
  fCircleRadiusCmd->SetRange("radius>0");
  fCircleRadiusCmd->AvailableForStates(G4State_PreInit, G4State_Idle);
}

//------------------------------------------------------------------------------
PrimaryGeneratorMessenger::~PrimaryGeneratorMessenger()
//------------------------------------------------------------------------------
{
  delete fUseZenithDistributionCmd;
  delete fMaxZenithAngleCmd;
  delete fSphereRadiusCmd;
  delete fCircleRadiusCmd;
  delete fPrimaryGeneratorDir;
}

//------------------------------------------------------------------------------
void PrimaryGeneratorMessenger::SetNewValue(G4UIcommand* command, G4String newValue)
//------------------------------------------------------------------------------
{
  if (command == fUseZenithDistributionCmd) {
    fPrimaryGenerator->SetUseZenithDistribution(fUseZenithDistributionCmd->GetNewBoolValue(newValue));
  }
  else if (command == fMaxZenithAngleCmd) {
    fPrimaryGenerator->SetMaxZenithAngle(fMaxZenithAngleCmd->GetNewDoubleValue(newValue));
  }
  else if (command == fSphereRadiusCmd) {
    fPrimaryGenerator->SetSphereRadius(fSphereRadiusCmd->GetNewDoubleValue(newValue));
  }
  else if (command == fCircleRadiusCmd) {
    fPrimaryGenerator->SetCircleRadius(fCircleRadiusCmd->GetNewDoubleValue(newValue));
  }
}
