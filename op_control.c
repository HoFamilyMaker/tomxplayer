//
//  op_control.c
//
//  Author:
//       Jonathan Jason Dennis <theonejohnnyd@gmail.com>
//
//  Copyright (c) 2017 Meticulus Development
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include "op_control.h"
#include "log.h"
#include "main_settings.h"
#include "op_dbus.h"

#define TAG "op_control"

int pos[4];
char *file_name;
int is_running = 0;
pthread_t update_thread;

void opc_stop_omxplayer() {
	op_dbus_send_stop();
	/* TODO: We should only kill our own instance but
	 * we can't handle that yet. Multiple instances
	 * of either tomxplayer or omxplayer causes problems.
	 *	popen?
	 */
	system("killall omxplayer 2>&1 > /dev/null");
	system("killall omxplayer.bin 2>&1 > /dev/null");
}

static void *start_omxplayer_system(void *arg) {
	char cmd[255];
	opc_stop_omxplayer(); 
	is_running = 1;
	sprintf(cmd, "omxplayer --adev %s --aspect-mode %s --no-keys --no-osd --win %d,%d,%d,%d '%s' 2>&1 > /dev/null", 
	audio_out.string_value,
	stretch.int_value ? "stretch": "Letterbox", 
	pos[0],pos[1],pos[2],pos[3], file_name);
	system(cmd);
	is_running = 0;
}

void opc_start_omxplayer_thread(int tpos[], char * tfile_name) {
	pos[0] = tpos[0];
	pos[1] = tpos[1];
	pos[2] = tpos[2];
	pos[3] = tpos[3];
	file_name = strdup(tfile_name);
	pthread_t thread;
	pthread_attr_t attr; 
	pthread_attr_init(&attr);
	pthread_create(&thread,&attr,&start_omxplayer_system,NULL);
}

void opc_update_pos(int tpos[]) {
	if(!is_running)return;
	op_dbus_send_setvideopos(tpos);
}

int opc_status(long long pbpos[]) {
	pbpos[0] = op_dbus_send_duration();
	pbpos[1] = op_dbus_send_position();
	return 0; 
}

void opc_toggle_playpause() {
	if(!is_running) return;
	op_dbus_send_pause();
	op_dbus_send_duration();
}

int opc_set_volume(double vol) {
	if(!is_running) return 1;
	return op_dbus_send_volume(vol);
}

void opc_set_pb_position(unsigned long pos) {
	if(!is_running) return;
	op_dbus_send_setposition((long long)pos);
}

void opc_hidevideo() {
	if(!is_running) return;
	op_dbus_send_hidevideo();
}

void opc_unhidevideo() {
	if(!is_running) return;
	op_dbus_send_unhidevideo();
}

void opc_set_aspect(char * aspect) {
	if(!is_running) return;
	op_dbus_send_setaspectmode(aspect);
}

int opc_is_running() {
	return is_running;
}
