from PIL import Image
import numpy as np

def process_pixels(image_path, x1, x2, y1, y2):

    img = np.array(Image.open(image_path))
    
    for x in range(x1, x2):
        for y in range(y1, y2):
            pixel = img[y][x]
            if(pixel[0] > 100):
                img[y][x] = pixel / (1+0.02*(pixel[0] - 100)**0.7);
    
    # Save the modified image
    modified_image_path = image_path.replace('.png', 'modified.png')
    Image.fromarray(img).save(modified_image_path)
    
    return modified_image_path

# Set the intervals for the operation
x1, x2, y1, y2 = 1200, 1450, 350, 550

# Call the function with the image path and intervals
modified_image_path = process_pixels('valleymap.png', x1, x2, y1, y2)
modified_image_path
