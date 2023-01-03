#include "zen/ui/nodes/vr-modal/vr-modal.h"

#include <cairo.h>
#include <zen-common.h>
#include <zigzag.h>

#include "zen/screen/output.h"
#include "zen/server.h"

#define BG_ALPHA 0.7
#define ANIMATION_DURATION_MS 1800
#define ANIMATION_UPDATE_MS 20

#define ICON_WIDTH 137
#define ICON_HEIGHT 78

#define HEADSET_DIALOG_WIDTH 400

static int
zn_vr_modal_handle_timer(void *data)
{
  struct zn_vr_modal *self = data;
  struct zn_server *server = zn_server_get_singleton();

  self->alpha += BG_ALPHA / (ANIMATION_DURATION_MS / ANIMATION_UPDATE_MS);
  zigzag_node_update_texture(self->zigzag_node, server->renderer);

  wl_event_source_timer_update(self->animation_timer_source,
      ANIMATION_UPDATE_MS * (self->alpha < BG_ALPHA));

  return 0;
}

static void
zn_vr_modal_on_click(struct zigzag_node *node, double x, double y)
{
  UNUSED(node);
  UNUSED(x);
  UNUSED(y);
}

static void
zn_vr_modal_render_headset_dialog(
    cairo_t *cr, double center_top_x, double center_top_y)
{
  double headset_dialog_height = 60;
  cairo_save(cr);

  zigzag_cairo_draw_rounded_rectangle(cr,
      center_top_x - HEADSET_DIALOG_WIDTH / 2, center_top_y,
      HEADSET_DIALOG_WIDTH, headset_dialog_height, 8.0);
  cairo_stroke_preserve(cr);
  cairo_set_line_width(cr, 1.0);
  cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.0);
  cairo_fill(cr);

  cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.5);
  cairo_set_font_size(cr, 13);
  zigzag_cairo_draw_text(cr, "Headset", center_top_x, center_top_y + 10,
      ZIGZAG_ANCHOR_CENTER, ZIGZAG_ANCHOR_TOP);
  cairo_set_font_size(cr, 12);

  zigzag_cairo_draw_text(cr, "192.168.2.0", center_top_x, center_top_y + 40,
      ZIGZAG_ANCHOR_CENTER, ZIGZAG_ANCHOR_CENTER);

  cairo_restore(cr);
}

static void
zn_vr_modal_render_exit_key_description(
    cairo_t *cr, double center_x, double center_y)
{
  cairo_save(cr);
  cairo_set_font_size(cr, 14);

  cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 1.0);
  zigzag_cairo_draw_text(cr, "Press    Meta    +    V    to exit VR mode",
      center_x, center_y, ZIGZAG_ANCHOR_CENTER, ZIGZAG_ANCHOR_CENTER);

  cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.3);
  zigzag_cairo_draw_rounded_rectangle(
      cr, center_x - 77, center_y - 8, 44, 20, 4);
  cairo_fill(cr);
  zigzag_cairo_draw_rounded_rectangle(
      cr, center_x - 14, center_y - 8, 24, 20, 4);
  cairo_fill(cr);

  cairo_restore(cr);
}

static bool
zn_vr_modal_render(struct zigzag_node *node, cairo_t *cr)
{
  struct zn_vr_modal *self = node->user_data;
  double center_x = node->frame.width / 2;
  double center_y = node->frame.height / 2;

  cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, self->alpha);
  cairo_paint(cr);

  cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, self->alpha);

  if (!zigzag_cairo_stamp_svg_on_surface(cr, VR_ICON, center_x - ICON_WIDTH / 2,
          (center_y - ICON_HEIGHT / 2) - 120, ICON_WIDTH, ICON_HEIGHT)) {
    return false;
  }

  cairo_set_font_size(cr, 32);
  zigzag_cairo_draw_text(cr, "You're in VR", center_x, center_y - 30,
      ZIGZAG_ANCHOR_CENTER, ZIGZAG_ANCHOR_CENTER);
  cairo_set_font_size(cr, 14);
  zigzag_cairo_draw_text(cr, "Put on your headset to start working in VR",
      center_x, center_y, ZIGZAG_ANCHOR_CENTER, ZIGZAG_ANCHOR_CENTER);

  zn_vr_modal_render_headset_dialog(cr, center_x, center_y + 60);

  zn_vr_modal_render_exit_key_description(
      cr, center_x, node->frame.height - 60);

  return true;
}

static void
zn_vr_modal_set_frame(
    struct zigzag_node *node, double screen_width, double screen_height)
{
  node->frame.x = 0.;
  node->frame.y = 0.;
  node->frame.width = screen_width;
  node->frame.height = screen_height;
}

static const struct zigzag_node_impl implementation = {
    .on_click = zn_vr_modal_on_click,
    .set_frame = zn_vr_modal_set_frame,
    .render = zn_vr_modal_render,
};

struct zn_vr_modal *
zn_vr_modal_create(
    struct zigzag_layout *zigzag_layout, struct wlr_renderer *renderer)
{
  struct zn_vr_modal *self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  struct zigzag_node *zigzag_node =
      zigzag_node_create(&implementation, zigzag_layout, renderer, self);

  if (zigzag_node == NULL) {
    zn_error("Failed to create a zigzag_node");
    goto err_vr_modal;
  }
  self->zigzag_node = zigzag_node;

  struct zn_server *server = zn_server_get_singleton();
  self->animation_timer_source =
      wl_event_loop_add_timer(server->loop, zn_vr_modal_handle_timer, self);
  wl_event_source_timer_update(
      self->animation_timer_source, ANIMATION_UPDATE_MS);

  return self;

err_vr_modal:
  free(self);

err:
  return NULL;
}

void
zn_vr_modal_destroy(struct zn_vr_modal *self)
{
  wl_event_source_remove(self->animation_timer_source);
  zigzag_node_destroy(self->zigzag_node);
  free(self);
}
