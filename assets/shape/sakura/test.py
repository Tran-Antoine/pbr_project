def remove_lines_starting_with_vn(file_path):
    # Read all lines from the file
    with open(file_path, "r") as file:
        lines = file.readlines()
    
    # Filter lines that do not start with 'vn'
    filtered_lines = [line for line in lines if not line.strip().startswith('vn')]
    
    # Write the filtered lines back to the file
    with open(file_path, "w") as file:
        file.writelines(filtered_lines)

# Replace 'path/to/your/file.txt' with the actual file path
file_path = "Sakura.obj"
remove_lines_starting_with_vn(file_path)
