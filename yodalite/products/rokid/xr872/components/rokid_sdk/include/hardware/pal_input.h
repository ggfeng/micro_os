/*
 * Copyright (c) 1999-2002 Vojtech Pavlik
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 */
#ifndef _INPUT_H_
#define _INPUT_H_

#include <hardware/input-event-codes.h>

/**
 * struct input_value - input value representation
 * @type: type of value (EV_KEY, EV_ABS, etc)
 * @code: the value code
 * @value: the value
 */
struct input_event {
    unsigned short type;
    unsigned short code;
    int value;
    const char* input_source;
};

typedef struct {
   struct timespec ts;
   struct input_event event;
} input_event_t;

#define INPUT_LOG printf

extern int pal_input_init(void);
extern int pal_input_get_event(input_event_t *event);
extern int pal_input_put_event(struct input_event *event);

#endif  /*_INPUT_H_*/
