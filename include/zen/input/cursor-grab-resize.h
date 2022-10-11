#pragma once

#include "zen/cursor.h"
#include "zen/input/cursor-grab.h"
#include "zen/scene/view.h"

struct zn_cursor_grab_resize {
  struct zn_cursor_grab base;
  struct zn_view* view;

  double init_width, init_height;
  double init_cursor_x, init_cursor_y;

  struct wl_listener view_unmap_listener;
};

void zn_cursor_grab_resize_start(
    struct zn_cursor* cursor, struct zn_view* view, uint32_t edges);
