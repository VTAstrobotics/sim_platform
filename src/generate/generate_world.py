import numpy as np
from PIL import Image
from noise import pnoise2

width = 1025
height = 1025
scale = 200.0     # Larger = wider dunes
octaves = 4
persistence = 0.5
lacunarity = 2.0

img = np.zeros((height, width))

for y in range(height):
    for x in range(width):
        img[y][x] = pnoise2(
            x / scale,
            y / scale,
            octaves=octaves,
            persistence=persistence,
            lacunarity=lacunarity,
            repeatx=width,
            repeaty=height,
            base=0
        )

# Normalize to 0–255
img = (img - img.min()) / (img.max() - img.min())
img = (img * 255).astype(np.uint8)

Image.fromarray(img).save("sand_dunes.png")
