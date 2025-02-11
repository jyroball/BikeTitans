import base64
import json
import time

import requests

# Hardcoded Base64 credentials
from credential import CREDENTIALS_BASE64
from cryptography.hazmat.primitives import serialization
from cryptography.hazmat.primitives.asymmetric import padding
from cryptography.hazmat.primitives.hashes import SHA256

# Decode credentials.json
credentials_json = base64.b64decode(CREDENTIALS_BASE64).decode()
credentials = json.loads(credentials_json)

# Extract required fields
PRIVATE_KEY_B64 = credentials["private_key"].replace("\\n", "\n")
CLIENT_EMAIL = credentials["client_email"]
TOKEN_URI = "https://oauth2.googleapis.com/token"

FOLDER_ID = "1tYyTa-e2eDs4cfoYDbhEDbZuCiJybdJc"

# Load RSA Private Key
private_key = serialization.load_pem_private_key(
    PRIVATE_KEY_B64.encode(), password=None
)


def create_jwt():
    """Generates a signed JWT using RSA-PKCS1-v1_5 and SHA-256."""
    header = {"alg": "RS256", "typ": "JWT"}
    payload = {
        "iss": CLIENT_EMAIL,
        "scope": "https://www.googleapis.com/auth/drive.file",
        "aud": TOKEN_URI,
        "exp": int(time.time()) + 3600,
        "iat": int(time.time()),
    }

    # Base64 encode header & payload
    encoded_header = (
        base64.urlsafe_b64encode(json.dumps(header).encode()).decode().rstrip("=")
    )
    encoded_payload = (
        base64.urlsafe_b64encode(json.dumps(payload).encode()).decode().rstrip("=")
    )

    # Concatenate to form message
    message = f"{encoded_header}.{encoded_payload}".encode()

    # Sign using RSA-PKCS1-v1_5 + SHA-256
    signature = private_key.sign(message, padding.PKCS1v15(), SHA256())
    encoded_signature = base64.urlsafe_b64encode(signature).decode().rstrip("=")

    return f"{encoded_header}.{encoded_payload}.{encoded_signature}"


def get_access_token():
    """Exchanges JWT for an access token."""
    jwt_token = create_jwt()
    response = requests.post(
        TOKEN_URI,
        data={
            "grant_type": "urn:ietf:params:oauth:grant-type:jwt-bearer",
            "assertion": jwt_token,
        },
        headers={"Content-Type": "application/x-www-form-urlencoded"},
    )

    data = response.json()
    if "access_token" not in data:
        raise Exception(f"Failed to get access token: {data}")

    return data["access_token"]


def get_file_id(file_name, access_token):
    """Finds the file ID in Google Drive by name."""
    query = f"name='{file_name}' and '{FOLDER_ID}' in parents and trashed=false"
    search_url = f"https://www.googleapis.com/drive/v3/files?q={query}&fields=files(id)"

    response = requests.get(
        search_url,
        headers={"Authorization": f"Bearer {access_token}"},
    )

    files = response.json().get("files", [])
    return files[0]["id"] if files else None


def upload_file(file_path, file_name):
    """Uploads or overwrites a file in Google Drive."""
    access_token = get_access_token()
    file_id = get_file_id(file_name, access_token)

    if file_id:
        metadata = {
            "name": file_name,
        }

        files = {
            "metadata": ("metadata.json", json.dumps(metadata), "application/json"),
            "file": (file_name, open(file_path, "rb"), "application/octet-stream"),
        }

        # If file exists, overwrite with PATCH
        upload_url = f"https://www.googleapis.com/upload/drive/v3/files/{file_id}?uploadType=multipart"
        response = requests.patch(
            upload_url,
            headers={"Authorization": f"Bearer {access_token}"},
            files=files,
        )
    else:
        metadata = {
            "name": file_name,
            "parents": [FOLDER_ID],  # Folder ID
        }

        files = {
            "metadata": ("metadata.json", json.dumps(metadata), "application/json"),
            "file": (file_name, open(file_path, "rb"), "application/octet-stream"),
        }

        # If file does not exist, create a new one with POST
        upload_url = (
            "https://www.googleapis.com/upload/drive/v3/files?uploadType=multipart"
        )
        response = requests.post(
            upload_url,
            headers={"Authorization": f"Bearer {access_token}"},
            files=files,
        )

    print(response.json())


# TEST UPLOAD

upload_file("test_image.jpg", "test_image.jpg")
# upload_file("test_image_2.jpeg", "test_image.jpg")
