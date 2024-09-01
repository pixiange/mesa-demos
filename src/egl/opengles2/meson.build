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

_deps_x11 = [dep_gles2, dep_egl, dep_x11]
executable(
  'es2_info', files('es2_info.c'),
  dependencies: _deps_x11,
  install: true
)
executable(
  'es2gears', files('es2gears.c'),
  dependencies: [dep_gles2, idep_eglut, idep_util],
  install: true
)
executable(
  'es2tri', files('es2tri.c'),
  dependencies: [dep_gles2, idep_eglut, idep_util],
  install: true
)
executable(
  'texture_from_pixmap_glesv2', files('texture_from_pixmap_glesv2.c'),
  dependencies: [_deps_x11, idep_util],
)
