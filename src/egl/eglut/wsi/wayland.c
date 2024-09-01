/*
 * Copyright © 2012-2022 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 */

#include <wayland-client.h>
#include <wayland-egl.h>

#include <assert.h>
#include <poll.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <sys/mman.h>
#include <sys/timerfd.h>
#include <unistd.h>
#include <xkbcommon/xkbcommon.h>

#include "eglutint.h"
#include "wsi.h"
#include <libdecor.h>

struct display {
   struct wl_display *display;
   struct wl_seat *wl_seat;
   struct wl_compositor *compositor;
   uint32_t mask;

   struct {
      struct wl_keyboard *keyboard;
      struct xkb_state *xkb_state;
      struct xkb_context *xkb_context;
      struct xkb_keymap *xkb_keymap;

      int key_repeat_fd;
      xkb_keycode_t repeat_keycode;

      int32_t rate, delay;
   } seat;
};

struct window {
   struct wl_surface *surface;
   struct libdecor *decor_context;
   struct libdecor_frame *frame;
   bool open;
   bool opaque;
   bool configured;
   int floating_width;
   int floating_height;
};

static struct display display = {0};
static struct window window = {0};

static void
keymap_callback(void *data, struct wl_keyboard *wl_keyboard,
                uint32_t format, int32_t fd, uint32_t size)
{
   struct display *d = data;
   assert(format == WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1);

   char *map_shm = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
   assert(map_shm != MAP_FAILED);

   struct xkb_keymap *xkb_keymap = xkb_keymap_new_from_string(
      d->seat.xkb_context, map_shm, XKB_KEYMAP_FORMAT_TEXT_V1,
      XKB_KEYMAP_COMPILE_NO_FLAGS);
   munmap(map_shm, size);
   close(fd);

   struct xkb_state *xkb_state = xkb_state_new(xkb_keymap);
   xkb_keymap_unref(d->seat.xkb_keymap);
   xkb_state_unref(d->seat.xkb_state);
   d->seat.xkb_keymap = xkb_keymap;
   d->seat.xkb_state = xkb_state;
}


static int
lookup_keysym(xkb_keysym_t sym)
{
   switch (sym) {

   /* function keys */
   case XKB_KEY_F1: return EGLUT_KEY_F1;
   case XKB_KEY_F2: return EGLUT_KEY_F2;
   case XKB_KEY_F3: return EGLUT_KEY_F3;
   case XKB_KEY_F4: return EGLUT_KEY_F4;
   case XKB_KEY_F5: return EGLUT_KEY_F5;
   case XKB_KEY_F6: return EGLUT_KEY_F6;
   case XKB_KEY_F7: return EGLUT_KEY_F7;
   case XKB_KEY_F8: return EGLUT_KEY_F8;
   case XKB_KEY_F9: return EGLUT_KEY_F9;
   case XKB_KEY_F10: return EGLUT_KEY_F10;
   case XKB_KEY_F11: return EGLUT_KEY_F11;
   case XKB_KEY_F12: return EGLUT_KEY_F12;

   /* directional keys */

   case XKB_KEY_Left:
   case XKB_KEY_KP_Left:
      return EGLUT_KEY_LEFT;

   case XKB_KEY_Up:
   case XKB_KEY_KP_Up:
      return EGLUT_KEY_UP;

   case XKB_KEY_Right:
   case XKB_KEY_KP_Right:
      return EGLUT_KEY_RIGHT;

   case XKB_KEY_Down:
   case XKB_KEY_KP_Down:
      return EGLUT_KEY_DOWN;

   default:
      return -1;
   }
}

static void
emit_keypress(struct display *d, xkb_keycode_t keycode)
{
   struct eglut_window *win = _eglut->current;

   xkb_keysym_t sym = xkb_state_key_get_one_sym(d->seat.xkb_state, keycode);
   int special = lookup_keysym(sym);
   if (special >= 0 && win->special_cb)
      win->special_cb(special);
   else {
      uint32_t utf32 = xkb_keysym_to_utf32(sym);
      if (utf32 && utf32 < 128)
         win->keyboard_cb(utf32);
   }
}

static void
handle_key(struct display *d, xkb_keycode_t keycode,
           enum wl_keyboard_key_state state)
{
   struct itimerspec timer = {0};
   if (d->seat.rate != 0 &&
       xkb_keymap_key_repeats(d->seat.xkb_keymap, keycode) &&
       state == WL_KEYBOARD_KEY_STATE_PRESSED) {
      d->seat.repeat_keycode = keycode;
      if (d->seat.rate > 1)
         timer.it_interval.tv_nsec = 1000000000 /  d->seat.rate;
      else
         timer.it_interval.tv_sec = 1;

      timer.it_value.tv_sec =  d->seat.delay / 1000;
      timer.it_value.tv_nsec = ( d->seat.delay % 1000) * 1000000;
   }
   timerfd_settime(d->seat.key_repeat_fd, 0, &timer, NULL);

   if (state == WL_KEYBOARD_KEY_STATE_PRESSED)
      emit_keypress(d, keycode);
}

static void
enter_callback(void *data, struct wl_keyboard *wl_keyboard,
               uint32_t serial, struct wl_surface *surface,
               struct wl_array *keys)
{
   struct display *d = data;
   uint32_t *key;
   wl_array_for_each(key, keys)
      handle_key(d, *key + 8, WL_KEYBOARD_KEY_STATE_PRESSED);
}

static void
leave_callback(void *data, struct wl_keyboard *wl_keyboard,
               uint32_t serial, struct wl_surface *surface)
{
   struct display *d = data;
   struct itimerspec timer = {0};
   timerfd_settime(d->seat.key_repeat_fd, 0, &timer, NULL);
}

static void
key_callback(void *data, struct wl_keyboard *keyboard,
             uint32_t serial, uint32_t time, uint32_t key,
             uint32_t state)
{
   struct display *d = data;
   handle_key(d, key + 8, state);
}

static void
modifiers_callback(void *data, struct wl_keyboard *wl_keyboard,
                   uint32_t serial, uint32_t mods_depressed,
                   uint32_t mods_latched, uint32_t mods_locked,
                   uint32_t group)
{
   struct display *d = data;
   xkb_state_update_mask(d->seat.xkb_state, mods_depressed, mods_latched,
                         mods_locked, 0, 0, group);
}

static void
repeat_info_callback(void *data, struct wl_keyboard *wl_keyboard,
                        int32_t rate, int32_t delay)
{
   struct display *d = data;
   d->seat.rate = rate;
   d->seat.delay = delay;
}

static const struct wl_keyboard_listener keyboard_listener = {
   keymap_callback,
   enter_callback,
   leave_callback,
   key_callback,
   modifiers_callback,
   repeat_info_callback,
};

static void
seat_capabilities(void *data, struct wl_seat *seat,
                  enum wl_seat_capability caps)
{
   struct display *d = data;
   if (caps & WL_SEAT_CAPABILITY_KEYBOARD) {
      d->seat.keyboard = wl_seat_get_keyboard(seat);
      wl_keyboard_add_listener(d->seat.keyboard, &keyboard_listener, data);
      d->seat.key_repeat_fd = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC | TFD_NONBLOCK);
   } else if (!(caps & WL_SEAT_CAPABILITY_KEYBOARD)) {
      wl_keyboard_destroy(d->seat.keyboard);
      d->seat.keyboard = NULL;
   }
}

static void
seat_name(void* data, struct wl_seat* wl_seat,
          const char* name)
{
}

static const struct wl_seat_listener seat_listener = {
   seat_capabilities,
   seat_name,
};

static void
registry_handle_global(void *data, struct wl_registry *registry, uint32_t id,
                       const char *interface, uint32_t version)
{
   struct display *d = data;

   if (strcmp(interface, wl_compositor_interface.name) == 0) {
      d->compositor =
         wl_registry_bind(registry, id, &wl_compositor_interface, 1);
   } else if (strcmp(interface, wl_seat_interface.name) == 0) {
      d->wl_seat =
         wl_registry_bind(registry, id, &wl_seat_interface, 4);
      wl_seat_add_listener(d->wl_seat, &seat_listener, d);
   }
}

static void
registry_handle_global_remove(void *data, struct wl_registry *registry,
                              uint32_t name)
{
}

static const struct wl_registry_listener registry_listener = {
   registry_handle_global,
   registry_handle_global_remove
};

static void
sync_callback(void *data, struct wl_callback *callback, uint32_t serial)
{
   int *done = data;

   *done = 1;
   wl_callback_destroy(callback);
}

static const struct wl_callback_listener sync_listener = {
   sync_callback
};

static int
wayland_roundtrip(struct wl_display *display)
{
   struct wl_callback *callback;
   int done = 0, ret = 0;

   callback = wl_display_sync(display);
   wl_callback_add_listener(callback, &sync_listener, &done);
   while (ret != -1 && !done)
      ret = wl_display_dispatch(display);

   if (!done)
      wl_callback_destroy(callback);

   return ret;
}

static void
init_display(void)
{
   struct wl_registry *registry;

   _eglut->native_dpy = display.display = wl_display_connect(NULL);

   if (!_eglut->native_dpy)
      _eglutFatal("failed to initialize native display");

   display.seat.xkb_context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);

   registry = wl_display_get_registry(_eglut->native_dpy);
   wl_registry_add_listener(registry, &registry_listener, &display);
   wayland_roundtrip(_eglut->native_dpy);
   wl_registry_destroy(registry);

   _eglut->surface_type = EGL_WINDOW_BIT;
   _eglut->redisplay = 1;
}

static void
fini_display(void)
{
   if (display.seat.keyboard)
      wl_keyboard_destroy(display.seat.keyboard);

   wl_seat_destroy(display.wl_seat);
   xkb_context_unref(display.seat.xkb_context);
   wl_compositor_destroy(display.compositor);
   wl_display_flush(_eglut->native_dpy);
   wl_display_disconnect(_eglut->native_dpy);
   _eglut->native_dpy = NULL;
}

static void
libdecor_error(struct libdecor *context,
               enum libdecor_error error,
               const char *message)
{
   printf("EGLUT: libdecor error %d due to %s\n", error, message);
}

static struct libdecor_interface libdecor_interface = {
   .error = libdecor_error,
};

static void
frame_configure(struct libdecor_frame *frame,
                struct libdecor_configuration *configuration,
                void *user_data)
{
   struct eglut_window *win = user_data;
   struct libdecor_state *state;
   int width, height;

   if (!libdecor_configuration_get_content_size(configuration, frame,
                                          &width, &height)) {
      width = window.floating_width;
      height = window.floating_height;
   }

   win->native.width = width;
   win->native.height = height;

   wl_egl_window_resize(win->native.u.window,
                        win->native.width,
                        win->native.height,
                        0,
                        0);

   if (win->reshape_cb)
      win->reshape_cb(win->native.width, win->native.height);

   state = libdecor_state_new(width, height);
   libdecor_frame_commit(frame, state, configuration);
   libdecor_state_free(state);

   /* store floating dimensions */
   if (libdecor_frame_is_floating(frame)) {
      window.floating_width = width;
      window.floating_height = height;
   }

   window.configured = true;
}

static void
frame_close(struct libdecor_frame *frame, void *user_data)
{
   struct eglut_window *win = user_data;
   eglutDestroyWindow(win->index);

   // FIXME: eglut does not terminate when all windows are closed.
   // eglut_x11 dies due to "X connection to $DISPLAY broken".
   // Since wl_display works fine with all windows closed, terminate ourselves.
   eglTerminate(_eglut->dpy);
   fini_display();

   window.open = false;
}

static void
frame_commit(struct libdecor_frame *frame, void *user_data)
{
   /* struct eglut_window *window = user_data; */
   _eglut->redisplay = true;
}


static struct libdecor_frame_interface frame_interface = {
   .configure = frame_configure,
   .close = frame_close,
   .commit = frame_commit,
};

static void
init_window(struct eglut_window *win, const char *title,
            int x, int y, int w, int h)
{
   struct wl_egl_window *native;

   window.surface = wl_compositor_create_surface(display.compositor);

   EGLint alpha_size;
   if (!eglGetConfigAttrib(_eglut->dpy,
            win->config, EGL_ALPHA_SIZE, &alpha_size))
      _eglutFatal("failed to get alpha size");
   window.opaque = !alpha_size;

   native = wl_egl_window_create(window.surface, w, h);

   win->native.u.window = native;
   win->native.width = w;
   win->native.height = h;

   window.decor_context = libdecor_new(display.display,
                                       &libdecor_interface);
   window.frame = libdecor_decorate(window.decor_context,
                                    window.surface,
                                    &frame_interface,
                                    win);
   window.floating_width = w;
   window.floating_height = h;
   libdecor_frame_set_app_id(window.frame, title);
   libdecor_frame_set_title(window.frame, title);
   libdecor_frame_map(window.frame);

   libdecor_frame_set_min_content_size(window.frame, 1, 1);
}

static void
fini_window(struct eglut_window *win)
{
   wl_egl_window_destroy(win->native.u.window);

   if (window.decor_context)
      libdecor_unref(window.decor_context);
}

static void
draw(struct window *window)
{
   struct eglut_window *win = _eglut->current;

   /* Our client doesn't want to push another frame; go back to sleep. */
   if (!_eglut->redisplay)
      return;
   _eglut->redisplay = 0;

   if (win->display_cb)
      win->display_cb();

   if (window->opaque) {
      struct wl_region *region =
         wl_compositor_create_region(display.compositor);
      wl_region_add(region, 0, 0, win->native.width, win->native.height);
      wl_surface_set_opaque_region(window->surface, region);
      wl_region_destroy(region);
   }

   eglSwapBuffers(_eglut->dpy, win->surface);
}

static void
event_loop(void)
{
   int ret;

   while (!window.configured) {
      if (libdecor_dispatch(window.decor_context, 0) < 0)
         return;
   }

   struct pollfd pollfds[] = {
      {
         .fd = wl_display_get_fd(display.display),
         .events = POLLIN,
      },
      {
         .fd = libdecor_get_fd(window.decor_context),
         .events = POLLIN,
      }, {
         .fd = display.seat.key_repeat_fd,
         .events = POLLIN,
      },
   };

   while (_eglut->native_dpy != NULL) {
      /* If we need to flush but can't, don't do anything at all which could
       * push further events into the socket. */
      if (!(pollfds[0].events & POLLOUT)) {
         wl_display_dispatch_pending(display.display);

         if (_eglut->idle_cb)
            _eglut->idle_cb();

         /* Client wants to redraw, but we have no frame event to trigger the
          * redraw; kickstart it by redrawing immediately. */
         if (_eglut->redisplay)
            draw(&window);
      }

      ret = wl_display_flush(display.display);
      if (ret < 0 && errno != EAGAIN)
         break; /* fatal error; socket is broken */
      else if (ret < 0 && errno == EAGAIN)
         pollfds[0].events |= POLLOUT; /* need to wait until we can flush */
      else
         pollfds[0].events &= ~POLLOUT; /* successfully flushed */

      unsigned poll_count = 2 + (display.seat.rate > 0);
      if (poll(pollfds, poll_count, -1) == -1)
         break;

      if ((pollfds[0].revents | pollfds[1].revents) &
          (POLLERR | POLLHUP | POLLNVAL))
         break;

      if (pollfds[0].events & POLLOUT) {
         if (!(pollfds[0].revents & POLLOUT))
            continue; /* block until we can flush */
         pollfds[0].events &= ~POLLOUT;
      }

      if (pollfds[0].revents & POLLIN) {
         ret = wl_display_dispatch(display.display);
         if (ret == -1)
            break;
      }

      ret = wl_display_flush(display.display);
      if (ret < 0 && errno != EAGAIN)
         break; /* fatal error; socket is broken */
      else if (ret < 0 && errno == EAGAIN)
         pollfds[0].events |= POLLOUT; /* need to wait until we can flush */
      else
         pollfds[0].events &= ~POLLOUT; /* successfully flushed */

      if (pollfds[1].revents & POLLIN) {
         if (window.open && libdecor_dispatch(window.decor_context, 0) < 0) {
            ret = 1;
            break;
         }
      }

      if (pollfds[2].revents & POLLIN) {
         uint64_t repeats;
         if (read(display.seat.key_repeat_fd, &repeats, sizeof(repeats)) == 8) {
            for (uint64_t i = 0; i < repeats; i++)
               emit_keypress(&display, display.seat.repeat_keycode);
         }
      }
   }
}

struct eglut_wsi_interface
wayland_wsi_interface(void)
{
   return (struct eglut_wsi_interface) {
      .init_display = init_display,
      .fini_display = fini_display,
      .init_window = init_window,
      .fini_window = fini_window,
      .event_loop = event_loop,
   };
}
