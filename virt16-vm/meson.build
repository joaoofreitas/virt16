project('virt16_vm', 'cpp',
  version : '0.1.0',
  default_options : ['cpp_std=c++20'])

imgui_dir = 'imgui'

imgui_sources = files(
  imgui_dir / 'imgui.cpp',
  imgui_dir / 'imgui_draw.cpp',
  imgui_dir / 'imgui_tables.cpp',
  imgui_dir / 'imgui_widgets.cpp',
  imgui_dir / 'imgui_demo.cpp',
  imgui_dir / 'backends' / 'imgui_impl_glfw.cpp',
  imgui_dir / 'backends' / 'imgui_impl_opengl3.cpp',
)

sources = files(
  'src/main.cpp',
  'src/ui.cpp',
  'src/vm/virt16.cpp',
)

cc = meson.get_compiler('cpp')

deps = [
  dependency('gl'),
  dependency('glfw3'),
]

if host_machine.system() == 'linux'
  deps += cc.find_library('dl', required : false)
  deps += dependency('x11', required : false)
endif

executable('virt16_vm',
  sources + imgui_sources,
  include_directories : include_directories('src', imgui_dir, imgui_dir / 'backends'),
  dependencies : deps,
)
