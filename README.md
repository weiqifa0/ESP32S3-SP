# ESP32S3-SP

- 开发板图

![image](https://user-images.githubusercontent.com/11375905/231498216-7d6f6118-a4fd-4940-8c35-5172c50f5473.png)

- 环境安装参考这个链接

https://mp.weixin.qq.com/s/TbulWs86TWMFHB3vj_xS_A

https://mp.weixin.qq.com/s/89i9LeKZv3B2qcryyxUWUQ

## clone仓库部分更新
git clone -b v5.0.1 --recursive https://github.com/espressif/esp-idf.git

要换成
git clone -b v5.0.1 --recursive git@github.com:espressif/esp-idf.git

# 分支介绍
* master分支的例程需要esp-idf 4.4的环境
* 下载后直接运行idf.py build就可以，不需要在set-target 这样会导致会覆盖默认配置

# ESP32S3-uac 音频

## 我调试uac音频的工程
https://github.com/weiqifa0/esp-box

编译usb_headset 这个例程，就可以通过电脑的uac录取到麦克风的音频
<img width="1097" alt="image" src="https://user-images.githubusercontent.com/11375905/231496712-e31f19ff-bab7-47b9-aac3-4ff6ad302cc4.png">


## 设备做HOST的部分

https://docs.espressif.com/projects/espressif-esp-iot-solution/zh_CN/latest/usb/usb_stream.html

## 设备做device heasdset

* 下面的工程需要esp-idf 5.0.1的环境

https://github.com/espressif/esp-box/tree/master/examples/usb_headset


# 常见的指令
* git describe --tags 查看esp-idf版本
* . $HOME/esp/esp-idf/export.sh  //设置环境变量
* idf.py set-target esp32s3 //设置编译芯片
* idf.py menuconfig //配置

* idf.py build //编译

* idf.py -p PORT [-b BAUD] flash //多分区完整烧录

* idf.py app //编译应用app
* idf.py app-flash //烧录应用app

* cp -r $IDF_PATH/examples/get-started/hello_world . //拷贝参考实例到当前目录
* git submodule update --init --recursiv //更新模块仓库


# Git相关
- **递归更新子仓库**
- git submodule update --init --recursive
* **切换esp-idf版本**
- git checkout v3.2
- git submodule update --init --recursive

# 常见错误
python依赖包没有

```c

To install the missing packages, please run "install.sh"
Diagnostic information:
    IDF_PYTHON_ENV_PATH: /Users/crisqifawei/.espressif/python_env/idf4.4_py3.9_env
    Python interpreter used: /Users/crisqifawei/.espressif/python_env/idf4.4_py3.9_env/bin/python
Constraint file: /Users/crisqifawei/.espressif/espidf.constraints.v5.1.txt
Requirement files:
 - /Users/crisqifawei/esp/esp-idf/tools/requirements/requirements.core.txt
Python being checked: /Users/crisqifawei/.espressif/python_env/idf4.4_py3.9_env/bin/python

```



-  ~/esp/esp-idf/install.sh /Users/crisqifawei/esp/esp-idf/tools/requirements/requirements.core.txt

# 使用idf.py flash monitor 真的贼爽
想退出的话按住‘Ctrl + ]’
<img width="1183" alt="image" src="https://user-images.githubusercontent.com/11375905/235952280-d0708b88-644e-4847-a1a3-b8efd6cc24bd.png">

