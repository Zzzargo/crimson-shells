# This script pixelates an image to a given size using nearest neighbor scaling
from PIL import Image
import sys

if len(sys.argv) != 3:
    print(f"Usage: {sys.argv[0]} <input_image_path> <output_image_path>")
    sys.exit(1)
# 168 270
desiredPixelnessW = 168
desiredPixelnessH = 270
#224 360
outputW = 168
outputH = 270

image_path = sys.argv[1]
img = Image.open(image_path)

# Downscale to a smaller size to pixelate
img_downscaled = img.resize((desiredPixelnessW, desiredPixelnessH), Image.NEAREST)

# Upscale to a larger size to get the pixelated effect
img_resized = img_downscaled.resize((outputW, outputH), Image.NEAREST)

output_path = sys.argv[2]
img_resized.save(output_path)