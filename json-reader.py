import firebase_admin
from firebase_admin import credentials, db
from datetime import datetime, timedelta

# Carregar a credencial do Firebase
cred = credentials.Certificate("/home/victor/Documents/esp32-json/esp32-umidty-sensor-firebase-adminsdk-fbsvc-a668911970.json")
firebase_admin.initialize_app(cred, {"databaseURL": "https://esp32-umidty-sensor-default-rtdb.firebaseio.com"})

# Obtendo automaticamente o JSON armazenado no Realtime Database
ref = db.reference("/")
data = ref.get()
print(data)



humidity_values = []
temperature_values = []
daily_data = {}

sensors1_data = data["sensors_data"]["sensor1"]

#armazena os valores contidos no json em listas
for key, data in sensors1_data.items():
    humidity_values.append(data["humidity"])
    temperature_values.append(data["temperature"])

#faz o cálculo da média
avg_humidity = sum(humidity_values) / len(humidity_values)
avg_temperature = sum(temperature_values) / len(temperature_values)

print(f"Média de Humidade: {avg_humidity:.0f}%")
print(f"Média de Temperatura: {avg_temperature:.2f}°C\n\n")

#armazena os dados separados com seus respectivos timestamps
temperature_data = [{"temperature": data["temperature"],
                     "timestamp": data["timestamp"]} for data in sensors1_data.values()]
humidity_data = [{"humidity": data["humidity"], "timestamp": data["timestamp"]}
                 for data in sensors1_data.values()]


#ordena os 5 maiores valores de cada dado mantendo seu timestamp
top_temperatures = sorted(temperature_data, key=lambda x: x["temperature"], reverse=True)[:5]
top_humidity = sorted(humidity_data, key=lambda x: x["humidity"], reverse=True)[:5]

#ordena os 5 menores valores de cada mantendo seu timestamp
lowest_temperature = sorted (temperature_data, key=lambda x: x["temperature"])[:5]
lowest_humidity = sorted (humidity_data, key=lambda x: x["humidity"])[:5]

#imprime os maiores valores colatados
print("Hightest temperatures and humidities")
for temp, hum in zip(top_temperatures, top_humidity):
    print("--------------------------------------------")
    print(f"highest temperature: {temp['temperature']} in {temp['timestamp']}")
    print(f"highest humidity: {hum['humidity']} in {hum['timestamp']}")
print("************************************************")
print("Lowest temperatures and humidities")
for temp, hum, in zip (lowest_temperature, lowest_humidity):
    print("--------------------------------------------")
    print(f"lowest temperature: {temp['temperature']} in {temp['timestamp']}")
    print(f"lowest humidity: {hum['humidity']} in {hum['timestamp']}")

for key, entry in sensors1_data.items():
    # Verifica se o timestamp é válido
    timestamp_str = entry.get("timestamp", "")
    
    # Pula entradas com timestamp inválido
    if timestamp_str == "Erro" or not timestamp_str:
        continue
        
    try:
        # Tenta primeiro o formato ISO
        try:
            timestamp = datetime.strptime(timestamp_str, "%Y-%m-%d %H:%M:%S")
        except ValueError:
            # Tenta o formato brasileiro
            timestamp = datetime.strptime(timestamp_str, "%d/%m/%Y %H:%M:%S")
            
        date_key = timestamp.date()
        
        if date_key not in daily_data:
            daily_data[date_key] = {
                "temperatures": [],
                "humidities": [],
                "timestamps": []
            }
        
        # Adiciona apenas se os valores forem numéricos
        if isinstance(entry.get("temperature", None), (int, float)):
            daily_data[date_key]["temperatures"].append(entry["temperature"])
        
        if isinstance(entry.get("humidity", None), (int, float)):
            daily_data[date_key]["humidities"].append(entry["humidity"])
            
        daily_data[date_key]["timestamps"].append(timestamp_str)
            
    except ValueError as e:
        print(f"Timestamp inválido ignorado: {timestamp_str} - {e}")
        continue

# Verifica se temos dados válidos
if not daily_data:
    print("Nenhum dado válido encontrado no banco de dados!")
    exit()

# Ordena as datas disponíveis
sorted_dates = sorted(daily_data.keys(), reverse=True)

# Pega o dia mais recente (hoje ou último dia disponível)
latest_date = sorted_dates[0]
today = datetime.now().date()

# Seleciona a data apropriada
selected_date = today if today in daily_data else latest_date
date_status = "hoje" if selected_date == today else f"último dia disponível ({selected_date})"

print(f"\nEstatísticas para {date_status}:")

# Calcula estatísticas para o dia selecionado
day_stats = daily_data[selected_date]

if day_stats["temperatures"]:
    print("--------------------------------------------")
    avg_temp = sum(day_stats["temperatures"]) / len(day_stats["temperatures"])
    max_temp = max(day_stats["temperatures"])
    min_temp = min(day_stats["temperatures"])
    print(f"Average temperature: {avg_temp:.2f}°C")
    print(f"Maximum temperature: {max_temp:.2f}°C")
    print(f"Mininum temperature: {min_temp:.2f}°C")
else:
    print("No valid data for this period")

if day_stats["humidities"]:
    print("--------------------------------------------")
    avg_humidity = sum(day_stats["humidities"]) / len(day_stats["humidities"])
    max_humidity = max(day_stats["humidities"])
    min_humidity = min(day_stats["humidities"])
    print(f"Average humidity: {avg_humidity:.0f}%")
    print(f"Maximum humidity: {max_humidity:.0f}%")
    print(f"Mininum humidity: {min_humidity:.0f}%")
else:
    print("No valid data for this period")
    

