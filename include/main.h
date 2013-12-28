#ifndef __MAIN_H__
#define __MAIN_H__

#define TRUE  1
#define FALSE 0

#define VERSION "   RasPI MPD v0.1   "

int b_end;

typedef struct queue_item queue_t;

struct queue_item
{
    queue_t *pr_next;
    int      i_keycode;
};

#endif
