#!/bin/bash


if command -v python3 &>/dev/null; then
    PYTHON_CMD=python3
elif command -v python &>/dev/null; then
    PYTHON_CMD=python
else
    echo "Error: Python is not installed."
    exit 1
fi


$PYTHON_CMD -m venv ENV


if [[ "$OSTYPE" == "msys" || "$OSTYPE" == "cygwin" ]]; then

    source ENV/Scripts/activate
else

    source ENV/bin/activate
fi

echo "Installing dependencies..."

pip install --prefer-binary -r requirements.txt

echo "Setup complete. Virtual environment is activated."
