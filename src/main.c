#include <signal.h>
#include <stdlib.h>

#include "server.h"
#include "zen-common.h"

static struct zn_server *server;

static void
zn_terminate(int exit_code)
{
  zn_server_terminate(server, exit_code);
}

static int
on_term_signal(int signal_number, void *data)
{
  UNUSED(data);

  zn_info("Caught signal %d", signal_number);
  zn_server_terminate(server, EXIT_FAILURE);

  return 1;
}

int
main()
{
  struct wl_display *display;
  struct wl_event_loop *loop;
  struct wl_event_source *signal_sources[3];
  int i, exit_status = EXIT_FAILURE;

  zn_log_init(ZEN_INFO, zn_terminate);

  display = wl_display_create();
  if (display == NULL) {
    zn_error("Failed to create a wayland display");
    goto err;
  }

  loop = wl_display_get_event_loop(display);
  signal_sources[0] =
      wl_event_loop_add_signal(loop, SIGTERM, on_term_signal, display);
  signal_sources[1] =
      wl_event_loop_add_signal(loop, SIGINT, on_term_signal, display);
  signal_sources[2] =
      wl_event_loop_add_signal(loop, SIGQUIT, on_term_signal, display);

  if (!signal_sources[0] || !signal_sources[1] || !signal_sources[2]) {
    zn_error("Failed to add signal handler");
    goto err_signal;
  }

  server = zn_server_create(display);
  if (server == NULL) {
    zn_error("Failed to create a zen server");
    goto err_signal;
  }

  exit_status = zn_server_run(server);

  zn_server_destroy(server);
  server = NULL;

err_signal:
  for (i = ARRAY_LENGTH(signal_sources) - 1; i >= 0; i--)
    if (signal_sources[i]) wl_event_source_remove(signal_sources[i]);

  wl_display_destroy_clients(display);
  wl_display_destroy(display);

err:
  zn_info("Exited gracefully");
  return exit_status;
}
