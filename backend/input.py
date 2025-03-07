import io
import os
import json

from google.oauth2 import service_account
from googleapiclient.discovery import build
from googleapiclient.http import MediaIoBaseDownload

# Constants
FOLDER_ID = "1tYyTa-e2eDs4cfoYDbhEDbZuCiJybdJc"
DOWNLOAD_DIR = "images/gdrive_images"

# Load credentials from environment variable
service_account_info = json.loads(os.getenv("GDRIVE_CRED"))
creds = service_account.Credentials.from_service_account_info(service_account_info)

# Build Drive service
drive_service = build("drive", "v3", credentials=creds)

# Ensure the download directory exists
os.makedirs(DOWNLOAD_DIR, exist_ok=True)


def list_files(folder_id):
    """Lists all files in the specified Google Drive folder."""
    query = f"'{folder_id}' in parents and trashed=false"
    results = drive_service.files().list(q=query, fields="files(id, name)").execute()
    return results.get("files", [])


def download_file(file_id, file_name):
    """Downloads a file from Google Drive."""
    request = drive_service.files().get_media(fileId=file_id)
    file_path = os.path.join(DOWNLOAD_DIR, file_name)

    with open(file_path, "wb") as f:
        downloader = MediaIoBaseDownload(f, request)
        done = False
        while not done:
            _, done = downloader.next_chunk()

    print(f"Downloaded: {file_name}")


def main():
    files = list_files(FOLDER_ID)
    if not files:
        print("No files found in the folder.")
        return

    for file in files:
        download_file(file["id"], file["name"])


if __name__ == "__main__":
    main()
