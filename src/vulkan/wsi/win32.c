/*
 * Copyright © 2024 Collabora Ltd
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <windows.h>

#include "wsi.h"

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>

#include <stdio.h>

static struct wsi_callbacks wsi_callbacks;

static HMODULE hInstance;
static ATOM window_class;
static HWND hWnd;

static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

static void init_display()
{
#if WINVER >= 0x0605
   SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE);
#endif

   hInstance = GetModuleHandle(NULL);
   window_class = RegisterClassEx(
      &(WNDCLASSEX) {
         .cbSize = sizeof(WNDCLASSEX),
         .style = CS_HREDRAW | CS_VREDRAW,
         .lpfnWndProc = WndProc,
         .hInstance = hInstance,
         .hIcon = LoadIcon(NULL, IDI_WINLOGO),
         .hCursor = LoadCursor(NULL, IDC_ARROW),
         .lpszClassName = "vulkan-win32",
      });
}

static void
fini_display()
{
   UnregisterClass(MAKEINTATOM(window_class), hInstance);
}

static void init_window(const char *title, int width, int height,
                        bool fullscreen)
{
   int x = CW_USEDEFAULT, y = CW_USEDEFAULT;

   if (fullscreen) {
      x = y = 0;
      width = GetSystemMetrics(SM_CXSCREEN);
      height = GetSystemMetrics(SM_CYSCREEN);
   }

   RECT winrect = { 0, 0, width, height };
   DWORD dwStyle, dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
   if (!fullscreen) {
      dwStyle = WS_OVERLAPPEDWINDOW;
      AdjustWindowRectEx(&winrect, dwStyle, FALSE, dwExStyle);
   }
   else {
      dwStyle = WS_POPUP;
   }

   hWnd = CreateWindowEx(dwExStyle, MAKEINTATOM(window_class), title, dwStyle,
                         x, y,
                         winrect.right - winrect.left,
                         winrect.bottom - winrect.top,
                         NULL, NULL, hInstance, NULL);

   ShowWindow(hWnd, SW_NORMAL);
}

static void fini_window()
{
   DestroyWindow(hWnd);
}

static bool update_window()
{
   MSG msg;
   while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
   }
   return false;
}

static void
set_wsi_callbacks(struct wsi_callbacks callbacks)
{
   wsi_callbacks = callbacks;
}

#define GET_INSTANCE_PROC(name) \
   PFN_ ## name name = (PFN_ ## name)vkGetInstanceProcAddr(instance, #name);

static bool
create_surface(VkPhysicalDevice physical_device, VkInstance instance,
               VkSurfaceKHR *surface)
{
   GET_INSTANCE_PROC(vkGetPhysicalDeviceWin32PresentationSupportKHR)
   GET_INSTANCE_PROC(vkCreateWin32SurfaceKHR)

   if (!vkGetPhysicalDeviceWin32PresentationSupportKHR ||
       !vkCreateWin32SurfaceKHR) {
      fprintf(stderr, "Failed to load extension functions\n");
      return false;
   }

   if (!vkGetPhysicalDeviceWin32PresentationSupportKHR(physical_device, 0)) {
      fprintf(stderr, "Vulkan not supported on given Win32 surface\n");
      return false;
   }

   return vkCreateWin32SurfaceKHR(
      instance,
      &(VkWin32SurfaceCreateInfoKHR) {
         .sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
         .hinstance = hInstance,
         .hwnd = hWnd,
      }, NULL, surface) == VK_SUCCESS;
}

struct wsi_interface
win32_wsi_interface(void) {
   return (struct wsi_interface) {
      .required_extension_name = VK_KHR_WIN32_SURFACE_EXTENSION_NAME,

      .init_display = init_display,
      .fini_display = fini_display,

      .init_window = init_window,
      .update_window = update_window,
      .fini_window = fini_window,

      .set_wsi_callbacks = set_wsi_callbacks,

      .create_surface = create_surface,
   };
}

static enum wsi_key map_keycode(UINT code)
{
   switch (code) {
   case VK_UP:
      return WSI_KEY_UP;
      break;
   case VK_DOWN:
      return WSI_KEY_DOWN;
      break;
   case VK_LEFT:
      return WSI_KEY_LEFT;
      break;
   case VK_RIGHT:
      return WSI_KEY_RIGHT;
      break;
   case 'A':
      return WSI_KEY_A;
      break;
   case VK_ESCAPE:
      return WSI_KEY_ESC;
      break;
   default:
      return WSI_KEY_OTHER;
   }
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
   switch (message) {
#if WINVER >= 0x0605
   case WM_NCCREATE:
      EnableNonClientDpiScaling(hWnd);
      return DefWindowProc(hWnd, message, wParam, lParam);
#endif

   case WM_QUIT:
   case WM_CLOSE:
      wsi_callbacks.exit();
      break;

   case WM_SIZE:
      wsi_callbacks.resize(LOWORD(lParam), HIWORD(lParam));
      break;

   case WM_KEYDOWN:
   case WM_KEYUP:
      wsi_callbacks.key_press(message == WM_KEYDOWN,
                              map_keycode(LOWORD(wParam)));
      break;

   case WM_DESTROY:
      PostQuitMessage(0);
      break;

   default:
      return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return 0;
}
