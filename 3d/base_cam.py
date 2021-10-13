from enum import Enum
import numpy as np
import abc


class eCamType(Enum):
    ARCBALL_CAM = 1
    ORTHO_CAM = 2


def look_at(eye, center, up):
    assert len(eye) == 3
    assert len(center) == 3
    assert len(up) == 3

    f = (center - eye).normalized()
    u = up.normalized()
    s = f.cross(u).normalized()
    u = s.cross(f)
    mat = np.zeros([4, 4], dtype=np.float32)
    mat[0, 0] = s.x()
    mat[0, 1] = s.y()
    mat[0, 2] = s.z()
    mat[0, 3] = -s.dot(eye)
    mat[1, 0] = u.x()
    mat[1, 1] = u.y()
    mat[1, 2] = u.z()
    mat[1, 3] = -u.dot(eye)
    mat[2, 0] = -f.x()
    mat[2, 1] = -f.y()
    mat[2, 2] = -f.z()
    mat[2, 3] = f.dot(eye)
    mat[3, :] = np.array([0, 0, 0, 1])
    return mat


class cCameraBase(metaclass=abc.ABCMeta):
    def __init__(self, pos, center, up, fov, near_plane, far_plane, cam_type):
        self.mouse_acc = 0.1
        self.key_acc = 0.02
        self.last_x = 0
        self.last_y = 0
        self.first_mouse = True
        self.cam_type = cam_type

        self.pos = pos
        self.center = center
        self.up = up
        self.fov = fov
        self.near = near_plane
        self.far = far_plane

    def GetType(self):
        return self.cam_type

    def ViewMatrix(self):
        pass

    def ProjMatrix(self, screen_width, screen_height, is_vulkan=False):
        pass

    def GetCameraPos(self):
        pass

    def GetCameraCenter(self):
        pass

    def GetCameraFront(self):
        pass

    def GetCameraUp(self):
        pass

    def GetCameraFovDeg(self):
        pass

    def MoveForward(self):
        pass

    def MoveBackward(self):
        pass

    def MoveLeft(self):
        pass

    def MoveRight(self):
        pass

    def MoveUp(self):
        pass

    def MoveDown(self):
        pass

    def MouseMove(mouse_x, mouse_y):
        pass

    def SetXY(mouse_x, mouse_y):
        pass

    def ResetFlag(self):
        pass

    def SetKeyAcc(acc):
        pass

    def SetMouseAcc(acc):
        pass

    def IsFirstMouse(self):
        pass

    def CalcCursorPointWorldPos(xpos, ypos, height, width):
        pass

    def MouseRotate(self):
        pass


if __name__ == "__main__":
    cam_pos = np.array([1, 1, 1])
    cam_focus = np.array([0, 0, 0])
    cam_up = np.array([0, 1, 0])
    view = look_at(cam_pos, cam_focus, cam_up)
    print(view)