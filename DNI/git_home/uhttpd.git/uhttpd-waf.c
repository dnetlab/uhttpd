
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "uhttpd-waf.h"

waf_item_list_t g_waf_list;

static struct list_head *list_add_first(struct list_head *new, struct list_head *head)
{
    if(new == NULL)
    {
        return NULL;
    }
    else
    {
        list_add(new, head);
        return new;
    }
}

static struct list_head *list_del_last(struct list_head *head)
{
    struct list_head *tmp = NULL;

    if(head->prev == head)
    {
        return NULL;
    }
    else
    {
        tmp = head->prev;
        list_del(head->prev);
        return tmp;
    }
}

long uptime_now()
{
    struct timespec time;
    clock_gettime(CLOCK_MONOTONIC, &time);
    return (time.tv_sec);
}

void waf_list_init()
{
    g_waf_list.count = 0;
    INIT_LIST_HEAD(&g_waf_list.head);
}

void waf_list_add(waf_item_t *waf)
{
    g_waf_list.count ++;
    list_add_first(&waf->list, &g_waf_list.head);
}

void waf_list_destroy()
{
    waf_item_t *waf, *tmp;

    list_for_each_entry_safe(waf, tmp, &g_waf_list.head, list)
    {
        list_del(&waf->list);
        waf_item_free(waf);
    }

    g_waf_list.count = 0;
}

waf_item_t *waf_item_new()
{
    waf_item_t *waf = NULL;
    waf_item_t *tmp2 = NULL;
    struct list_head *tmp1 = NULL;

    if(g_waf_list.count >= MAX_WAF_SESSION)
    {
        tmp1 = list_del_last(&g_waf_list.head);
        tmp2 = list_entry(tmp1, waf_item_t, list);
        waf_item_free(tmp2);
        g_waf_list.count --;
    }

    waf = (waf_item_t *)malloc(sizeof(waf_item_t));
    if(!waf)
    {
        return NULL;
    }
    
    memset(waf, 0x0, sizeof(waf_item_t));

    return waf;
}

void waf_item_update(waf_item_t *waf)
{
    int cnt = 0;
    long now = 0;
    int intval = 0;

    cnt = waf->failed_count;
    now = uptime_now();
    
    if(waf->last_time != 0)
    {
         intval = now - waf->last_time;
    }
    else
    {
        intval = 0;
    }
    
    waf->failed_intval[cnt % MAX_FAILED_TRY] = intval;
    waf->last_time = now;
    waf->failed_count ++;
}

void waf_item_free(waf_item_t *waf)
{
    free(waf);
}

waf_item_t *waf_item_get(char *ipaddr)
{
    waf_item_t *waf = NULL;

    list_for_each_entry(waf, &g_waf_list.head, list)
    {
        if(strcmp(waf->ipaddr, ipaddr) == 0)
        {
            return waf;
        }
    }

    return NULL;
}

int waf_item_check(waf_item_t *waf)
{
    int i = 0;
    int failed_sum = 0;
    long now;

    now = uptime_now();

    if(waf->failed_lock) 
    {
        if((now - waf->last_time) < MAX_LOCKED_TIME)
        {
            return 1;
        }
        else
        {
            waf->failed_lock = 0;
            waf->failed_count = 0;
        }
    }

    for(i = 0; i < MAX_FAILED_TRY; i ++)
    {
        failed_sum += waf->failed_intval[i];
    }

    if(waf->failed_count >= MAX_FAILED_TRY 
        && failed_sum < MAX_FAILED_TIME)
    {
        waf->failed_lock = 1;
        return 1;
    }

    return 0;
}
