#include "zen/pointer.h"

#include <wlr/types/wlr_input_device.h>
#include <wlr/types/wlr_pointer.h>

#include "zen-common.h"
#include "zen/cursor.h"
#include "zen/server.h"

static void
zn_pointer_handle_motion_relative(struct wl_listener* listener, void* data)
{
  UNUSED(listener);
  struct wlr_event_pointer_motion* event = data;
  struct zn_server* server = zn_server_get_singleton();
  struct zn_cursor* cursor = server->input_manager->seat->cursor;

  zn_cursor_move_relative(cursor, event->delta_x, event->delta_y);
}

struct zn_pointer*
zn_pointer_create(struct wlr_input_device* wlr_input_device)
{
  if (!zn_assert(wlr_input_device->type == WLR_INPUT_DEVICE_POINTER,
          "Wrong type - expect: %d, actual: %d", WLR_INPUT_DEVICE_POINTER,
          wlr_input_device->type)) {
    goto err;
  }

  struct zn_pointer* self;
  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->wlr_pointer = wlr_input_device->pointer;

  self->motion_relative_listener.notify = zn_pointer_handle_motion_relative;
  wl_signal_add(&wlr_input_device->pointer->events.motion,
      &self->motion_relative_listener);

  return self;

err:
  return NULL;
}

void
zn_pointer_destroy(struct zn_pointer* self)
{
  // wlr_pointer is destroyed by wlr_input_device
  wl_list_remove(&self->motion_relative_listener.link);
  free(self);
}
