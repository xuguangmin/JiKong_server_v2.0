/*
 * connect_http.h
 *
 *  Created on: 2013Äê11ÔÂ21ÈÕ
 *      Author: flx
 */

#ifndef __CONNECT_HTTP_H__
#define __CONNECT_HTTP_H__

extern int connect_http_init();
extern int connect_http_close();
extern int connect_http_send_url(const char *url);
extern int connect_http_send_onvif(const char *uri, const char *http_body);

#endif /* __CONNECT_HTTP_H__ */
