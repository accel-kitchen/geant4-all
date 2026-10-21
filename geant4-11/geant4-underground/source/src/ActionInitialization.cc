//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// ActionInitialization.cc
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#include "ActionInitialization.hh"
#include "PrimaryGenerator.hh"
#include "RunAction.hh"

//------------------------------------------------------------------------------
  void ActionInitialization::BuildForMaster() const
//------------------------------------------------------------------------------
{
  SetUserAction(new RunAction);
}

//------------------------------------------------------------------------------
  void ActionInitialization::Build() const
//------------------------------------------------------------------------------
{
  SetUserAction(new PrimaryGenerator);
  SetUserAction(new RunAction);
}
