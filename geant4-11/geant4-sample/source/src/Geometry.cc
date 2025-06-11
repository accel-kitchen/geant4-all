//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Geometry.cc
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#include "Geometry.hh"
#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
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
    Block(G4String name, G4double pos_X, G4double pos_Y, G4double pos_Z, G4double size_X, G4double size_Y, G4double size_Z, G4int copyNum, G4int nDiv_z, G4String materi, G4PVPlacement*& physVol_World);
};
Block::Block(G4String name, G4double pos_X, G4double pos_Y, G4double pos_Z, G4double size_X, G4double size_Y, G4double size_Z, G4int copyNum, G4int nDiv_z, G4String materi, G4PVPlacement*& physVol_World)
{
    auto materi_Man = G4NistManager::Instance();
       auto materi_BlockEnvG = materi_Man->FindOrBuildMaterial(materi);
   auto solid_BlockEnvG = new G4Box("Solid_BlockEnvG", size_X/2.0, size_Y/2.0, size_Z/2.0);

   auto logVol_BlockEnvG = new G4LogicalVolume(solid_BlockEnvG, materi_BlockEnvG, "LogVol_BlockEnvG");
   auto threeVect_LogV_BlockEnvG = G4ThreeVector(pos_X, pos_Y, pos_Z);
   auto rotMtrx_LogV_BlockEnvG = G4RotationMatrix();
   auto trans3D_LogV_BlockEnvG = G4Transform3D(rotMtrx_LogV_BlockEnvG, threeVect_LogV_BlockEnvG);
   new G4PVPlacement(trans3D_LogV_BlockEnvG, "PhysVol_BlockEnvG", logVol_BlockEnvG, physVol_World, 
                     false, copyNum);
    /////// Sensitive Detector
    auto BlockSV = new SensitiveVolume("SensitiveVolume");
    logVol_BlockEnvG->SetSensitiveDetector(BlockSV);         // Add sensitivity to the logical volume
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
   auto solid_World = new G4Box("Solid_World", 10.0*m, 10.0*m, 10.0*m); // 20m×20m×20mのワールド
   auto materi_World = materi_Man->FindOrBuildMaterial("G4_AIR");
   auto logVol_World = new G4LogicalVolume(solid_World, materi_World, "LogVol_World");
   logVol_World->SetVisAttributes ( G4VisAttributes::GetInvisible() );
   auto physVol_World  =  new G4PVPlacement(G4Transform3D(), "PhysVol_World", logVol_World, 0, false, 0);


  /////// 土のブロック (Z=1mから11m、5種類の密度)
   
   // 5つの異なる密度の土材料を作成
   auto materi_Soil1 = new G4Material("Soil1", 1.4*g/cm3, 1);
   materi_Soil1->AddMaterial(materi_Man->FindOrBuildMaterial("G4_CONCRETE"), 1.0);
   
   auto materi_Soil2 = new G4Material("Soil2", 1.5*g/cm3, 1);
   materi_Soil2->AddMaterial(materi_Man->FindOrBuildMaterial("G4_CONCRETE"), 1.0);
   
   auto materi_Soil3 = new G4Material("Soil3", 1.6*g/cm3, 1);
   materi_Soil3->AddMaterial(materi_Man->FindOrBuildMaterial("G4_CONCRETE"), 1.0);
   
   auto materi_Soil4 = new G4Material("Soil4", 1.7*g/cm3, 1);
   materi_Soil4->AddMaterial(materi_Man->FindOrBuildMaterial("G4_CONCRETE"), 1.0);
   
   auto materi_Soil5 = new G4Material("Soil5", 1.8*g/cm3, 1);
   materi_Soil5->AddMaterial(materi_Man->FindOrBuildMaterial("G4_CONCRETE"), 1.0);
   
   // 5つの土ブロックの形状を作成（各3.2m×32m×10m）
   auto solid_SoilBlock1 = new G4Box("Solid_SoilBlock1", 1.6*m, 4.0*m, 5.0*m); // 3.2m×32m×10m
   auto solid_SoilBlock2 = new G4Box("Solid_SoilBlock2", 1.6*m, 4.0*m, 5.0*m);
   auto solid_SoilBlock3 = new G4Box("Solid_SoilBlock3", 1.6*m, 4.0*m, 5.0*m);
   auto solid_SoilBlock4 = new G4Box("Solid_SoilBlock4", 1.6*m, 4.0*m, 5.0*m);
   auto solid_SoilBlock5 = new G4Box("Solid_SoilBlock5", 1.6*m, 4.0*m, 5.0*m);
   
   // 5つの土ブロックの論理ボリュームを作成
   auto logVol_SoilBlock1 = new G4LogicalVolume(solid_SoilBlock1, materi_Soil1, "LogVol_SoilBlock1");
   auto logVol_SoilBlock2 = new G4LogicalVolume(solid_SoilBlock2, materi_Soil2, "LogVol_SoilBlock2");
   auto logVol_SoilBlock3 = new G4LogicalVolume(solid_SoilBlock3, materi_Soil3, "LogVol_SoilBlock3");
   auto logVol_SoilBlock4 = new G4LogicalVolume(solid_SoilBlock4, materi_Soil4, "LogVol_SoilBlock4");
   auto logVol_SoilBlock5 = new G4LogicalVolume(solid_SoilBlock5, materi_Soil5, "LogVol_SoilBlock5");
   
   // 土ブロックの可視化設定（密度に応じて色を変更）
   auto visAttr_Soil1 = new G4VisAttributes(G4Colour(0.6, 0.4, 0.2, 0.5)); // 茶色（密度1.4）
   auto visAttr_Soil2 = new G4VisAttributes(G4Colour(0.65, 0.45, 0.25, 0.5)); // 少し明るい茶色（密度1.5）
   auto visAttr_Soil3 = new G4VisAttributes(G4Colour(0.7, 0.5, 0.3, 0.5)); // より明るい茶色（密度1.6）
   auto visAttr_Soil4 = new G4VisAttributes(G4Colour(0.75, 0.55, 0.35, 0.5)); // さらに明るい茶色（密度1.7）
   auto visAttr_Soil5 = new G4VisAttributes(G4Colour(0.8, 0.6, 0.4, 0.5)); // 最も明るい茶色（密度1.8）
   logVol_SoilBlock1->SetVisAttributes(visAttr_Soil1);
   logVol_SoilBlock2->SetVisAttributes(visAttr_Soil2);
   logVol_SoilBlock3->SetVisAttributes(visAttr_Soil3);
   logVol_SoilBlock4->SetVisAttributes(visAttr_Soil4);
   logVol_SoilBlock5->SetVisAttributes(visAttr_Soil5);
   
   // Z=6mの位置に5つのブロックをX方向に1/5ずつ配置
   // ブロック1（密度1.4、中心X=-6.4m）
   auto threeVect_SoilBlock1 = G4ThreeVector(-6.4*m, 0, 6.0*m);
   new G4PVPlacement(0, threeVect_SoilBlock1, "PhysVol_SoilBlock1", logVol_SoilBlock1, physVol_World, false, 0);
   
   // ブロック2（密度1.5、中心X=-3.2m）
   auto threeVect_SoilBlock2 = G4ThreeVector(-3.2*m, 0, 6.0*m);
   new G4PVPlacement(0, threeVect_SoilBlock2, "PhysVol_SoilBlock2", logVol_SoilBlock2, physVol_World, false, 1);
   
   // ブロック3（密度1.6、中心X=0m）
   auto threeVect_SoilBlock3 = G4ThreeVector(0*m, 0, 6.0*m);
   new G4PVPlacement(0, threeVect_SoilBlock3, "PhysVol_SoilBlock3", logVol_SoilBlock3, physVol_World, false, 2);
   
   // ブロック4（密度1.7、中心X=3.2m）
   auto threeVect_SoilBlock4 = G4ThreeVector(3.2*m, 0, 6.0*m);
   new G4PVPlacement(0, threeVect_SoilBlock4, "PhysVol_SoilBlock4", logVol_SoilBlock4, physVol_World, false, 3);
   
   // ブロック5（密度1.8、中心X=6.4m）
   auto threeVect_SoilBlock5 = G4ThreeVector(6.4*m, 0, 6.0*m);
   new G4PVPlacement(0, threeVect_SoilBlock5, "PhysVol_SoilBlock5", logVol_SoilBlock5, physVol_World, false, 4);

  /////// 検出器アレイ (コインシデンス測定用上下2段)
   // プラスチックシンチレータ材料
   auto materi_Detector = materi_Man->FindOrBuildMaterial("G4_PLASTIC_SC_VINYLTOLUENE");
   auto solid_Detector = new G4Box("Solid_Detector", 25.0*cm, 25.0*cm, 0.5*cm); // 10cm×10cm×1cm
   auto logVol_Detector = new G4LogicalVolume(solid_Detector, materi_Detector, "LogVol_Detector");
   
   // 検出器の可視化設定
   auto visAttr_Detector = new G4VisAttributes(G4Colour(0.0, 1.0, 0.0, 0.8)); // 緑色
   logVol_Detector->SetVisAttributes(visAttr_Detector);
   
   // 検出器に感度を設定
   auto DetectorSV = new SensitiveVolume("DetectorSV");
   logVol_Detector->SetSensitiveDetector(DetectorSV);
   auto SDmanDetector = G4SDManager::GetSDMpointer();
   SDmanDetector->AddNewDetector(DetectorSV);
   
   // X=-3mから3mまで10cm間隔で上下2段に配置
   // CopyNo と検出器位置の対応関係:
   // 下段 (Z=0):
   // CopyNo  0: X = -3.0m, Z = 0
   // CopyNo  1: X = -2.9m, Z = 0
   // ...
   // CopyNo 60: X =  3.0m, Z = 0
   // 上段 (Z=10cm):
   // CopyNo 61: X = -3.0m, Z = 10cm
   // CopyNo 62: X = -2.9m, Z = 10cm
   // ...
   // CopyNo 121: X = 3.0m, Z = 10cm
   
   G4int detectorCount = 0;
   
   // 下段の検出器 (Z=0)
   for (G4double x = -6.0*m; x <= 6.0*m; x += 0.6*m) {
     auto threeVect_Detector = G4ThreeVector(x, 0, 0);
     new G4PVPlacement(0, threeVect_Detector, "PhysVol_Detector_Lower", logVol_Detector, 
                       physVol_World, false, detectorCount);
     detectorCount++;
   }
   
   // 上段の検出器 (Z=10cm)
   for (G4double x = -6.0*m; x <= 6.0*m; x += 0.6*m) {
     auto threeVect_Detector = G4ThreeVector(x, 0, 50.0*cm);
     new G4PVPlacement(0, threeVect_Detector, "PhysVol_Detector_Upper", logVol_Detector, 
                       physVol_World, false, detectorCount);
     detectorCount++;
   }

/////// Return the physical world
   return physVol_World;
}
