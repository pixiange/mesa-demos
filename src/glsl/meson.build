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
  ['array', []],
  ['bitmap', []],
  ['brick', []],
  ['bump', [idep_readtex]],
  ['blinking-teapot', []],
  ['convolutions', [idep_readtex]],
  ['deriv', []],
  ['fragcoord', []],
  ['fsraytrace', []],
  ['geom-outlining-150', []],
  ['identity', []],
  ['linktest', []],
  ['mandelbrot', []],
  ['multinoise', []],
  ['multitex', [idep_readtex]],
  ['noise2', []],
  ['noise', []],
  ['pointcoord', []],
  ['points', []],
  ['samplers', []],
  ['shadow_sampler', []],
  ['shtest', [idep_readtex]],
  ['simplex-noise', []],
  ['skinning', []],
  ['texaaline', []],
  ['texdemo1', [idep_readtex]],
  ['toyball', []],
  ['trirast', []],
  ['twoside', []],
  ['vert-or-frag-only', []],
  ['vert-tex', []],
  ['vsraytrace', []],
]
base_deps = [
  dep_gl, dep_glu, dep_glut, dep_m, idep_glad, idep_util
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

executable(
  'gsraytrace', files('gsraytrace.cpp'),
  dependencies: [
    dep_gl, dep_glu, dep_glut, dep_m, idep_glad, idep_util
  ],
  install: true
)

executable(
  'samplers_array', files('samplers.c'),
  c_args: '-DSAMPLERS_ARRAY',
  dependencies: [
    dep_gl, dep_glu, dep_glut, dep_m, idep_glad, idep_util
  ],
  install: true
)

glsl_data = [
  'blinking-teapot.frag',
  'blinking-teapot.vert',
  'brick.shtest',
  'CH06-brick.frag',
  'CH06-brick.vert',
  'CH11-bumpmap.frag',
  'CH11-bumpmaptex.frag',
  'CH11-bumpmap.vert',
  'CH11-toyball.frag',
  'CH11-toyball.vert',
  'CH18-mandel.frag',
  'CH18-mandel.vert',
  'convolution.frag',
  'convolution.vert',
  'cubemap.frag',
  'mandelbrot.shtest',
  'multitex.frag',
  'multitex.shtest',
  'multitex.vert',
  'reflect.vert',
  'shadowtex.frag',
  'simple.vert',
  'simplex-noise.glsl',
  'skinning.frag',
  'skinning.vert',
  'toyball.shtest',
]

install_data(
  glsl_data,
  install_dir: get_option('datadir') / 'mesa-demos'
)
