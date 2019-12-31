bCoreを物理的なIFで操作するためのM5StickCを使った操作デバイス、ｂDriverM5の設計製作データ。

# bDriverM5 Files（ファイルの説明）
*  bDriveM5 : ArduinoIDE ino files for M5StickC（M5StickCのソースコード）  
*  bDriverM5_Sch.pdf : Schematic PDF file（回路図）  
*  Grip : Grip mechanical designe files（グリップメカ）  
  
Notic!!  
arduino-esp32/libraries/BLE has some bugs and you need to use wakwak-koba's BLE lib to works this M5Stick application. (Nov. 2019)  
本家のBLEライブラリはバグがあってこのアプリを動かすにはwakwak-kobaさんのBugFix版のBLEライブラリが必要です。（2019年11月時点）  
https://github.com/wakwak-koba/arduino-esp32/tree/master/libraries/BLE  

# その他
* 使用しているアナログジョイスティックは任天堂ゲームキューブのもの（押し込みスイッチがない）
* 使用しているスライダーVRは秋月電子で売っている10kΩBカーブのもの
