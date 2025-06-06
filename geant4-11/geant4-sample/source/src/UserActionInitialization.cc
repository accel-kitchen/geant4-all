//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// UserActionInitialization.cc
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#include "UserActionInitialization.hh"
#include "PrimaryGenerator.hh"
#include "G4SystemOfUnits.hh"

//------------------------------------------------------------------------------
  UserActionInitialization::UserActionInitialization()
  : G4VUserActionInitialization()
//------------------------------------------------------------------------------
{}

//------------------------------------------------------------------------------
  UserActionInitialization::~UserActionInitialization()
//------------------------------------------------------------------------------
{}

//------------------------------------------------------------------------------
  void UserActionInitialization::Build() const
//------------------------------------------------------------------------------
{
  PrimaryGenerator* primaryGenerator = new PrimaryGenerator{};
  
  // 天頂角度分布を有効化
  primaryGenerator->SetUseZenithDistribution(true);
  primaryGenerator->SetMaxZenithAngle(60.0 * deg);
  
  SetUserAction( primaryGenerator );
}
