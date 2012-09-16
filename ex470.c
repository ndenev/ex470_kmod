/*-
 * Copyright (c) 2009 Nikolay Denev <ndenev@gmail.com> 
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer
 *    in this position and unchanged.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <sys/types.h>
#include <sys/module.h>
#include <sys/systm.h>
#include <sys/errno.h>
#include <sys/param.h>
#include <sys/kernel.h>
#include <dev/led/led.h>

#define ADDR	0x1064 /* io address */

#define	OFF	0
#define	BLUE	1
#define	PURPLE	2
#define	RED	3
#define	NUMLED	4
#define NUMCOL	4

static u_int16_t encreg, setreg;
static struct cdev *blue_led[NUMLEDS], *red_led[NUMLEDS], *purple_led[NUMLEDS];

static
u_int16_t led[NUMLEDS][NUMCOLORS] = {
	{0x0000, 0x0001, 0x1001, 0x1000},
	{0x0000, 0x0002, 0x0102, 0x0100},
	{0x0000, 0x0008, 0x0208, 0x0200},
	{0x0000, 0x0020, 0x0420, 0x0400}
};


static void
ex470_led(void *ptr, int state)
{
	setreg = *(u_int16_t *)ptr;	
	encreg = inw(ADDR);
	if (state == 0) {
		if (!(encreg & setreg))
			encreg = encreg ^ setreg;
	} else {
		if (encreg & setreg)
			encreg = encreg ^ setreg;
	}
	outw(ADDR, encreg);
}

static void
attach_leds(void)
{
	int i;
	char led_name[10];

	for (i=0; i<NUMLEDS; i++) {
		snprintf(led_name, sizeof(led_name), "hdd%d_act", i);	
		blue_led[i] = led_create(ex470_led, &led[i][BLUE], led_name);
		snprintf(led_name, sizeof(led_name), "hdd%d_err", i);
		red_led[i] = led_create(ex470_led, &led[i][RED], led_name);
		snprintf(led_name, sizeof(led_name), "hdd%d_inf", i);
		purple_led[i] = led_create(ex470_led, &led[i][PURPLE], led_name);
	}
}

static void
detach_leds(void)
{
	int i;

	for (i=0; i<NUMLEDS; i++) {
		led_destroy(blue_led[i]);
		led_destroy(red_led[i]);
		led_destroy(purple_led[i]);
	}
}

static void
flash_switch(int state)
{
	setreg = 0x0080;	
	encreg = inw(ADDR);
	if (state == 0) {
		if (encreg & setreg)
			encreg = encreg ^ setreg;
	} else {
		if (!(encreg & setreg))
			encreg = encreg ^ setreg;
	}
	outw(ADDR, encreg);
}

static int
ex470_loader(struct module *m, int what, void *arg)
{
	int err = 0;

	switch (what) {
		case MOD_LOAD:
		  printf("HP MediaSmart Server EX470 Management Module Loaded.\n");
		  flash_switch(0);
		  attach_leds();
		  break;
		case MOD_UNLOAD:
		  printf("HP MediaSmart Server EX470 Management Module Unloaded.\n");
		  flash_switch(1);
		  detach_leds();
		  break;
		default:
		  err = EOPNOTSUPP;
		  break;
	}
	return(err);
}

static moduledata_t ex470_mod = {
	"ex470",
	ex470_loader,
	NULL
};  

DECLARE_MODULE(ex470, ex470_mod, SI_SUB_DRIVERS, SI_ORDER_MIDDLE);
