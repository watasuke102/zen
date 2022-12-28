#include <cairo.h>
#include <stdlib.h>
#include <wlr/render/wlr_renderer.h>
#include <zen-common.h>
#include <zigzag.h>

struct wlr_texture *
zigzag_node_render_texture(
    struct zigzag_node *self, struct wlr_renderer *renderer)
{
  struct wlr_texture *texture = NULL;
  cairo_surface_t *surface = cairo_image_surface_create(
      CAIRO_FORMAT_RGB24, self->frame->width, self->frame->height);

  if (cairo_surface_status(surface) != CAIRO_STATUS_SUCCESS) {
    zn_error("Failed to create cairo_surface");
    goto err_cairo_surface;
  }

  cairo_t *cr = cairo_create(surface);
  if (cairo_status(cr) != CAIRO_STATUS_SUCCESS) {
    zn_error("Failed to create cairo_t");
    goto err_cairo;
  }

  self->implementation->render(self, cr);

  texture = zigzag_wlr_texture_from_cairo_surface(surface, renderer);
err_cairo:
  cairo_destroy(cr);
err_cairo_surface:
  cairo_surface_destroy(surface);

  return texture;
}

struct zigzag_node *
zigzag_node_create(const struct zigzag_node_impl *implementation,
    struct zigzag_layout *layout, struct wlr_renderer *renderer,
    void *user_data)
{
  struct zigzag_node *self;
  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->layout = layout;
  self->user_data = user_data;
  self->implementation = implementation;

  wl_list_init(&self->node_list);
  self->frame = zalloc(sizeof self->frame);
  self->implementation->set_frame(
      self, layout->output_width, layout->output_height);

  self->texture = zigzag_node_render_texture(self, renderer);

err:
  return self;
}

void
zigzag_node_destroy(struct zigzag_node *self)
{
  wl_list_remove(&self->node_list);
  wl_list_remove(&self->link);
  wlr_texture_destroy(self->texture);
  free(self->frame);
  free(self);
}
