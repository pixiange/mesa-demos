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

inc_util = include_directories('.')

files_libutil = files(
  'glinfo_common.c',
  'showbuffer.c',
  'trackball.c',
  'matrix.c',
)

_deps = [dep_glu, dep_m]
if dep_glut.found()
  files_libutil += files('shaderutil.c')
  _deps += dep_glut
endif

_libutil = static_library(
  'util',
  files_libutil,
  include_directories: inc_glad,
  dependencies: _deps,
)

idep_util = declare_dependency(
  link_with: _libutil,
  include_directories: inc_util,
)

_libreadtex = static_library(
  'readtex',
  files('readtex.c'),
  include_directories: inc_glad,
  dependencies: _deps + [dep_png],
)
idep_readtex = declare_dependency(
  link_with: [_libreadtex],
  include_directories: inc_util,
)
