# wavetools
Utility programs for a long sound file

## コマンド

### rawcut_autozcr

振幅とゼロクロス回数で音声区間を検出し時間を出力する．
出力された時間を保存したファイルは後述のrawcut_labelで利用できる．

### rawcut_label

ラベルファイルに基づいて音声ファイルを分割する．

### Example

``` sh
./rawcut_autozcr xxxx.le > xxxx.lab
./rawcut_label xxxx.le xxxx.lab xxxx_cut
```
