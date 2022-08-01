#ifndef ZEN_INPUT_MANAGER_H
#define ZEN_INPUT_MANAGER_H

#include <wayland-server.h>
#include <wlr/types/wlr_input_device.h>

#include "zen/input-device.h"

struct zn_input_manager {
  struct zn_seat* seat;
};

void zn_input_manager_handle_new_wlr_input(
    struct zn_input_manager* self, struct wlr_input_device* wlr_input);

struct zn_input_manager* zn_input_manager_create(struct wl_display* display);

void zn_input_manager_destroy(struct zn_input_manager* self);

#endif  // ZEN_INPUT_MANAGER_H
