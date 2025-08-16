#define XPLM200 1
#include "XPLMProcessing.h"
#include "XPLMUtilities.h"
#include "XPLMPlugin.h"
#include "XPLMDataAccess.h"
#include "XPLMNavigation.h"
// #include "XPLM/XPLMCommands.h"

#include <fcntl.h>
#include <unistd.h>
#include <linux/joystick.h>
#include <cstring>
#include <chrono>

static int js_fd = -1;
static bool button_state[32] = { false };
static bool last_button_state[32] = { false };

static XPLMCommandRef cmd_flaps_down = NULL;
static XPLMCommandRef cmd_flaps_up = NULL;
// Add other command refs as needed

// Debounce timing
static std::chrono::steady_clock::time_point last_press_time[32];
static const int debounce_ms = 50;

bool is_debounce_passed(int btn) {
	using namespace std::chrono;
	auto now = steady_clock::now();
	auto diff = duration_cast<milliseconds>(now - last_press_time[btn]).count();
	if (diff > debounce_ms) {
		last_press_time[btn] = now;
		return true;
	}
	return false;
}

void ProcessJoystickEvents() {
	struct js_event e;
	while (read(js_fd, &e, sizeof(e)) > 0) {
		if (e.type == JS_EVENT_BUTTON) {
			int btn = e.number;
			bool pressed = e.value != 0;

			if (btn < 32) {
				if (pressed != button_state[btn]) {
					// State changed: debounce
					if (is_debounce_passed(btn)) {
						button_state[btn] = pressed;
						if (btn == 22 && cmd_flaps_down) {
							if (pressed)
								XPLMCommandBegin(cmd_flaps_down);
							else
								XPLMCommandEnd(cmd_flaps_down);
						}
						// else if for other buttons...
						if (btn == 21 && cmd_flaps_up) {
							if (pressed)
								XPLMCommandBegin(cmd_flaps_up);
							else
								XPLMCommandEnd(cmd_flaps_up);
						}
					}
				}
			}
		}
	}
}

float JoystickPollCallback(float inElapsedSinceLastCall, float inElapsedTimeSinceLastFlightLoop, int inCounter, void *inRefcon) {
	ProcessJoystickEvents();
	return -1; // Run every frame
}

PLUGIN_API int XPluginStart(char *outName, char *outSig, char *outDesc) {
	strcpy(outName, "DebouncePlugin");
	strcpy(outSig, "yourdomain.debounce");
	strcpy(outDesc, "Debounces joystick button inputs and triggers commands manually.");

	cmd_flaps_down = XPLMFindCommand("sim/flight_controls/flaps_down");
	cmd_flaps_up = XPLMFindCommand("sim/flight_controls/flaps_up");
	// load other commands similarly

	js_fd = open("/dev/input/js3", O_RDONLY | O_NONBLOCK);
	if (js_fd < 0) {
		XPLMDebugString("Failed to open joystick device\n");
		return 0;
	}

	XPLMDebugString("debounce plugin opened joystick device\n");
	XPLMRegisterFlightLoopCallback(JoystickPollCallback, 0.01, NULL);
	return 1;
}

PLUGIN_API void XPluginStop() {
	if (js_fd >= 0) close(js_fd);
	XPLMUnregisterFlightLoopCallback(JoystickPollCallback, NULL);
}

PLUGIN_API void XPluginDisable() {}
PLUGIN_API int XPluginEnable() { return 1; }
PLUGIN_API void XPluginReceiveMessage(XPLMPluginID inFromWho, int inMessage, void *inParam) {}
