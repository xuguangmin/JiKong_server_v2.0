/******************************************************************************

                  版权所有 (C), 2001-2020, 北京飞利信科技股份有限公司

 ******************************************************************************
  文件名称 : curl_wrapper.h
  作者    : 贾延刚
  生成日期 : 2012-11-01

  版本    : 1.0
  功能描述 : 对curl库的一个封装。目前只用到了这个库实现的http协议，用来向松下
            摄像机发送指令
            在外部编译该库后，把静态库（.a）文件复制到lib文件夹，然后把安装后的头
            文件复制到服务器源代码的src/include目录。

  修改历史 :

******************************************************************************/

#ifndef __CURL_WRAPPER_H__
#define __CURL_WRAPPER_H__

extern int curl_wrapper_init();
extern int curl_wrapper_close();
extern int curl_wrapper_send(const unsigned char *url_buffer, int data_len);
extern int ghttp_wrapper_onvif_send(const char *uri, const char *http_body);

#endif /* CURL_WRAPPER_H_ */
