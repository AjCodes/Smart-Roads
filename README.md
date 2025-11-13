# 🚦 Smart Roads – Traffic Optimization System junior

A smart traffic-management prototype using **Arduino**, **Firebase**, **Node.js**, and **React**.  
The system detects traffic density, analyzes it with simple AI logic, and automatically controls traffic lights.

---

## 📁 Project Structure

```
Smart-Roads/
│
├── arduino/      → Sensor reading + LED control (Ethan)
│
├── backend/      → API + AI logic + Firebase connection (AJ + Junior)
│
└── dashboard/    → Web dashboard for live data (Julia + Elias)
```

---

## 🚀 Get Started (Team Instructions)

### 1. Clone the project
```
git clone <repo-link>
```

### 2. Open the main folder in VS Code  
You will see:
```
arduino/
backend/
dashboard/
```

### 3. Go to **your** folder  
- **Ethan:** `arduino/`  
- **AJ + Junior:** `backend/`  
- **Julia + Elias:** `dashboard/`

### 4. Install dependencies (only for backend or dashboard)

**Backend**
```
cd backend
npm install
```

**Dashboard**
```
cd dashboard
npm install
```

### 5. Start working  
Only edit files inside your assigned folder.

### 6. Push your work  
```
git add .
git commit -m "update"
git push
```

### 7. Always pull before starting  
```
git pull
```

---

## 🔧 Tech Stack

### **Hardware**
- Arduino / ESP32  
- Ultrasonic sensors  
- LEDs  

### **Backend**
- Node.js  
- Express  
- Firebase Admin SDK  

### **Database**
- Firebase Realtime Database  

### **Dashboard**
- React  
- Firebase Web SDK  
- Chart.js  

---

## 🧠 System Flow (Simple)

1. Arduino reads traffic distance  
2. Sends data to Firebase Realtime Database  
3. Backend listens, analyzes, and picks the best lane  
4. Backend writes decision back into Firebase  
5. Arduino reads decision and switches LEDs  
6. Dashboard displays real-time data and graphs  
