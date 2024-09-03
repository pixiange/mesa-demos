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
  'arbfslight',
  'arbocclude',
  'arbocclude2',
  'bounce',
  'clearspd',
  'copypix',
  'cubemap',
  'cuberender',
  'dinoshade',
  'dissolve',
  'drawpix',
  'engine',
  'fbo_firecube',
  'fbotexture',
  'fire',
  'fogcoord',
  'fplight',
  'fslight',
  'gamma',
  'gearbox',
  'gears',
  'geartrain',
  'glinfo',
  'gloss',
  'gltestperf',
  'ipers',
  'isosurf',
  'lodbias',
  'morph3d',
  'multiarb',
  'paltex',
  'pixeltest',
  'pointblast',
  'projtex',
  'ray',
  'readpix',
  'reflect',
  'renormal',
  'shadowtex',
  'singlebuffer',
  'spectex',
  'spriteblast',
  'stex3d',
  'teapot',
  'terrain',
  'tessdemo',
  'texcyl',
  'texenv',
  'textures',
  'trispd',
  'tunnel2',
  'tunnel',
  'vao_demo',
  'winpos',
]
foreach p : progs
  executable(
    p, files(p + '.c'),
    dependencies: [
      dep_gl, dep_glu, dep_glut, dep_m, idep_glad, idep_util, dep_winmm
    ],
    install: true
  )
endforeach
