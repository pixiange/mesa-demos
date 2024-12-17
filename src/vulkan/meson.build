# Copyright © 2022 Collabora Ltd

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

glsl_shaders = files(
	'gear.frag',
	'gear.vert',
)

sources = files('vkgears.c', 'wsi/wsi.c')

args = []
wsi_deps = []

if dep_wayland.found()
  args += ['-DWAYLAND_SUPPORT']
  sources += files('wsi/wayland.c')
  wsi_deps += [dep_wayland, dep_epoll, dep_libdecor]
endif

if dep_xcb.found()
  args += ['-DXCB_SUPPORT']
  sources += files('wsi/xcb.c')
  wsi_deps += dep_xcb
endif

is_macos = host_machine.system() == 'darwin'
if is_macos
  args += ['-DMETAL_SUPPORT']
  sources += files('wsi/metal.m')
  wsi_deps += dependency(
    'appleframeworks',
    modules : ['AppKit', 'QuartzCore', 'Metal']
  )
endif

is_windows = host_machine.system() == 'windows'
if is_windows
  args += ['-DWIN32_SUPPORT']
  sources += files('wsi/win32.c')
endif

if (dep_wayland.found() or dep_xcb.found() or is_macos or is_windows) and prog_glslang.found()
  _gen = generator(
    prog_glslang,
    output : '@PLAINNAME@.spv.h',
    arguments : [ '@INPUT@', '-V', '-x', '-o', '@OUTPUT@' ]
  )

  spirv_shaders = _gen.process(glsl_shaders)

  executable(
    'vkgears', files('vkgears.c'), sources,
    spirv_shaders,
    dependencies: [dep_vulkan, idep_util, wsi_deps],
    c_args: args,
    objc_args: args,
    install: true
  )
endif

devenv = environment()
devenv.set('VK_LOADER_LAYERS_ENABLE', 'VK_LAYER_KHRONOS_validation')
devenv.set('VK_LAYER_SETTINGS_PATH', meson.current_source_dir())
meson.add_devenv(devenv)
