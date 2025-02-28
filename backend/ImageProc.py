from yolov5 import YOLOv5
import cv2
import pandas as pd

# Image path (ensure the file exists at this location)
image_path = '/Users/josephcaraan/Downloads/biketrainingpics/bikeimg5.jpg'

# Load image using OpenCV
image = cv2.imread(image_path)

# Check if the image was loaded properly
if image is None:
    print(f"Error: Unable to load image at {image_path}")
    exit(1)

# Load a pretrained YOLO model
model = YOLOv5("yolov5s.pt")  # You can change this to "yolov5x.pt" for a larger model if needed

# Run the model on the image
results = model.predict(image)  # Use the correct method for inference

# Show results (optional)
results.show()

# Extract the predictions as a pandas DataFrame
predictions = results.pandas().xywh[0]  # Index 0 for the first (only) image

# Check the first few predictions to verify class and index
print(predictions.head())  # This will show the predictions and class names

# Assuming 'bicycle' corresponds to class index 2 in the default COCO dataset
bike_class_name = 'bicycle'

# Filter predictions by the class name
bikes = predictions[predictions['name'] == bike_class_name]

# Count how many bicycles are detected
bike_count = bikes.shape[0]

print(f"Number of bikes detected: {bike_count}")

