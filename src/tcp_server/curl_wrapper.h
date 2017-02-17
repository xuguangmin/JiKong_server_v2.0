/******************************************************************************

                  ��Ȩ���� (C), 2001-2020, ���������ſƼ��ɷ����޹�˾

 ******************************************************************************
  �ļ����� : curl_wrapper.h
  ����    : ���Ӹ�
  �������� : 2012-11-01

  �汾    : 1.0
  �������� : ��curl���һ����װ��Ŀǰֻ�õ��������ʵ�ֵ�httpЭ�飬����������
            ���������ָ��
            ���ⲿ����ÿ�󣬰Ѿ�̬�⣨.a���ļ����Ƶ�lib�ļ��У�Ȼ��Ѱ�װ���ͷ
            �ļ����Ƶ�������Դ�����src/includeĿ¼��

  �޸���ʷ :

******************************************************************************/

#ifndef __CURL_WRAPPER_H__
#define __CURL_WRAPPER_H__

extern int curl_wrapper_init();
extern int curl_wrapper_close();
extern int curl_wrapper_send(const unsigned char *url_buffer, int data_len);
extern int ghttp_wrapper_onvif_send(const char *uri, const char *http_body);

#endif /* CURL_WRAPPER_H_ */
