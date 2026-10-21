"""§2 の概算 (Plan.html) を再現する解析式。Geant4 を使わない。

- 海面のミュオン微分強度: Gaisser 式 + Guan et al. (2015) の低エネルギー・大天頂角の補正
  (source/include/MuonFlux.hh と同じ式)
- 土中のエネルギー損失: dE/dX = a + bE (標準岩石の典型値) から、透過に要る最小エネルギーを求める
- 2 枚の正方形 (一辺 a、間隔 D) の同時計数率: 水平投影の重なり面積 × cosθ を天頂角 60° まで積分

実行すると、Plan.html §2 の表に使った数値を順に出力する (数秒)。
V3 (土なし 2 層テレスコープ) の期待値は rate(0.6, 0.3, 0) = 28.46 Hz
(E >= 0.1 GeV、上限なし)。MC と比べるときは、同じ E 範囲 (例: 0.15〜500 GeV) で
Iint の積分範囲を合わせること。
依存: numpy (np.trapz を使うので numpy 1.x)。
"""
import numpy as np
# --- sea-level muon spectrum: Gaisser with Guan et al.(2015) low-E / large-angle modification
def cos_star(c):
    p1,p2,p3,p4,p5 = 0.102573,-0.068287,0.958633,0.0407253,0.817285
    return np.sqrt((c**2 + p1**2 + p2*c**p3 + p4*c**p5)/(1+p1**2+p2+p4))
def dIdE(E,c):  # cm^-2 s^-1 sr^-1 GeV^-1
    cs = cos_star(c)
    Ee = E*(1+3.64/(E*cs**1.29))
    return 0.14*Ee**-2.7*(1/(1+1.1*E*cs/115)+0.054/(1+1.1*E*cs/850))
Eg = np.logspace(-1,4,4000)
def Iint(c,Emin):
    m = Eg>=Emin
    return np.trapz(dIdE(Eg[m],c),Eg[m])  # cm^-2 s^-1 sr^-1
# vertical integral >1 GeV sanity (expect ~70 m^-2 s^-1 sr^-1 = 7e-3 cm^-2..)
print("I_v(>1GeV) [m^-2 s^-1 sr^-1] =", Iint(1.0,1.0)*1e4)
# horizontal-plane flux: 2pi int I(theta) cos theta sinθ dθ
th = np.linspace(0,np.pi/2,400)
Ih = [Iint(np.cos(t),0.1) for t in th]
Fh = 2*np.pi*np.trapz(np.array(Ih)*np.cos(th)*np.sin(th),th)
print("horizontal flux E>0.1GeV [cm^-2 min^-1] =", Fh*60)
# --- energy threshold to cross X g/cm2 of soil (CSDA with a+bE)
a,b = 2.2e-3, 4.0e-6   # GeV cm2/g , cm2/g
def Emin(X): return (a/b)*(np.exp(b*X)-1)
rho = 1.6
for h in [0,3,5,10,20,50]:
    X = rho*h*100
    print(f"h={h:3d} m  X={X:6.0f} g/cm2  Emin(vert)={Emin(X):6.2f} GeV  Iv={Iint(1.0,max(Emin(X),0.1))*1e4:7.2f} m-2s-1sr-1")
# --- telescope rate: two squares side a, gap D, overburden h
def rate(a_m,D_m,h_m,rho=1.6,nth=90,nph=72,thmax=np.deg2rad(60)):
    ths = np.linspace(0.0005,thmax,nth); phs = np.linspace(0,2*np.pi,nph,endpoint=False)
    tot=0.0
    for t in ths:
        c=np.cos(t); X=rho*h_m*100/c
        I=Iint(c,max(Emin(X),0.1))*1e4  # m^-2 s^-1 sr^-1
        sh=D_m*np.tan(t)
        ov=np.maximum(a_m-sh*np.abs(np.cos(phs)),0)*np.maximum(a_m-sh*np.abs(np.sin(phs)),0)
        tot+= I*c*np.sin(t)*ov.mean()*2*np.pi*(ths[1]-ths[0])
    return tot  # s^-1
print("\nCoincidence rate [Hz] / counts per 2 h (a=0.6m)")
for D in [0.2,0.3,0.4]:
    row=[]
    for h in [0,3,5,10,20,50]:
        r=rate(0.6,D,h); row.append(f"h={h}:{r:6.2f}Hz/{r*7200:8.0f}")
    print(f"D={D}", " | ".join(row))
print("\n5 cm and 10 cm and 20 cm square prototype, D=0.1/0.15 m, surface, per 2h")
for a_,D_ in [(0.05,0.05),(0.10,0.10),(0.20,0.15),(0.6,0.3)]:
    r=rate(a_,D_,0.0); print(a_,D_, f"{r:.4f} Hz  {r*7200:.0f}/2h  {r*86400:.0f}/day")
# --- vertical bin: 3deg x 3deg around vertical, a=0.6
dOm=np.deg2rad(3)**2
for h in [3,5,10,20,50]:
    X=rho*h*100; Iv=Iint(1.0,Emin(X))*1e4
    print(f"h={h}: counts in 3x3deg bin near vertical in 2h = {Iv*0.36*dOm*7200:.1f}")
# --- contrast: d ln I / d ln X at vertical
for h in [3,5,10,20,50]:
    X1=rho*h*100*0.99; X2=rho*h*100*1.01
    g=-(np.log(Iint(1.0,Emin(X2)))-np.log(Iint(1.0,Emin(X1))))/(np.log(X2)-np.log(X1))
    print(f"h={h}: gamma_eff(dlnI/dlnX)={g:.2f}")
# angular resolution from pitch: sigma_theta = sqrt2 p /(sqrt12 D) (vertical)
for p in [0.02,0.01]:
    for D in [0.15,0.2,0.3,0.4]:
        print(f"pitch {p*100:.0f}cm D={D*100:.0f}cm sigma_theta(vertical)={np.degrees(np.sqrt(2)*p/(np.sqrt(12)*D)):.2f} deg")
# mass budget
print("scint mass per 60x60x1cm layer [kg]=", 60*60*1*1.032/1000)
