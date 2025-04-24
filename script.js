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
  const databaseRefluminosity = ref(database, "sensors_data/sensor2");

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

  onValue(databaseRefluminosity, (snapshot) => {
    const lume = snapshot.val();
    const dataArray = Array.isArray(lume) ? lume : Object.values(lume);

    const info = dataArray[dataArray.length - 1];
    let lux = info.lux.toFixed(2);
    document.getElementById("luminosity").innerHTML = `${lux} lx`;
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

  onValue(databaseRefJson, (snapshot) => {
    const firebaseJson = snapshot.val();
    const dataArray = Array.isArray(firebaseJson)
      ? firebaseJson
      : Object.values(firebaseJson);

    console.log(dataArray);

    const timestamps = dataArray.map(item => item.timestamp || "Sem data");
    const temperatures = dataArray.map(item => item.temperature);
    const humidities = dataArray.map(item => item.humidity);

    const avgTemps = temperatures.reduce((sum, data) => sum + data.temperature, 0) / dataArray.length;
    const avgHumidity = humidities.reduce((s, d) => s + d.humidity, 0) / dataArray.length;
    
    const tempChartEl = document.getElementById("tempChart");
    const humidityChartEl = document.getElementById("humidityChart");



    new Chart(document.getElementById("tempChart"), {
      type: "line",
      data: {
        labels: timestamps,
        datasets: [
          {
            label: "Temperatura Média",
            data: temperatures,
            borderColor: "rgb(255, 99, 132)",
            backgroundColor: "rgba(255, 99, 132, 0.2)",
            tension: 0.3,
            fill: true,
          },
        ],
      },
      options: {
        responsive: true,
        scales: {
          y: {
            title: {
              display: true,
              text: "Temperatura (°C)",
            },
          },
        },
      },
    });

    // Configuração do gráfico de Umidade
    new Chart(document.getElementById("humidityChart"), {
      type: "line",
      data: {
        labels: timestamps,
        datasets: [
          {
            label: "Umidade Média",
            data: humidities,
            borderColor: "rgb(54, 162, 235)",
            backgroundColor: "rgba(54, 162, 235, 0.2)",
            tension: 0.3,
            fill: true,
          },
        ],
      },
      options: {
        responsive: true,
        scales: {
          y: {
            title: {
              display: true,
              text: "Umidade (%)",
            },
            min: 0,
            max: 100,
          },
        },
      },
    });
  });

  //end of "main" function
});

