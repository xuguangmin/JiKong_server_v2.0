/*
 * ghttp_wrapper.h
 *
 *  Created on: 2013��11��21��
 *      Author: flx
 */

#ifndef __GHTTP_WRAPPER_H__
#define __GHTTP_WRAPPER_H__

extern int ghttp_wrapper_send_url(char *url);
extern int ghttp_wrapper_send_onvif(char *uri, char *body);
extern void ghttp_wrapper_clear();

#endif /* __GHTTP_WRAPPER_H__ */
