/*
 * protocol_http.c
 *
 *  Created on: 2012-12-24
 *      Author: flx
 */

#include <stdio.h>
#include <string.h>
#include "protocol_http.h"

#define STRING_HTTP_URL_HEADER    "http://"
#define STRING_HTTP_URL    "http://192.168.1.16/cgi-bin/aw_ptz?cmd=%23R00&res=1"
int protocol_http_parse_url(const char *url, struct url_element *url_em)
{
	const char *p, *req;
	int header_len = strlen(STRING_HTTP_URL_HEADER);
	if(!url_em)
		return 0;
	if(strncasecmp(url, STRING_HTTP_URL_HEADER, header_len) != 0)
		return 0;

	p = url;
	p += header_len;

	memset(url_em->ip, 0, 256);
	url_em->req = (void *)0;
	if((req = strchr(p, '/')))
	{
		strncpy(url_em->ip, p, req-p);
		url_em->req = req;
	}
	url_em->port = 80;

	return 1;
}

int protocol_http_get(char *proto_get, const struct url_element *url_em)
{
	if(!proto_get)
		return 0;

	sprintf(proto_get, "GET %s HTTP/1.0\n\n", url_em->req);
	//sprintf(proto_get, "GET %s HTTP/1.0\nHost: %s\nAccept: */*\n\n", url_em->req, url_em->ip);
	return 1;
}

