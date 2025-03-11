# Bike Titans

## Setting up Developer Environment

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