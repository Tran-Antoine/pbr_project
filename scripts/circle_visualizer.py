import matplotlib.pyplot as plt
import numpy as np

# Define radii
R = 5
N = 10
r = R / (0.5 * N)

# Create figure and axis
fig, ax = plt.subplots()

# Draw the large circle
large_circle = plt.Circle((0, 0), R, fill=False, edgecolor='blue', linewidth=2)
ax.add_artist(large_circle)

# Function to generate random points inside a circle
def generate_random_points(radius, n_points):
    points = []
    while len(points) < n_points:
        x, y = (np.random.uniform(-radius, radius), np.random.uniform(-radius, radius))
        if np.sqrt(x**2 + y**2) + r <= radius:  # Ensure the small circle stays inside the large circle
            points.append((x, y))
    return points

# Generate points for small circles
points = generate_random_points(R, N)

# Draw the small circles
for (x, y) in points:
    small_circle = plt.Circle((x, y), r, fill=True, edgecolor='red', facecolor='lightcoral', linewidth=1)
    ax.add_artist(small_circle)

# Set the aspect of the plot to be equal
ax.set_aspect('equal')

# Set the limits of the plot
ax.set_xlim(-R-1, R+1)
ax.set_ylim(-R-1, R+1)

# Hide axes
ax.axis('off')

# Show plot
plt.show()