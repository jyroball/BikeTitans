import base64
from credential import PRIVATE_KEY

# Load your PEM private key
pem_private_key = PRIVATE_KEY

# Remove header, footer, and newlines
pem_cleaned = pem_private_key.replace("-----BEGIN PRIVATE KEY-----", "").replace("-----END PRIVATE KEY-----", "").replace("\n", "")

# Encode as Base64 (ensure URL-safe format)
base64_private_key = base64.b64encode(pem_cleaned.encode()).decode()

print(base64_private_key)
