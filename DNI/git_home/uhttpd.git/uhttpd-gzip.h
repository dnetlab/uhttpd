
#ifndef __UHTTPD_GZIP_H_
#define __UHTTPD_GZIP_H_

int uh_gzip_init(struct client * cl);
int uh_gzip_http_send(struct client * cl, struct http_request * req, const char * buf, int len, int more);

#endif
