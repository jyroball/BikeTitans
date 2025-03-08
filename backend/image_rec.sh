#!/bin/bash

# Install dependencies
python3 -m pip install --upgrade pip
pip3 install -r requirements.txt

# Download images from Google Drive
echo "------------------"
echo "Downloading Uploaded Images"
echo "------------------"
python3 input.py

# Define directories for images and labels
IMAGE_DIR="../images/gdrive_images"
LABEL_DIR="../images/label_folder"

# Go to the yolov5 directory for inference
echo "------------------"
echo "YOLOv5 Model Recognition"
echo "------------------"

rm -rf yolov5
git clone https://github.com/ultralytics/yolov5
cd yolov5
pip install -r requirements.txt

# Iterate over all .jpg images in the IMAGE_DIR
for img in "$IMAGE_DIR"/*.JPG; do
  # Run the YOLOv5 detection script on the image
  python3 detect.py --source "$img" --weights ../best.pt --img 640 --device cpu --save-txt --conf-thres 0.45

  # Define the label file path
  label_file="runs/detect/exp/labels/$(basename "$img" .jpg).txt"

  # If the label file exists, move it to the LABEL_DIR
  if [ -f "$label_file" ]; then
    mv "$label_file" "$LABEL_DIR/"
  fi

  # Clean up the temporary folder
  rm -rf runs/detect/exp
done

# Go back to the backend directory to run parser
cd ..

# Parse the images and labels with your custom parser
echo "------------------"
echo "Running Parser"
echo "------------------"
python3 parser.py

echo "------------------"
echo "Final Output"
echo "------------------"
cat ../frontend/static/output.json

# Set up git configuration
git config --global user.name "GitHub Actions"
git config --global user.email "github-actions@github.com"

# Add changes to frontend and push to remote repository
git add ../frontend
git commit -m "Update frontend based on parser.py changes"
git push
