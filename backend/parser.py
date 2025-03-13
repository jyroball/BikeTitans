import json
import os
import re

# Hardcoded folder names
image_folder = "images/gdrive_images"
txt_folder = "images/label_folder"
output_folder = "../frontend/static"


# Function to read the text file and return the number of bikes
def read_text_file(file_path):
    try:
        with open(file_path, "r", encoding="utf-8") as file:
            content = file.readlines()
            return len(content)  # Number of bikes is the number of lines
    except FileNotFoundError:
        print(f"Error: The file '{file_path}' was not found.")
        return 0
    except Exception as e:
        print(f"An error occurred: {e}")
        return 0


# Function to find the JPG file corresponding to a .txt file
def find_jpg_file(txt_file):
    filename_without_extension = os.path.splitext(txt_file)[0]
    jpg_file = f"{filename_without_extension}.jpg"
    jpg_file_upper = f"{filename_without_extension}.JPG" 

    # Log the files being checked
    print(f"Looking for: {jpg_file} or {jpg_file_upper}")

    if os.path.exists(os.path.join(image_folder, jpg_file)):
        return jpg_file
    elif os.path.exists(os.path.join(image_folder, jpg_file_upper)):
        return jpg_file_upper
    else:
        print(f"JPG file '{jpg_file}' or '{jpg_file_upper}' not found.")
        return None


# Function to extract the total number of slots from the image filename
def extract_number_from_filename(filename):
    match = re.search(r"(.+?)_(\d+)\.(jpg|JPG)$", filename)
    if match:
        part_before_underscore = match.group(1).strip()  # Title (before '_')
        number_after_underscore = match.group(2)  # Total slots number (after '_')
        try:
            return part_before_underscore, int(number_after_underscore)
        except ValueError:
            print(f"Error: '{number_after_underscore}' is not a valid number")
            return None, None
    else:
        print(f"Filename '{filename}' doesn't match the expected pattern.")
        return None, None


# Function to create the output JSON file
def create_output_json(data, output_file):
    try:
        with open(output_file, "w", encoding="utf-8") as json_file:
            json.dump(data, json_file, indent=4)
            print(f"Output written to {output_file}")
    except Exception as e:
        print(f"An error occurred while writing the JSON file: {e}")


if __name__ == "__main__":
    output_data = {}

    # Loop through all .txt files in the txt_folder
    for txt_file in os.listdir(txt_folder):
        if txt_file.endswith(".txt"):
            # Find corresponding JPG file
            jpg_file = find_jpg_file(txt_file)
            if jpg_file:
                # Extract title and total slots from image filename
                title, total_slots = extract_number_from_filename(jpg_file)
                if title and total_slots is not None:
                    # Get number of bikes from the text file
                    file_path = os.path.join(txt_folder, txt_file)
                    num_bikes = read_text_file(file_path)

                    # Calculate free slots
                    free_slots = total_slots - num_bikes

                    # Add the data to the output dictionary
                    output_data[title] = {
                        "total_slot_number": total_slots,
                        "open_slots": free_slots,
                    }

    # Create the output folder if it doesn't exist
    if not os.path.exists(output_folder):
        os.makedirs(output_folder)

    # Define the output file path
    output_file = os.path.join(output_folder, "output.json")

    # Write the output data to JSON file
    create_output_json(output_data, output_file)
