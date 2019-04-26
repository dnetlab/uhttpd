
#ifndef __UHTTPD_WAF_H_
#define __UHTTPD_WAF_H_

#include <stdint.h>
#include "list.h"

/*
 * MAX_FAILED_TIME 时间内，
 * 连续尝试 MAX_FAILED_TRY 次之后，
 * 将用户锁定 MAX_LOCKED_TIME 秒
 */
#define MAX_FAILED_TRY (5 - 1) /* basic认证决定了尝试次数比实际大1, 这里实际是5次 */
#define MAX_FAILED_TIME (30)
#define MAX_LOCKED_TIME (5 * 60)

#define MAX_WAF_SESSION 128

typedef struct {
    char ipaddr[46];
    int last_time;
    int failed_count;
    int failed_lock;
    int failed_intval[MAX_FAILED_TRY];
    struct list_head list;
} waf_item_t;

typedef struct {
    int count;
    struct list_head head;
} waf_item_list_t;

void waf_list_init();
void waf_list_add(waf_item_t * waf);
void waf_list_destroy();

waf_item_t *waf_item_new();
void waf_item_free(waf_item_t * waf);
void waf_item_update(waf_item_t * waf);

waf_item_t *waf_item_get(char *ipaddr);
int waf_item_check(waf_item_t * waf);

#endif
