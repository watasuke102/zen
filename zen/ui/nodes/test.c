#include "zen/ui/nodes/test.h"

#include <cairo.h>
#include <zen-common.h>
#include <zigzag.h>

#include "zen/screen/output.h"
#include "zen/server.h"

static void
zn_test_on_click(struct zigzag_node *node, double x, double y)
{
  UNUSED(x);
  UNUSED(y);
  struct zn_test *self = node->user_data;
  if (self->c != 2) {
    return;
  }
  self->t = (self->t + 1) % 4;
  zn_debug(">> reconfigure! %d", self->t);
  zigzag_node_reconfigure(node, ZIGZAG_RECONFIGURE_HORIZONTAL, self->t);
  zigzag_node_reconfigure(node, ZIGZAG_RECONFIGURE_VERTICAL, self->t);
}

static bool
zn_test_render(struct zigzag_node *node, cairo_t *cr)
{
  struct zn_test *self = node->user_data;
  if (self->c == 2) {
    zigzag_cairo_draw_node_frame(cr, node,
        (struct zigzag_color){0.3, 0.3, 0.3, 0.8},
        (struct zigzag_color){0.3, 0.3, 0.8, 0.0}, 20, 0);
    return true;
  }

  struct zigzag_color c[2] = {
      {1, 0, 0, 1},
      {0, 1, 0, 1},
  };
  zigzag_cairo_draw_node_frame(
      cr, node, c[self->c % 2], c[(self->c + 1) % 2], 16, 0);
  return true;
}

static void
zn_test_init_frame(
    struct zigzag_node *node, double screen_width, double screen_height, int w)
{
  UNUSED(screen_height);
  node->pending.frame.x = 50.;
  node->pending.frame.y = 50.;
  if (w == 0) {
    /// parent
    // node->padding.top = 20.;
    // node->padding.bottom = 20.;
    // node->padding.left = 20.;
    // node->padding.right = 20.;
    node->pending.frame.width = screen_width * 0.8;
  } else {
    /// child
    // node->margin.top = 10.;
    // node->margin.bottom = 10.;
    // node->margin.left = 10.;
    // node->margin.right = 10.;
    node->pending.frame.width = w;
  }
  node->pending.frame.height = 50. + (w == 0) * 400;
}

static const struct zigzag_node_impl implementation = {
    .on_click = zn_test_on_click,
    .render = zn_test_render,
};

struct zn_test *
zn_test_create(struct zigzag_layout *zigzag_layout,
    struct wlr_renderer *renderer, int c, int w)
{
  UNUSED(renderer);
  struct zn_test *self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  struct zigzag_node *zigzag_node =
      zigzag_node_create(&implementation, zigzag_layout, true, self);

  if (zigzag_node == NULL) {
    zn_error("Failed to create a zigzag_node");
    goto err_test;
  }
  self->zigzag_node = zigzag_node;

  self->c = c;

  zn_test_init_frame(self->zigzag_node, self->zigzag_node->layout->screen_width,
      self->zigzag_node->layout->screen_height, w);

  return self;

err_test:
  free(self);

err:
  return NULL;
}

void
zn_test_destroy(struct zn_test *self)
{
  zigzag_node_destroy(self->zigzag_node);
  free(self);
}
