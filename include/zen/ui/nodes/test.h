#pragma once

#include <cairo.h>
#include <wlr/render/wlr_renderer.h>
#include <zigzag.h>

struct zn_test {
  struct zigzag_node *zigzag_node;
  int c;
  int t;
};

struct zn_test *zn_test_create(struct zigzag_layout *zigzag_layout,
    struct wlr_renderer *renderer, int c, int w);

void zn_test_destroy(struct zn_test *self);
