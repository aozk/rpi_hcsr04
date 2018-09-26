HC-SR04距離測定モジュール
---
Raspberry Piに繋いだHC-SR04を用いて測定対象物までの距離を測定する。

## 概要
Raspberry PiのGPIOに繋いだHC-SR04をコントロールするPython3モジュールです。HC-SR04から測定対象物までの距離を測定します。GPIOコントロールは`/dev/gpiomem`を直接利用しています。gpioグループのメンバーで動作させることができるでしょう。

Cの標準ライブラリのみ用いているので、たぶんソフトウェア上の依存関係は多くありません。恐らくコンパイルだけなら多くの環境で行えると思います。ハードウェア依存関係はそもそも当モジュールがHC-SR04をコントロールすることを目的にしているので完全にHC-SR04に依存します。`/dev/gpiomem`を用いている関係上、Raspberry Piにも依存していると思います（他のプラットフォームで確認していないので確信はない）。

## ビルド方法

I have build and tested this code on Raspberry Pi2 B+.

### Required
- Python3
- raspbian build-essential

### Build and Install
```sh
$ python3 setup.py install
```

### Image of use
```python
>>> import rpi_hcsr04 as gpio
>>> gpio.init()
>>> gpio.measure()
218.28000000000003
>>> gpio.close()
```

## 推奨環境・依存関係

### ハードウェア環境
Raspberry Piでしかモジュールの動作確認をしてません。

- Raspberry Pi 
    - 恐らく全バージョンで動作するはず
    - 手元で試したのはRaspberry Pi2 B+
- HC-SR04
    - 距離測定モジュール。当モジュールはHC-SR04の仕様を前提としています

Raspberry PiとHC-SR04のつなぎ方は次の図が参考になるはずです。

```
Raspberry Pi                   HC-SR04
GPIO Pin                          Pins
------------                   -------

PIN:2     (5v)--------------------VCC

PIN:11(gpio17)--------------------Trig
                     .____.
PIN:13(gpio27)----o--|    |-------Echo
                  |  '----'R1
                 .-. R2    5=0
                 | | 1K
                 | |
                 '-'
                  |
PIN:20   (GND)----o---------------GND
```

### ソフトウェア環境

## ライセンス
The MIT License
