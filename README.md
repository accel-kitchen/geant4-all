導入の仕方
- Virtual Machineを立ち上げる。
- home directoryで`git clone https://github.com/accel-kitchen/geant4-all.git` としてサンプルファイルをコピーする。
- `rm -rf bin build`
- `mkdir bin build`
- `cd ./geant4-all/simple/TestBench`
- `sh build.sh`と打つとGeant4のGUIが立ち上がる。
- 下部のコンソールに`/control/execute run1.mac`とうってビームが打たれたらOK.
