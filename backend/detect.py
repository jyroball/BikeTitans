import os
import re

numBikes = 0
totSlots = 20
freeSlots = totSlots

def read_text_file(file_path):
    global numBikes
    try:
        with open(file_path, 'r', encoding='utf-8') as file:
            content = file.readlines()
            numBikes = len(content)
            # print(f"Number of lines: {numBikes}")
    except FileNotFoundError:
        print(f"Error: The file '{file_path}' was not found.")
    except Exception as e:
        print(f"An error occurred: {e}")

def find_jpg_file(directory):
    for file in os.listdir(directory):
        if file.lower().endswith(".jpg"):
            #print(f"Found JPG file: {file}")
            return file
    print("No JPG file found.")
    return None

def extract_number_from_filename(filename):
    match = re.search(r"([^_]+)_(\d+)\.JPG", filename)
    if match:
        part_before_underscore = match.group(1)  
        number_after_underscore = match.group(2)
        try:
            return int(number_after_underscore)
        except ValueError:
            print(f"Error: '{number_after_underscore}' is not a valid number")
            return None
    else:
        print(f"Filename '{filename}' doesn't match the expected pattern.")
        return None

if __name__ == "__main__":
    file_path = "detect.txt"
    read_text_file(file_path)

    # extract total number of slots from image name
    jpg_file = find_jpg_file(".")
    if jpg_file:
        extracted_number = extract_number_from_filename(jpg_file)
        # print(f"Match: {extracted_number}")
        if extracted_number is not None:
            totSlots = extracted_number
            # print(f"Updated number of free slots based on image file: {freeSlots}")

    #get free slots
    freeSlots = totSlots - numBikes
    print(f"Number of free slots: {freeSlots}")
