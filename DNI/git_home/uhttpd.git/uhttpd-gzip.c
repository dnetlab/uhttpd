
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>

#include "uhttpd.h"
#include "uhttpd-gzip.h"

int uh_gzip_init(struct client *cl) 
{
    cl->zstream.zalloc = Z_NULL;
    cl->zstream.zfree = Z_NULL;
    cl->zstream.opaque = Z_NULL;

    return deflateInit2(&cl->zstream, Z_DEFAULT_COMPRESSION,
             Z_DEFLATED, 16 | MAX_WBITS,
             8, Z_DEFAULT_STRATEGY);
}

int uh_gzip_http_send(struct client *cl, struct http_request * req, const char * buf, int len, int more)
{
    int ret = 0;
    int gzlen;
    char *gzbuf;
    int flush = 0;
    z_stream *zs = &cl->zstream;

    if(!more && len == 0 && buf[0] == '\0')
    {
        flush = 1;    
    }
    else
    {
        flush = 0;
    }

    zs->avail_in = len;
    zs->next_in = (z_const Bytef *)buf;

    gzlen = len > 0 ? len : 1024;
    gzbuf = calloc(gzlen, sizeof(char));
    if (!gzbuf)
        return -1;
   
more:
    zs->avail_out = gzlen;
    zs->next_out = (Bytef *)gzbuf;

    ret = deflate(zs, flush ? Z_FINISH : Z_SYNC_FLUSH);

    uh_http_send(cl, req, gzbuf, gzlen - zs->avail_out);

    if (flush && ret == Z_OK)
        goto more;
    else if (ret == Z_STREAM_END)
        ret = deflateEnd(zs);

    free(gzbuf);
    
    return len;
}

