import mitsuba as mi
import matplotlib.pyplot as plt

mi.set_variant("scalar_rgb")
scene = mi.load_file("references/disney_cloud/cloud.xml")
image = mi.render(scene, spp=64)

plt.axis("off")
plt.imshow(image ** (1.0 / 2.2))
plt.show()
