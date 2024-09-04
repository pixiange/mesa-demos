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

_deps = [dep_gl, dep_m, idep_util]

if target_machine.system() == 'linux'
  executable(
    'dmabufshare', files('dmabufshare.c'),
    dependencies: [_deps, idep_glad, idep_eglut],
    install: true
  )
endif
executable(
  'eglgears', files('eglgears.c'),
  dependencies: [_deps, dep_glu, idep_eglut],
  install: true
)
executable(
  'egltri', files('egltri.c'),
  dependencies: [_deps, dep_glu, idep_eglut],
  install: true
)
executable(
  'xeglgears', files('xeglgears.c'),
  dependencies: [_deps, dep_glu, dep_egl, dep_x11],
  install: true
)
executable(
  'xeglthreads', files('xeglthreads.c'),
  dependencies: [_deps, dep_egl, dep_x11],
  install: true
)

executable(
  'eglkms', 'eglkms.c',
  dependencies: [_deps, dep_drm, dep_gbm, dep_egl],
  install: true
)

executable(
  'eglinfo', 'eglinfo.c',
  dependencies: [dep_egl, idep_glad, idep_util],
  include_directories: [inc_glad],
  install: true
)

executable(
  'peglgears', 'peglgears.c',
  dependencies: [dep_gl, dep_glu, dep_egl, dep_m, idep_util],
  install: true
)

