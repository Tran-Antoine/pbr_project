import mitsuba as mi
mi.set_variant("scalar_rgb")

scene = mi.load_file("mi_scene.xml")
image = mi.render(scene, spp=256)
mi.util.write_bitmap("mi_result.png", image)
mi.util.write_bitmap("mi_result.exr", image)

import matplotlib.pyplot as plt
plt.axis('off')
plt.imshow(image ** (1.0 / 2.2));
plt.show()