# -*- coding: utf-8 -*-
import glfw
import OpenGL.GL as gl
import imgui
from imgui.integrations.glfw import GlfwRenderer
import numpy as np


def key_callback(window, key, scancode, action, mods):
    if key == glfw.KEY_ESCAPE:
        glfw.set_window_should_close(window, True)


class cRender:
    def __init__(self, window_name="", window_size=(800, 600)):
        self.widnow_name = window_name
        self.window_width = window_size[0]
        self.window_height = window_size[1]
        self.window = cRender._initGL(self.window_width, self.window_height,
                                      self.widnow_name)
        imgui.create_context()
        self.impl = GlfwRenderer(self.window, False)

    @staticmethod
    def _initGL(width, height, window_name):

        if not glfw.init():
            print("Could not initialize OpenGL context")
            exit(1)

        # OS X supports only forward-compatible core profiles from 3.2
        glfw.window_hint(glfw.CONTEXT_VERSION_MAJOR, 3)
        glfw.window_hint(glfw.CONTEXT_VERSION_MINOR, 3)
        glfw.window_hint(glfw.OPENGL_PROFILE, glfw.OPENGL_CORE_PROFILE)

        glfw.window_hint(glfw.OPENGL_FORWARD_COMPAT, gl.GL_TRUE)

        # Create a windowed mode window and its OpenGL context
        window = glfw.create_window(int(width), int(height), window_name, None,
                                    None)
        glfw.make_context_current(window)
        glfw.set_key_callback(window, key_callback)
        if not window:
            glfw.terminate()
            print("Could not initialize Window")
            exit(1)
        return window

    def pre_update(self):
        glfw.poll_events()
        self.impl.process_inputs()
        gl.glClearColor(1., 1., 1., 1)
        gl.glClear(gl.GL_COLOR_BUFFER_BIT)
        imgui.new_frame()

    def need_to_resize(self, height, width):
        cur_width, cur_height = glfw.get_window_size(self.window)
        # print(f"[check_resize] cur {cur_width} {cur_height}")
        # print(f"[check_resize] desired {width} {height}")
        if cur_height == height and width == cur_width:
            return False
        else:
            # print(f"[check_resize] return true")
            return True

    def resize(self, height, width):
        glfw.set_window_size(self.window, width, height)
        # new_width, new_height = glfw.get_window_size(self.window)
        # print(new_width, width)
        # print(new_height, height)
        # exit()

    def post_update(self):
        imgui.render()
        self.impl.render(imgui.get_draw_data())
        glfw.swap_buffers(self.window)

    def need_to_close(self):
        need = glfw.window_should_close(self.window)
        # print(f"need {need}")
        return need

    def shutdown(self):
        self.impl.shutdown()
        glfw.terminate()

    def update_imgui(self):
        imgui.begin("test setting")
        # print("add image")
        # new_image = np.random.rand(100, 100)
        # imgui.image(new_image, 100, 100)
        # texture_id = imgui.get_io().fonts.texture_id
        # draw_list = imgui.get_window_draw_list()
        # draw_list.add_image(texture_id, (20, 35), (180, 80), col=imgui.get_color_u32_rgba(0.5,0.5,1,1))
        # draw_list = imgui.get_window_draw_list()
        # draw_list.add_circle_filled(100, 60, 30, imgui.get_color_u32_rgba(1,1,0,1))
        # imgui.text("bar")
        imgui.end()
        # if imgui.begin_main_menu_bar():
        #     if imgui.begin_menu("File", True):

        #         clicked_quit, selected_quit = imgui.menu_item(
        #             "Quit", 'Cmd+Q', False, True)

        #         if clicked_quit:
        #             exit(1)

        #         imgui.end_menu()
        #     imgui.end_main_menu_bar()

        # imgui.begin("Custom window", True)
        # imgui.text("Bar")
        # imgui.text_ansi("B\033[31marA\033[mnsi ")
        # imgui.text_ansi_colored("Eg\033[31mgAn\033[msi ", 0.2, 1., 0.)
        # imgui.extra.text_ansi_colored("Eggs", 0.2, 1., 0.)
        # imgui.end()


def main():
    render = cRender()
    while not render.need_to_close():
        render.pre_update()
        render.update_imgui()
        render.post_update()

    render.shutdown()


if __name__ == "__main__":
    main()