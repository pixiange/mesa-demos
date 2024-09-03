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

subdir('glad')

subdir('util')

if dep_glut.found()
  subdir('demos')
  subdir('tests')
  subdir('trivial')
  subdir('redbook')
  subdir('samples')
  subdir('objviewer')

  subdir('glsl')
  subdir('fpglsl')
  subdir('vpglsl')
  subdir('tools')
endif

if dep_egl.found()
  subdir('egl')
endif

if dep_x11.found()
  subdir('xdemos')
endif

if dep_osmesa.found()
  subdir('osdemos')
endif

if dep_vulkan.found()
  subdir('vulkan')
endif

if host_machine.system() == 'windows'
  subdir('wgl')
endif

subdir('data')
