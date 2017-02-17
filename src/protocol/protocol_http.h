/*
 * protocol_http.h
 *
 *  Created on: 2012-12-24
 *      Author: flx
 */

#ifndef __PROTOCOL_HTTP_H__
#define __PROTOCOL_HTTP_H__

struct url_element
{
	char        ip[256];
	int         port;
	const char *req;
};

extern int protocol_http_parse_url(const char *url, struct url_element *url_em);
extern int protocol_http_get(char *proto_get, const struct url_element *url_em);

#endif /* __PROTOCOL_HTTP_H__ */
