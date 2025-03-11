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
echo "------------------"
echo "Create Folders"
echo "------------------"
CURRENT_DIR=$(pwd)

IMAGE_DIR="$CURRENT_DIR/images/gdrive_images"
LABEL_DIR="$CURRENT_DIR/images/label_folder"

mkdir -p "$IMAGE_DIR"
mkdir -p "$LABEL_DIR"

echo "Image directory: $IMAGE_DIR"
echo "Label directory: $LABEL_DIR"

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
  img_filename=$(basename "$img")
  
  python3 detect.py --source "$img" --weights ../best.pt --img 640 --device cpu --save-txt --conf-thres 0.45

  label_filename="${img_filename%.JPG}.txt"
  label_file="runs/detect/exp/labels/$label_filename"
  
  if [ -f "$label_file" ]; then
    echo "Moving $label_file to $LABEL_DIR/"
    mv "$label_file" "$LABEL_DIR/"
  else
    echo "Label file not found: $label_file"
    echo "Files in labels directory:"
    ls -la runs/detect/exp/labels/
  fi

  # delete temporary folder
  rm -rf runs/detect/exp
done

# Go back to the backend directory to run parser
cd ..

# Parser
echo "------------------"
echo "Running Parser"
echo "------------------"
python3 parser.py

echo "------------------"
echo "Final Output"
echo "------------------"
cat ../frontend/static/output.json

# Commit all changes (if any)
if [[ -n $(git status --porcelain ../frontend) ]]; then
  echo "Changes detected. Committing..."
  
  git config --global user.name "GitHub Actions"
  git config --global user.email "github-actions@github.com"
  
  git add ../frontend
  git commit -m "Update frontend based on parser.py changes"
  git push
else
  echo "No changes detected. Skipping commit."
fi
