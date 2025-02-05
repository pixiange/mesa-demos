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

progs = [
  ['accum', []],
  ['bitmap1', []],
  ['bitmap2', []],
  ['blendeq', []],
  ['blendxor', []],
  ['copy', []],
  ['cursor', []],
  ['depth', []],
  ['eval', []],
  ['fog', []],
  ['font', []],
  ['line', []],
  ['logo', []],
  ['nurb', []],
  ['olympic', []],
  ['overlay', []],
  ['point', []],
  ['prim', []],
  ['quad', []],
  ['select', []],
  ['shape', []],
  ['sphere', [idep_readtex]],
  ['star', []],
  ['stencil', []],
  ['stretch', []],
  ['texture', []],
  ['tri', []],
  ['wave', []],
]
base_deps = [
  dep_gl, dep_glu, dep_glut, dep_m, idep_glad, idep_util
]
foreach pair : progs
  p = pair[0]
  extra_deps = pair[1]
  executable(
    p, files(p + '.c'),
    dependencies: base_deps + extra_deps)
endforeach
