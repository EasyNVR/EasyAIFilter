# EasyAIFilter #

EasyAIFilter是由[EasyNVR](http://www.easynvr.com "www.easynvr.com")团队开发和运维的一款功能组件，安照既有的API模板，用户可以获取到EasyNVR中原始音视频数据，可以作自定义的处理、分析、修改等操作，并将处理之后的数据回调回去，EasyNVR可以直播和录像的数据就是自定义处理后的数据。

**使用场景**：适合需要提供互联网无插件直播，同时需要进行视频分析处理的场景，如人脸识别、智慧工地、烟雾/火灾报警等

## 调用示例 ##

- 用户不可以修改EasyAIFilter.h中提供的接口函数和回调函数。

- **EasyAIFilter_Init**： EasyNVR上层开始一路视频播放或录制时，会自动调用此接口，创建一路视频的处理对象，并传入音视频的具体参数。

- **EasyAIFilter_Deinit**： EasyNVR上层停止视频播放和录制时，会调用此接口销毁具体的处理对象。

- **EasyAIFilter_SendData**： EasyNVR上层出入实时音视频数据

- **EasyAIFilterCallBack**： 处理后的数据通过这个回调传入EasyNVR上层


	Windows编译方法，

    	Visual Studio 2015 编译：libEasyAIFilter.sln

	Linux编译方法，
		
		cd libEasyAIFilter\libEasyAIFilter
		mkdir build
		cd build
		cmake -DCMAKE_BUILD_TYPE=Release ..
		make

## 示例 ##
OverlayDemo 为一个使用示例，为视频叠加字幕，仅供参考。

## 技术支持 ##

- 邮件：[support@easynvr.com](mailto:support@easynvr.com) 

- QQ交流群：[383501345](https://jq.qq.com/?_wv=1027&k=5RTdCMn "EasyNVR")


## 获取更多信息 ##

**EasyDarwin**开源流媒体服务器：[www.EasyDarwin.org](http://www.easydarwin.org)

**EasyDSS**商用流媒体解决方案：[www.EasyDSS.com](http://www.easydss.com)

**EasyNVR**无插件直播方案：[www.EasyNVR.com](http://www.easynvr.com)

Copyright &copy; EasyDarwin Team 2012-2019

![EasyDarwin](http://www.easydarwin.org/skin/easydarwin/images/wx_qrcode.jpg)
