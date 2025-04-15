import { initializeApp } from "https://www.gstatic.com/firebasejs/11.6.0/firebase-app.js";
import { getAnalytics } from "https://www.gstatic.com/firebasejs/11.6.0/firebase-analytics.js";
import {
  getDatabase,
  ref,
  onValue,
} from "https://www.gstatic.com/firebasejs/11.6.0/firebase-database.js";

document.addEventListener("DOMContentLoaded", () => {
  const firebaseConfig = {
    apiKey: "AIzaSyDLTaSJJSQiB-5vFgtSzGzoxSFlWDxDof8",
    authDomain: "esp32-umidty-sensor.firebaseapp.com",
    projectId: "esp32-umidty-sensor",
    storageBucket: "esp32-umidty-sensor.firebasestorage.app",
    messagingSenderId: "134008705907",
    appId: "1:134008705907:web:eecb5add14929fb544eb06",
    measurementId: "G-X7JB1E8CWQ",
    databaseURL: "https://esp32-umidty-sensor-default-rtdb.firebaseio.com",
  };

  const app = initializeApp(firebaseConfig);
  const analytics = getAnalytics(app);

  const database = getDatabase(app);

  const databaseRefHumidity = ref(database, "sensors/humidity");
  const databaseRefTemperature = ref(database, "sensors/temperature");
  const databaseRefTimestamp = ref(database, "sensors/timestamp");
  const databaseRefJson = ref(database, "sensors_data/sensor1");

  const toggleButton = document.getElementById("toggle-table");
  const tableContainer = document.querySelector(".table-container");

  toggleButton.addEventListener("click", () => {
    tableContainer.classList.toggle("active");

    // Altera o ícone do botão  
    if (tableContainer.classList.contains("active")) {
      toggleButton.textContent = "Ocultar Histórico";
    } else {
      toggleButton.textContent = "Mostrar Histórico";
    }
  });

  onValue(databaseRefHumidity, (snapshot) => {
    const humi = snapshot.val();
    document.getElementById("humidity").innerHTML = humi + "%";
  });

  onValue(databaseRefTemperature, (snapshot) => {
    const temp = snapshot.val();
    document.getElementById("temperature").innerHTML = temp + "ºC";
  });

  onValue(databaseRefTimestamp, (snapshot) => {
    const date = new Date(snapshot.val().replace(" ", "T"));
    const pad = (n) => n.toString().padStart(2, "0");

    document.getElementById("tmstmp").innerHTML = `
          ${date.getHours()}:${pad(date.getMinutes())} ${date.getDate()}/${pad(
      date.getMonth() + 1
    )}/${date.getFullYear()}`;
  });

  onValue(databaseRefJson, (snapshot) => {
    const tableBody = document.getElementById("table-body");

    const firebaseJson = snapshot.val();
    const dataArray = Array.isArray(firebaseJson)
      ? firebaseJson
      : Object.values(firebaseJson);

    dataArray.forEach((e) => {
      const row = document.createElement("tr");
      row.style.borderBottom = "1px solid #eee";
      row.style.padding = "10px";

      row.innerHTML = `
        <td style="padding: 12px; color: #6eb7ff; font-weight: bold;">Horário ${new Date(
          e.timestamp.replace(" ", "T")
        ).toLocaleString()}</td>
        <td style="padding: 12px; color: #6eb7ff; font-weight: bold;">Temperatura ${
          e.temperature
        }ºC</td>
        <td style="padding: 12px; color: #6eb7ff; font-weight: bold;">Humidade ${
          e.humidity
        }%</td>`;

      tableBody.appendChild(row);
    });
  });
});
