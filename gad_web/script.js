// Import Firebase modules
import { initializeApp } from "https://www.gstatic.com/firebasejs/9.16.0/firebase-app.js";
import { getAuth, signInAnonymously, onAuthStateChanged } from "https://www.gstatic.com/firebasejs/9.16.0/firebase-auth.js";
import { getDatabase, ref, onValue } from "https://www.gstatic.com/firebasejs/9.16.0/firebase-database.js";
import { firebaseConfig } from './firebase-config.js';

// Initialize Firebase App
const app = initializeApp(firebaseConfig);
const auth = getAuth(app);
const database = getDatabase(app);

function agregarCero(numero) {
  return numero < 10 ? "0" + numero : String(numero);
}


// Function to retrieve and display the latest data from Firebase
function updateSensorData(data) {
  // Get the latest date and time keys
  console.log("Object.keys(data)", Object.keys(data));
  const latestDateKey = Object.keys(data).sort().pop();
  console.log("latestDateKey", latestDateKey);
  const latestTimeData = data[latestDateKey];
  console.log("latestTimeData", latestTimeData);
  const latestTimeKey = Object.keys(latestTimeData).sort().pop();
  const keysDate = Object.keys(latestTimeData)
    .map((key) => {
      const [day, month, year] = key.split("-");
      const date = new Date();
      date.setDate(day);
      date.setMonth(month-1);
      date.setFullYear(year);
      return date;
    })
    .sort((a, b) => b - a);
    console.log("keysDate", keysDate);
  const lastDate = keysDate[0]
  
  const lastKeyDate = `${agregarCero(lastDate.getDate())}-${
    agregarCero(lastDate.getMonth() + 1)
  }-${lastDate.getFullYear()}`;

  console.log("lastKeyDate", lastKeyDate);

  const lastKeySecondary = Object.keys(latestTimeData[lastKeyDate]).at(-1);
    const latestData = latestTimeData[lastKeyDate][lastKeySecondary];

  console.log("Latest data entry:", latestData);  // Log for debugging

  // Update tank status based on "flechita" value
  if (latestData && latestData.deposito) {
     const flechitaStatus = latestData.deposito.flechita || "Estable";  // Default to "Estable" if undefined

     let tankStatusMessage;
     if (flechitaStatus === "Arriba") {
        tankStatusMessage = "Cargando";
     } else if (flechitaStatus === "Abajo") {
        tankStatusMessage = "Descargando";
     } else {
        tankStatusMessage = "Estable";
     }

     // Update tank status
     const tankIndicator = document.getElementById('tankIndicator');
     if (tankIndicator) tankIndicator.textContent = tankStatusMessage;

     // Update other tank data
     const volumenTotal = document.getElementById('volumenTotal');
     if (volumenTotal) volumenTotal.textContent = `Volumen Total: ${parseFloat(latestData.deposito.volumenTotal || 0).toFixed(2)} m³`;

     const volumenAdministrable = document.getElementById('volumenAdministrable');
     if (volumenAdministrable) volumenAdministrable.textContent = `Volumen Administrable: ${parseFloat(latestData.deposito.volumenAdministrable || 0).toFixed(2)} m³`;

     const volumenAguaDisponible = document.getElementById('volumenAguaDisponible');
     if (volumenAguaDisponible) volumenAguaDisponible.textContent = `Volumen Disponible: ${parseFloat(latestData.deposito.volumenAguaDisponible || 0).toFixed(2)} m³`;

     const porcentajeVolumenTotal = document.getElementById('porcentajeVolumenTotal');
     if (porcentajeVolumenTotal) porcentajeVolumenTotal.textContent = `${parseFloat(latestData.deposito.porcentajeVolumenTotal || 0).toFixed(2)}%`;

     const consumoDiarioPromedio = document.getElementById('consumoDiarioPromedio');
     if (consumoDiarioPromedio) consumoDiarioPromedio.textContent = `Consumo Diario Promedio: ${parseFloat(latestData.deposito.consumoDiarioPromedio || 0).toFixed(2)} L/día`;
  } else {
     console.warn('Data structure for "deposito" is missing or undefined.');
  }

  // Update system information if elements exist and data is present
  if (latestData && latestData.sistema) {
     const tipoTanque = document.getElementById('tipoTanque');
     if (tipoTanque) tipoTanque.textContent = `Tipo de Tanque: ${latestData.sistema.tipoTanque || 'N/A'}`;

     const configuracion = document.getElementById('configuracion');
     if (configuracion) configuracion.textContent = `Configuración: ${latestData.sistema.configuracion || 'N/A'}`;

     const localizacion = document.getElementById('localizacion');
     if (localizacion) localizacion.textContent = `Localización: ${latestData.sistema.localizacion || 'N/A'}`;

     const timestamp = document.getElementById('timestamp');
     if (timestamp) timestamp.textContent = `Fecha de Instalación: ${latestData.sistema.fechaON || 'N/A'}`;
  } else {
     console.warn('Data structure for "sistema" is missing or undefined.');
  }

  // Update timestamp if present
  if (latestData && latestData.timestamp) {
     const timestampLegible = document.getElementById('timestampLegible');
     if (timestampLegible) timestampLegible.textContent = `Última Lectura: ${latestData.timestamp.legible || 'N/A'}`;
  } else {
     console.warn('Data structure for "timestamp" is missing or undefined.');
  }
}



// Listen for authentication state changes
onAuthStateChanged(auth, (user) => {
   if (user) {
      console.log("User signed in anonymously:", user.uid);

      // Reference to the 'mediciones' path in Firebase
      const dbRef = ref(database, '/mediciones');
      onValue(dbRef, (snapshot) => {
         const data = snapshot.val();
         console.log('Firebase data:', data);  // Log the full data structure for debugging
         updateSensorData(data);
      });

   } else {
      console.log("User is signed out.");
   }
});

// Sign in anonymously to Firebase
signInAnonymously(auth).catch((error) => {
   console.error("Error signing in anonymously:", error);
});
