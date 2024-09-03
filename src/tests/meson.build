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

deps = [dep_gl, dep_glu, dep_m, idep_glad, idep_util]

# glut programs
glut_progs = [
  'afsmultiarb',
  'antialias',
  'arbnpot',
  'arbnpot-mipmap',
  'backspecular',
  'blendxor',
  'blitfb',
  'bufferobj',
  'bumpmap',
  'bug_3050',
  'bug_3101',
  'bug_3195',
  'bug_texstore_i8',
  'calibrate_rast',
  'clip',
  'condrender',
  'copypixrate',
  'cva',
  'cva_huge',
  'cylwrap',
  'drawbuffers',
  'drawbuffers2',
  'drawstencil',
  'exactrast',
  'floattex',
  'fbotest1',
  'fbotest2',
  'fbotest3',
  'fillrate',
  'fogcoord',
  'fptest1',
  'fptexture',
  'front-back-modes',
  'getteximage',
  'glutfx',
  'interleave',
  'invert',
  'lineclip',
  'linehacks',
  'line-sample',
  'manytex',
  'mapbufrange',
  'minmag',
  'mipgen',
  'mipmap_comp',
  'mipmap_comp_tests',
  'mipmap_limits',
  'mipmap_view',
  'mipmap_tunnel',
  'multipal',
  'multitexarray',
  'multiwindow',
  'no_s3tc',
  'packedpixels',
  'pbo',
  'persp_hint',
  'prim',
  'quads',
  'random',
  'readrate',
  'rubberband',
  'scissor',
  'scissor-viewport',
  'seccolor',
  'shader_api',
  'shader-interp',
  'shadow-sample',
  'stencilreaddraw',
  'stencilwrap',
  'step',
  'streaming_rect',
  'subtex',
  'subtexrate',
  'tex1d',
  'texcmp',
  'texcompress2',
  'texcompsub',
  'texdown',
  'texfilt',
  'texgenmix',
  'texline',
  'texobj',
  'texrect',
  'unfilledclip',
  'viewmemory',
  'vparray',
  'vpeval',
  'vptest1',
  'vptest2',
  'vptest3',
  'vptorus',
  'vpwarpmesh',
  'yuvrect',
  'yuvsquare',
  'zbitmap',
  'zcomp',
  'zdrawpix',
  'zreaddraw',
]

if host_machine.system() != 'windows'
  glut_progs += 'texleak'
endif

foreach p : glut_progs
  executable(
    p, files(p + '.c'),
    dependencies: [deps, dep_glut]
  )
endforeach

x11_progs = [
  'auxbuffer',
  'jkrahntest',
  'sharedtex',
  'texobjshare',
]
foreach p : x11_progs
  executable(
    p, files(p + '.c'),
    dependencies: [deps, dep_x11, dep_glx]
  )
endforeach
