#define LIN true

#include "XPLMPlugin.h"
#include "XPLMProcessing.h"
#include "XPLMUtilities.h"
#include <linux/joystick.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

static int js_fd = -1;
static unsigned char button_state[32] = {0};  // up to 32 buttons

float ReadJoystick(float, float, int, void *) {
	struct js_event e;
	while (read(js_fd, &e, sizeof(e)) > 0) {
		e.type &= ~JS_EVENT_INIT; // strip init flag
		if (e.type == JS_EVENT_BUTTON && e.number < 32) {
			if (button_state[e.number] != e.value) {
				button_state[e.number] = e.value;
				XPLMDebugString("Button change detected: ");
				char msg[64];
				snprintf(msg, sizeof(msg), "Btn %d -> %d\n", e.number, e.value);
				XPLMDebugString(msg);
			}
		}
	}
	return -1.0f; // run every frame
}

PLUGIN_API int XPluginStart(char *name, char *sig, char *desc) {
	strcpy(name, "DebounceTester");
	strcpy(sig, "nigel.debounce");
	strcpy(desc, "Logs joystick button changes directly from /dev/input/js1");

	js_fd = open("/dev/input/js1", O_RDONLY | O_NONBLOCK);
	if (js_fd < 0) {
		XPLMDebugString("Failed to open /dev/input/js1\n");
	} else {
		XPLMDebugString("Opened /dev/input/js1 successfully\n");
	}

	XPLMRegisterFlightLoopCallback(ReadJoystick, -1.0f, NULL);
	return 1;
}

PLUGIN_API void XPluginStop(void) {
	if (js_fd >= 0) close(js_fd);
}

PLUGIN_API void XPluginDisable(void) {}
PLUGIN_API int XPluginEnable(void) { return 1; }
PLUGIN_API void XPluginReceiveMessage(XPLMPluginID, int, void *) {}
