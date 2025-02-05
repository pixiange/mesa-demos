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
  ['arbfslight', []],
  ['arbocclude', []],
  ['arbocclude2', []],
  ['bounce', []],
  ['clearspd', []],
  ['copypix', [idep_readtex]],
  ['cubemap', [idep_readtex]],
  ['cuberender', []],
  ['dinoshade', []],
  ['dissolve', [idep_readtex]],
  ['drawpix', [idep_readtex]],
  ['engine', [idep_readtex]],
  ['fbo_firecube', [idep_readtex]],
  ['fbotexture', []],
  ['fire', [idep_readtex]],
  ['fogcoord', []],
  ['fplight', []],
  ['fslight', []],
  ['gamma', []],
  ['gearbox', []],
  ['gears', []],
  ['geartrain', []],
  ['glinfo', []],
  ['gloss', [idep_readtex]],
  ['gltestperf', []],
  ['ipers', [idep_readtex]],
  ['isosurf', [idep_readtex]],
  ['lodbias', [idep_readtex]],
  ['morph3d', []],
  ['multiarb', [idep_readtex]],
  ['paltex', []],
  ['pixeltest', []],
  ['pointblast', []],
  ['projtex', [idep_readtex]],
  ['ray', []],
  ['readpix', [idep_readtex]],
  ['reflect', [idep_readtex]],
  ['renormal', []],
  ['shadowtex', []],
  ['singlebuffer', []],
  ['spectex', []],
  ['spriteblast', []],
  ['stex3d', []],
  ['teapot', [idep_readtex]],
  ['terrain', []],
  ['tessdemo', []],
  ['texcyl', [idep_readtex]],
  ['texenv', []],
  ['textures', [idep_readtex]],
  ['trispd', []],
  ['tunnel2', [idep_readtex]],
  ['tunnel', [idep_readtex]],
  ['vao_demo', []],
  ['winpos', [idep_readtex]],
]
base_deps = [
  dep_gl, dep_glu, dep_glut, dep_m, idep_glad, idep_util, dep_winmm
]
foreach pair : progs
  p = pair[0]
  extra_deps = pair[1]
  executable(
    p, files(p + '.c'),
    dependencies: base_deps + extra_deps,
    install: true
  )
endforeach
