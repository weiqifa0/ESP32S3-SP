# ESP32S3-SP

环境安装参考这个链接
https://mp.weixin.qq.com/s/TbulWs86TWMFHB3vj_xS_A

# ESP32S3-uac 音频

https://docs.espressif.com/projects/espressif-esp-iot-solution/zh_CN/latest/usb/usb_stream.html


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
