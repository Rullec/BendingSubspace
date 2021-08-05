import cv2
import glob
import numpy as np
from operator import itemgetter

files = [i for i in glob.glob("*png")]
files = list(itemgetter(* (np.random.permutation(len(files))[:12]))(files))
files = [cv2.imread(i) for i in files] 

img_row0 = cv2.hconcat(files[0:4])
img_row1 = cv2.hconcat(files[4:8])
img_row2 = cv2.hconcat(files[8:12])
img = cv2.vconcat([img_row0, img_row1, img_row2])
# print(img.shape)
# cv2.imshow(img)
cv2.imwrite("go.png", img)