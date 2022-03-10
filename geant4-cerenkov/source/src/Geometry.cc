//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Geometry.cc
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#include "Geometry.hh"

#include "G4Box.hh"
#include "G4Trd.hh"
#include "G4Tubs.hh"
#include "G4Orb.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4PVReplica.hh"
#include "G4VPhysicalVolume.hh"

#include "G4ThreeVector.hh"
#include "G4RotationMatrix.hh"
#include "G4Transform3D.hh"
#include "G4NistManager.hh"
#include "G4VisAttributes.hh"
#include "G4SystemOfUnits.hh"
#include "G4SubtractionSolid.hh"
#include "SensitiveVolume.hh"
#include "G4SDManager.hh"


//------------------------------------------------------------------------------
// block class 
class Block{
  public:
    Block(G4String namebox, G4double pos_X, G4double pos_Y, G4double pos_Z, G4double size_X, G4double size_Y, G4double size_Z, G4int copyNum, G4int nDiv_z, G4String materi, G4PVPlacement*& physVol_World);
};
Block::Block(G4String namebox, G4double pos_X, G4double pos_Y, G4double pos_Z, G4double size_X, G4double size_Y, G4double size_Z, G4int copyNum, G4int nDiv_z, G4String materi, G4PVPlacement*& physVol_World)
{


  G4NistManager* nistManager = G4NistManager::Instance();
  nistManager->FindOrBuildMaterial("G4_Pb");
  nistManager->FindOrBuildMaterial("G4_GLASS_LEAD"); //追加。
G4Material* absorberMaterial = G4Material::GetMaterial("G4_GLASS_LEAD");


  G4double a, z, density;
  G4String name, symbol;
  G4int nel;


//  ------------- Elements -------------
  a = 1.01*g/mole;
  G4Element* elH  = new G4Element(name="Hydrogen", symbol="H", z=1., a);

  a = 12.01*g/mole;
  G4Element* elC  = new G4Element(name="Carbon",   symbol="C", z=6., a);

  a = 14.01*g/mole;
  G4Element* elN  = new G4Element(name="Nitrogen", symbol="N", z=7., a);

  a = 16.00*g/mole;
  G4Element* elO  = new G4Element(name="Oxygen",   symbol="O", z=8., a);

  a = 28.09*g/mole;
  G4Element* elSi = new G4Element(name="Silicon", symbol="Si", z=14., a);

    density = 1.19*g/cm3;
    G4Material* Acrylic = new G4Material(name="Acrylic", density, nel=3);
    Acrylic->AddElement(elC, 5);
    Acrylic->AddElement(elH, 8);
    Acrylic->AddElement(elO, 2);

 // ------------ Generate & Add Material Properties Table ------------
 //
   const G4int nEntries = 2;
 
   G4double PhotonEnergy[nEntries] =
             { 2.034*eV, 4.136*eV };
 //
 // Water
 //
   G4double RefractiveIndex1[nEntries] =
             { 1.49, 1.49};
 
   G4double Absorption1[nEntries] =
            {10*m,  10*m};
 
   G4double ScintilFast[nEntries] =
             { 1.00,1.00 };
   G4double ScintilSlow[nEntries] =
             { 0.01, 4.00 };
   G4MaterialPropertiesTable* myMPT1 = new G4MaterialPropertiesTable();
   myMPT1->AddProperty("RINDEX",       PhotonEnergy, RefractiveIndex1,nEntries);
   myMPT1->AddProperty("ABSLENGTH",    PhotonEnergy, Absorption1,     nEntries);
   myMPT1->AddProperty("FASTCOMPONENT",PhotonEnergy, ScintilFast,     nEntries);
   myMPT1->AddProperty("SLOWCOMPONENT",PhotonEnergy, ScintilSlow,     nEntries);
 
   myMPT1->AddConstProperty("SCINTILLATIONYIELD",0./MeV);
   myMPT1->AddConstProperty("RESOLUTIONSCALE",1.0);
   myMPT1->AddConstProperty("FASTTIMECONSTANT", 1.*ns);
   myMPT1->AddConstProperty("SLOWTIMECONSTANT",10.*ns);
   myMPT1->AddConstProperty("YIELDRATIO",0.8);
 
   Acrylic->SetMaterialPropertiesTable(myMPT1);
   auto solid_BlockEnvG = new G4Box("Solid_BlockEnvG", size_X/2.0, size_Y/2.0, size_Z/2.0);

   auto logVol_BlockEnvG = new G4LogicalVolume(solid_BlockEnvG, Acrylic, "LogVol_BlockEnvG");
   auto threeVect_LogV_BlockEnvG = G4ThreeVector(pos_X, pos_Y, pos_Z);
   auto rotMtrx_LogV_BlockEnvG = G4RotationMatrix();
   auto trans3D_LogV_BlockEnvG = G4Transform3D(rotMtrx_LogV_BlockEnvG, threeVect_LogV_BlockEnvG);
   new G4PVPlacement(trans3D_LogV_BlockEnvG, "PhysVol_BlockEnvG", logVol_BlockEnvG, physVol_World, 
                     false, copyNum);
    //Generate Local envelope of Water/co
   auto solid_BlockEnvL = new G4Box("Solid_BlockEnvL", size_X/2.0, size_Y/2.0, size_Z/nDiv_z/2.0);
   auto logVol_BlockEnvL = new G4LogicalVolume(solid_BlockEnvL, Acrylic, "LogVol_BlockEnvL");
   logVol_BlockEnvL->SetVisAttributes (G4VisAttributes::Invisible);
   new G4PVReplica("PhysVol_BlockEnvL", logVol_BlockEnvL, logVol_BlockEnvG, kZAxis, nDiv_z, size_Z/nDiv_z); 
  /////// Sensitive Detector
    auto BlockSV = new SensitiveVolume("SensitiveVolume");
    logVol_BlockEnvL->SetSensitiveDetector(BlockSV);         // Add sensitivity to the logical volume
    auto SDmanBlock = G4SDManager::GetSDMpointer();
    SDmanBlock->AddNewDetector(BlockSV);
}
//------------------------------------------------------------------------------
  Geometry::Geometry() {}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
  Geometry::~Geometry() {}
//------------------------------------------------------------------------------
/////// 実装必須のConstruct()
// 最後に物理ワールドへのポインタを返す
//------------------------------------------------------------------------------
  G4VPhysicalVolume* Geometry::Construct()
//------------------------------------------------------------------------------
{
  /////// Generate world
   auto materi_Man = G4NistManager::Instance();//Get pointer "material manager"
   auto solid_World = new G4Box("Solid_World", 1.0*m/2.0, 1.0*m/2.0, 1.0*m/2.0); //define world volume
   auto materi_World = materi_Man->FindOrBuildMaterial("G4_AIR");
   auto logVol_World = new G4LogicalVolume(solid_World, materi_World, "LogVol_World");
   logVol_World->SetVisAttributes (G4VisAttributes::Invisible);
   auto physVol_World  =  new G4PVPlacement(G4Transform3D(), "PhysVol_World", logVol_World, 0, false, 0);


  /////// put volumess in the world



//arguments are follows:
// name, pos_X, pos_Y, pos_Z, size_X, size_Y, size_Z, copyNum, nDiv_z, material ,physVol_World
Block Block1("Block1", 0.0 * cm, 0.0 * cm, 20 * cm, 50* mm, 50 * mm, 150 * mm, 1 ,1, "" ,physVol_World); 



/////// Return the physical world
   return physVol_World;
}