import sys

sys.path.append("../render")
import os
from render import cRender


class c3dRender(cRender):
    def __init__(self, window_name, window_size):
        super().__init__(window_name, window_size)

    

# do 3d rendering in pyopengl
# 1. set up the basic renderer (shading)
# 2. set up the arcball / ortho camera
# 3. set up the mouse event
# 4. set up the VAO & VBO
# 5. draw

if __name__ == "__main__":
    print("hello world")