import json

#diretório do arquivo json
jfile = "/home/victor/Documents/esp32-json/esp32-umidty-sensor-default-rtdb-export.json"

humidity_values = []
temperature_values = []

#ler o arquivo
with open(jfile, "r", encoding="utf8") as file:
    data = json.load(file)

sensors1_data = data["sensors_data"]["sensor1"]

#armazena os valores contidos no json em listas
for key, data in sensors1_data.items():
    humidity_values.append(data["humidity"])
    temperature_values.append(data["temperature"])

#faz o cálculo da média
avg_humidity = sum(humidity_values) / len(humidity_values)
avg_temperature = sum(temperature_values) / len(temperature_values)

print(f"Média de Humidade: {avg_humidity:.0f}%")
print(f"Média de Temperatura: {avg_temperature:.2f}°C")

#armazena os dados separados com seus respectivos timestamps
temperature_data = [{"temperature": data["temperature"],
                     "timestamp": data["timestamp"]} for data in sensors1_data.values()]
humidity_data = [{"humidity": data["humidity"], "timestamp": data["timestamp"]}
                 for data in sensors1_data.values()]


#ordena os 5 maiores valores de cada dado mantendo seu timestamp
top_temperatures = sorted(
    temperature_data, key=lambda x: x["temperature"], reverse=True)[:5]
top_humidity = sorted(
    humidity_data, key=lambda x: x["humidity"], reverse=True)[:5]
print("Hightest in the room\n\n")

#imprime os maiores valores colatados
for temp, hum in zip(top_temperatures, top_humidity):
    print(f"highest temperature: {temp['temperature']} in {temp['timestamp']}")
    print(f"highest humidity: {hum['humidity']} in {hum['timestamp']}")
    print("----------------------")



