import numpy as np
from PIL import Image


img = Image.open('map.jpg', 'r')
pixels = img.load()
width, height = img.size

pixel_values = list(img.getdata())
pixel_values = np.array(pixel_values).reshape((width, height, 3))

max_layers = 5
pos = []

min_x = -1200; max_x = 300
min_y = 160; max_y = 180
min_z = -1500; max_z = 1500

tx = 3000
ty = 0
tz = 0
sx = 1
sy = 1
sz = 1

transform = np.array([[sx, 0,  0,  tx],
                      [0,  sy, 0,  ty],
                      [0,  0,  sz, tz],
                      [0,  0,  0,  1]])

for x in range(height):
    for z in range(width):
        for y in range((max_layers * pixel_values[x][z][0]) // 255):
            pos.append(transform @ np.array([x, max_layers - y, z, 1]))

print(pos)

all_pos = open("final_pos.txt", 'w')
for position in pos:
    all_pos.write(f"Vector3f({position[0]},{position[1]},{position[2]})")
    all_pos.write(",")
all_pos.close()