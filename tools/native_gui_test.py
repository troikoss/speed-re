"""Display-free SDL/OpenGL setup shared by native graphical verification scripts."""
import os


def add_render_arguments(parser):
    mode = parser.add_mutually_exclusive_group()
    mode.add_argument('--headless', dest='headless', action='store_true',
                      help='use SDL offscreen software rendering (default; no display server needed)')
    mode.add_argument('--headed', dest='headless', action='store_false',
                      help='use the normal SDL video driver and desktop display')
    parser.set_defaults(headless=True)


def render_environment(args, **extra):
    environment = dict(os.environ, **extra)
    if args.headless:
        environment.update(SDL_VIDEODRIVER='offscreen', LIBGL_ALWAYS_SOFTWARE='1')
        environment.pop('DISPLAY', None)
        environment.pop('WAYLAND_DISPLAY', None)
    return environment
