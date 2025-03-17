# Bike Titans

## Developer Environment

### Backend
```sh
cd backend
python -m venv ENV
pip install -r requirements.txt
```

### Hardware
```sh
cd hardware
```
* Install the PlatformIO VSCode Extension.
* Open the "hardware" folder in its own VSCode window. VSCode should automatically detect and initialize the project. 

### Frontend

```sh
cd frontend
npm install | bun install
npm run build | bun run build
npm start | bun start
```
* Pushing / merging to main will automatically trigger a new github pages deployment. 

## GitHub Actions

### main.yaml
* Downloads images from Google Drive.
* Runs model on those images.
* Formats output properly.
* Commits to `main` branch.
* Runs every 20 minutes from 8am-8pm, and every 1 hour from 8pm-8am.

### pages.yaml
* Redeploys website with new image data.
* Runs every time `main` branch is updated.
