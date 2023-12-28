def print_file_contents(file_path):
    try:
        with open(file_path, 'r') as file:
            for line in file:
                print(line, end='')  # Print each line in the file
    except IOError as e:
        print(f"Error opening file: {e}")

# Example usage
file_path = './database/user_001/contacts.txt'  # Replace with the path to your file
print_file_contents(file_path)
print()
file_path = './database/user_001/messages.txt'  # Replace with the path to your file
print_file_contents(file_path)