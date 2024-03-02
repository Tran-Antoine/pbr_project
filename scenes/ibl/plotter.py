# Now that the file has been created, let's proceed with loading the data and plotting the points.
import matplotlib.pyplot as plt

# Re-initialize lists to hold the x and y coordinates
x_coords = []
y_coords = []

file_path = "data.txt"

# Read the data from the newly created file
with open(file_path, 'r') as file:
    t = 0
    for line in file:
        if t > 500: break
        t += 1
        x, y = line.split()  # Split each line into x and y coordinates
        x_coords.append(float(x))  # Convert to float and append to the list
        y_coords.append(float(y))

# Plotting the cloud of points
plt.scatter(x_coords, y_coords)
plt.xlabel('X Coordinate')
plt.ylabel('Y Coordinate')
plt.title('Cloud of Points from data.txt')
plt.grid(True)
plt.show()
