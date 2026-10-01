# Safai Setu: Kachra Se Kamaal

Folder structure:

```
safai-setu/
├── index.html            <- the whole web app (open this)
├── esp32_smartbin.ino    <- optional: real IoT sensor code
└── README.md             <- this guide
```

## 1. Run it (no setup)

Open `index.html` in VS Code, right-click, **Open with Live Server** (or double-click the file).
Without Firebase keys it runs in **offline demo mode** using browser storage. Everything works.

Demo logins:
- Admin: `admin@safai.in` / `admin123`
- Citizen: `citizen@safai.in` / `city123`

## 2. Turn on the cloud (Firebase), about 10 minutes

1. Go to https://console.firebase.google.com and click **Add project**.
2. In the project, click the **</>** (Web) icon, register an app, and copy the `firebaseConfig` values.
3. Open `index.html`, find `FIREBASE_CONFIG` near the top of the script, and paste your values.
4. **Build > Authentication > Get started > Email/Password > Enable**.
5. **Build > Firestore Database > Create database** (start in test mode).
6. Open the **Rules** tab, paste this, and **Publish**:

```
rules_version = '2';
service cloud.firestore {
  match /databases/{database}/documents {
    match /complaints/{id} { allow read: if true; allow write: if request.auth != null; }
    match /users/{uid}     { allow read, write: if request.auth != null && request.auth.uid == uid; }
    match /bins/{id}       { allow read, write: if true; }
  }
}
```

Reload the page. The login screen should say **Cloud sync is ON**.
The first load fills Firestore with 30 sample complaints. Open the app on two devices and change a status on one: the other updates live.

The demo buttons create the demo accounts in Firebase on first tap.
Typing `SAFAI2026` in the "Admin code" box at registration makes an admin (hackathon shortcut; change `ADMIN_CODE` in the file).
The `bins` rule is open so a sensor can write without logging in. Tighten it after the hackathon.

## 3. IoT smart bins

The **Smart bins** page works right away with simulated sensors (8 bins filling up, auto-complaint at 90%, AI forecast of when each bin will be full).

To go live with real hardware:
1. Needs: ESP32, HC-SR04 ultrasonic sensor, jumper wires.
2. Open `esp32_smartbin.ino` in Arduino IDE, fill in WiFi, `PROJECT_ID`, `API_KEY`, bin name and location, then upload.
3. Mount the sensor under the bin lid facing down. The page switches to **Live data from ESP32 sensors** the moment a `bins` document appears.

No hardware? Fake a sensor from any terminal (replace the two values):

```
curl -X PATCH "https://firestore.googleapis.com/v1/projects/YOUR_PROJECT_ID/databases/(default)/documents/bins/BIN-LIVE?key=YOUR_API_KEY&updateMask.fieldPaths=name&updateMask.fieldPaths=lat&updateMask.fieldPaths=lng&updateMask.fieldPaths=fill" -H "Content-Type: application/json" -d "{\"fields\":{\"name\":{\"stringValue\":\"Demo Bin\"},\"lat\":{\"doubleValue\":28.62},\"lng\":{\"doubleValue\":77.21},\"fill\":{\"doubleValue\":93}}}"
```

Note: the admin must have the app open for the auto-complaint to be created in cloud mode.

## 4. AI features

- **AI waste scanner:** TensorFlow.js MobileNet runs in the browser. It is a general model, so it is best on common items (bottles, fruit, phones). Say honestly that a custom-trained waste model is the next step.
- **AI triage and forecast:** priority scores in the Control room and "full in ~X h" on every smart bin. These are rule-based and statistical, not deep learning.

## 5. Checklist before the demo

- Test on the venue wifi. Map tiles, fonts, Firebase and the AI model need internet.
- Open the AI scanner once beforehand so the model is cached.
- Change `TEAM` and `CENTER` at the top of the script.
