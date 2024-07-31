# for-optim

## 概要
このリポジトリには、2022年度のTeam Bプロジェクト、2023年度のTeam Aプロジェクト、およびawsを使った遠隔潅水に関連するソースコードが含まれています。
### プロジェクト構成
<pre>
<details>
<summary>.git</summary>
.git
├── hooks
│ ├── applypatch-msg.sample
│ ├── commit-msg.sample
│ ├── fsmonitor-watchman.sample
│ ├── post-update.sample
│ ├── pre-applypatch.sample
│ ├── pre-commit.sample
│ ├── pre-merge-commit.sample
│ ├── pre-push.sample
│ ├── pre-rebase.sample
│ ├── pre-receive.sample
│ ├── prepare-commit-msg.sample
│ ├── push-to-checkout.sample
│ ├── sendemail-validate.sample
│ └── update.sample
├── info
│ └── exclude
├── logs
│ ├── refs
│ │ ├── heads
│ │ │ └── main
│ │ └── remotes
│ │ └── origin
│ │ └── main
│ └── HEAD
├── objects
│ ├── 00
│ │ └── 635b0accdca7d8339c8a06fd2339582ddf1786
│ ├── 0b
│ │ └── b4a2e8a4b7f07bf6139e5402fbf6f309ebf417
│ ├── 0c
│ │ └── 02b3c4cf181b39170217af5e0a881feb85026e
│ ├── 0e
│ │ └── 112643e6ab621e9a805a15bdaeff775b916879
│ ├── 0f
│ │ └── c5e15c65a4083750e2dbc928f90f4da16e04eb
│ ├── 14
│ │ └── c8b552de7874ff80b74817f21eb9f15651601c
│ ├── 17
│ │ └── 2603f4c0f255de636579e6f0951a97eb112b0e
│ ├── 1c
│ │ └── 7a4caaee8e5d9ef24f440cb2dfe1b62e09e7c3
│ ├── 1f
│ │ ├── 617a6706a41b4e98c8147d3a42191d4b1e2a94
│ │ └── add2e20f497c4596eb02d28924379aedcd4688
│ ├── 28
│ │ └── 46561f499e2726e1b087432abc5bf261b154bb
│ ├── 2f
│ │ └── 5c0a9e969fc11ac5aa7b726d215e1683675362
│ ├── 34
│ │ └── 0b7bc61459ac0a014c84a72a498650dc9cc855
│ ├── 40
│ │ └── bc4729b1da4dc1f1302c982bea02f54f755c33
│ ├── 41
│ │ ├── a66578f8f49b3019ac267c5ed199cc89b3720d
│ │ └── ee4d3dc7f219e2204115645b8e08d2a9d3c470
│ ├── 48
│ │ ├── 4c4dc4970986af3251322fda2c81afb60b69ca
│ │ └── 95921a9a8f6d2eea4e33e5424517b767135f13
│ ├── 49
│ │ ├── a1e787156c863633e1e4b6c4344feab1164c42
│ │ └── d408c3560473268a6c071bcee55ad561a843c9
│ ├── 4c
│ │ └── 481e1cc3eb23c8b4ee5704ba1009dc525018d5
│ ├── 55
│ │ └── 18813a1b3d375c96488986e7137bda2c82a6f0
│ ├── 59
│ │ └── 204288e4f417c5c70501010ffcb94014d83c0e
│ ├── 66
│ │ └── 49b528393fd78fe1f37f06d993a21b2b22a787
│ ├── 6f
│ │ └── 38240e18bc7ad2f459dabaeb15234115efe1e2
│ ├── 79
│ │ └── 7ba2068b7bef60adbcbd5fbd27f33f343e3954
│ ├── 80
│ │ └── 48658693fc6cb38e68347b2b78ceaaef13bcb2
│ ├── 82
│ │ └── 8a46e20cf7e68a711a984453714f88b9c35c2f
│ ├── 86
│ │ └── 74c609894f3107318d8d75b61359401357f90a
│ ├── 89
│ │ └── 12d7cd63b7e27b4ee28e03475ce07cace0552b
│ ├── 8b
│ │ └── 097087671137ac87da8356e20c6e61a29bc6aa
│ ├── 8c
│ │ └── 9c4cb55ca4ac1c6540ecb33a68882bba797e84
│ ├── 8f
│ │ └── 04f8fc009855336dd961b8951771a2a0c643e2
│ ├── 92
│ │ └── 7e0371eea33fb961ca5b7f1df5386a3e107197
│ ├── 93
│ │ └── 3d0b93a7736e1cc3f2beab30656c909543c765
│ ├── 97
│ │ └── 8d3eb1798463ada4440a1808208b95eafe5e39
│ ├── 9b
│ │ └── a28064e80cd98562b1949c5489bf1b9deda73e
│ ├── a1
│ │ ├── 3cb6cd0a9bad94721cd92715c2b39d9775f3ec
│ │ └── 944f2ab152d3d0c5f206050fa81f8e955ca5c6
│ ├── a7
│ │ └── 3de7fa9738a591880ae1391b514f4fe1d44718
│ ├── a9
│ │ └── 25b7170f1a27927be37d53d531d3961eda25b2
│ ├── b3
│ │ └── e8739502644687a74900f443ac05fbc6576c5b
│ ├── b4
│ │ └── e82e53def6d981793796bf5e252cd5772504d4
│ ├── b6
│ │ └── 354e503bb8f8b31a8b43017b0478b176821cc8
│ ├── bb
│ │ ├── 879da5a0774e5e3a4e5e90dba7a0770370c0f7
│ │ └── bdd681359cb711fcbca2bf825b14cb6e9ddd4d
│ ├── bd
│ │ └── 609ebf2b8ac8723e82883dfe0e799db05af89a
│ ├── c0
│ │ └── 3ca145378adbda720291ac7758d220bb34579a
│ ├── c2
│ │ └── 682864bfb457ded9176755624a2bf1d2010372
│ ├── c4
│ │ └── 85250091e8d69c6f4295001053719242e2c7a2
│ ├── c8
│ │ └── 57e8f4a72a297ad3f496c9d52771eb67a84740
│ ├── cf
│ │ └── 1cfb769dff36b8e0ef14ba7120169bdec16267
│ ├── d0
│ │ └── a01744f6c5383e02abd5dee1b97c2fa3d8afed
│ ├── d1
│ │ └── 47be8e6a134a2e2c6b55c260175f9e15d039fb
│ ├── d6
│ │ └── b2f993a7e40bec6c8d5940b2c88b196e13331b
│ ├── dc
│ │ └── a120edd2924de1534d4601329388c9a32bf9db
│ ├── e9
│ │ └── 5a917a249095ef74da0252f36ab2e2a5f195c5
│ ├── eb
│ │ └── cbea06eea32bdfde88d692b59c6ef6ba70ceb1
│ ├── f1
│ │ └── 8a699507bfe237ff5089e3afa825eeed3f2c9c
│ ├── f2
│ │ └── 4e3fe5c28efeb1fb8bc30969b590f0f33ba6f3
│ ├── fb
│ │ └── 52780892bb3c267e4c253dde06ed7c2320230f
│ ├── fc
│ │ ├── 013f813dc21fc06a4e703e7981a246d6901cac
│ │ └── 6ee44c694b979ca2fba3353f4bb9b899c0fa97
│ ├── ff
│ │ ├── 0958213713e51e8d63fa048d5a00afe9c6c544
│ │ └── c64ce29496e07baf613d134b4ba7f4b8f8a8c1
│ ├── info
│ └── pack
├── refs
│ ├── heads
│ │ └── main
│ ├── remotes
│ │ └── origin
│ │ └── main
│ └── tags
├── COMMIT_EDITMSG
├── config
├── description
├── HEAD
└── index
</details>
<details>
<summary>.vscode</summary>
.vscode
├── c_cpp_properties.json
├── launch.json
└── settings.json
</details>
<details>
<summary>2022_TeamB</summary>
2022_TeamB
├── .vscode
│ ├── c_cpp_properties.json
│ ├── launch.json
│ └── settings.json
├── ArduinoUNO
│ └── DS4.ino
└── STM32f446re
├── Library
│ ├── OmniMove
│ │ ├── OmniMove.cpp
│ │ └── OmniMove.h
│ └── SerialCtrl
│ ├── SerialCtrl.cpp
│ └── SerialCtrl.h
└── main.cpp
</details>
<details>
<summary>2023_TeamA</summary>
2023_TeamA
├── ArduinoUNO
│ └── DS4.ino
└── STM32f446re
├── Library
│ ├── OmniMove
│ │ ├── OmniMove.cpp
│ │ └── OmniMove.h
│ ├── SerialCtrl
│ │ ├── SerialCtrl.cpp
│ │ └── SerialCtrl.h
│ └── SoftI2C
│ ├── SoftI2C.cpp
│ └── SoftI2C.h
└── main.cpp
</details>
<details>
<summary>aws_20240701</summary>
aws_20240701
├── css
│ └── custom_style.css
├── img
│ ├── camera.png
│ ├── connection.png
│ ├── faq.png
│ ├── manual_auto.png
│ ├── manual.png
│ ├── punch_clock.png
│ ├── sample.jpg
│ ├── sensors.png
│ ├── timer.png
│ ├── toggle.png
│ ├── valve_active.png
│ ├── valve_inactive.png
│ ├── wifi_connected.png
│ └── wifi_disconnected.png
├── php
│ ├── control.php
│ ├── get_schedule.php
│ ├── login.php
│ ├── set_hash.php
│ └── set_schedule.php
├── ras
│ ├── display.php
│ └── gpio_control.php
└── index.html
</details>
</pre>
