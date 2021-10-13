import sys
import glfw
import os
import cv2
import os.path as osp
import imgui
import numpy as np

sys.path.append("../render")
from render import cRender


class cRenderResource:
    def __init__(self):
        self.img = None
        self.path = None
        self.height = None
        self.width = None
        self.channels = None

    def load_png(self, path):
        assert osp.exists(path), path
        self.img = cv2.imread(path) / 255
        self.img = self.img[::-1, :, ::-1]
        self.path = path
        self.height = self.img.shape[0]
        self.width = self.img.shape[1]
        self.channels = self.img.shape[2]


def layout_algorithm(resource_lst: list):
    pos_lst = []
    widnow_height = 0
    window_width = 0
    for res in resource_lst:
        cur_height = res.height
        cur_width = res.width
        assert res.channels == 3

        pos_lst.append((window_width, 0))
        widnow_height = max(widnow_height, cur_height)
        window_width += cur_width
    assert len(pos_lst) == len(resource_lst)
    return pos_lst, widnow_height, window_width


import OpenGL.GL as gl


class Step1Render(cRender):
    def __init__(self, window_name="", window_size=(800, 600)):
        super().__init__(window_name, window_size)
        self.render_resource_lst = []
        self.texture = None
        self.fbo = None

    def add_render_resource(self, res: cRenderResource):
        assert res.img is not None
        self.render_resource_lst.append(res)
        self.pos_lst, desired_window_height, desired_window_width = layout_algorithm(
            self.render_resource_lst)
        # # print(f"pos_lst {pos_lst}")
        # print(f"desired_window_height {desired_window_height}")
        # print(f"desired_window_width {desired_window_width}")
        if True == self.need_to_resize(desired_window_height,
                                       desired_window_width):
            # print(f"need to resize now")
            self.resize(desired_window_height, desired_window_width)
        # exit(1)

    def resize(self, height, width):
        
        cRender.resize(self, height, width)

        # we need to delete the old texture
        # print(f"[log] begin to delete texture {self.texture}")
        tex_array = (gl.GLuint * 1)(self.texture)
        gl.glDeleteTextures(1, tex_array)
        # print(f"[log] begin to delete fbo")
        fbo_array = (gl.GLuint * 1)(self.fbo)
        gl.glDeleteFramebuffers(1, fbo_array)

        self.init_texture_and_fbo()
        # delete the frame buffers
        # init the texture and FBO

    def create_texture(self, width, height):
        # # print(f"[log] create texture")
        self.texture = gl.glGenTextures(1)
        gl.glBindTexture(gl.GL_TEXTURE_2D, self.texture)
        gl.glTexParameteri(gl.GL_TEXTURE_2D, gl.GL_TEXTURE_MIN_FILTER,
                           gl.GL_NEAREST)
        gl.glTexParameteri(gl.GL_TEXTURE_2D, gl.GL_TEXTURE_MAG_FILTER,
                           gl.GL_LINEAR)

        gl.glTexImage2D(gl.GL_TEXTURE_2D, 0, gl.GL_RGB, width, height, 0,
                        gl.GL_RGB, gl.GL_FLOAT,
                        np.ones(height * width * 3, dtype=np.float32) * 0)
        gl.glGenerateMipmap(gl.GL_TEXTURE_2D)

    def create_fbo_from_texture(self):
        self.fbo = gl.glGenFramebuffers(1)
        gl.glBindFramebuffer(gl.GL_READ_FRAMEBUFFER, self.fbo)

        #   gl.GL_COLOR_ATTACHMENT0,
        #                      gl.GL_TEXTURE_2D, self.texture, 0)
        gl.glFramebufferTexture2D(gl.GL_READ_FRAMEBUFFER,
                                  gl.GL_COLOR_ATTACHMENT0, gl.GL_TEXTURE_2D,
                                  self.texture, 0)
        gl.glBindFramebuffer(gl.GL_READ_FRAMEBUFFER, 0)

    def init_texture_and_fbo(self):
        # print(f"--------begin to init tex and fbo-------")
        cur_width, cur_height = glfw.get_window_size(self.window)
        # print(f"init height {cur_height}, init width {cur_width}")
        self.create_texture(cur_width, cur_height)
        self.create_fbo_from_texture()
        # print(f"--------end to init tex and fbo-------")

    def init(self):
        self.init_texture_and_fbo()

    def draw_texture(self):
        gl.glBindTexture(gl.GL_TEXTURE_2D, self.texture)
        for idx in range(len(self.render_resource_lst)):
            resource = self.render_resource_lst[idx]
            cur_st = self.pos_lst[idx]
            st_height, st_width = cur_st[0], cur_st[1]
            img_height, img_width, channels = resource.img.shape
            data = resource.img.reshape(-1)
            # # print(st_width, st_height)
            # # print(img_width, img_height)
            # # print(data.size)
            # exit()
            gl.glTexSubImage2D(gl.GL_TEXTURE_2D, 0, st_width, st_height,
                               img_width, img_height, gl.GL_RGB, gl.GL_FLOAT,
                               data)

    def update_fbo(self):
        gl.glBindFramebuffer(gl.GL_READ_FRAMEBUFFER, self.fbo)
        width, height = glfw.get_window_size(self.window)
        gl.glBlitFramebuffer(0, 0, width, height, 0, 0, width, height,
                             gl.GL_COLOR_BUFFER_BIT, gl.GL_LINEAR)

    def update(self):
        # 1. draw my custom images
        self.draw_texture()
        self.update_fbo()
        # 2. update imgui
        self.update_imgui()

    def update_imgui(self):
        imgui.begin("step1 rendering")
        imgui.text("test text")
        imgui.end()


if __name__ == "__main__":

    res = cRenderResource()
    res.load_png("pikachu.png")
    window_name = "step1_bezier_to_gbsim"
    render = Step1Render(window_name)
    render.init()
    render.add_render_resource(res)

    while not render.need_to_close():
        render.pre_update()
        render.update()
        render.post_update()
    render.shutdown()