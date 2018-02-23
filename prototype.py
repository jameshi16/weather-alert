from http import client
from json import decoder
import json

city = input("City: ")
apiKey = input("Insert API Key: ")

# Establish connection with Open Weather Map
owm_connection = http.client.HTTPConnection("api.openweathermap.org")

# Request for data from Open Weather Map
owm_connection.request("POST", "/data/2.5/weather?q={}&appid={}".format(city, apiKey))
reply = owm_connection.getresponse()

# Parses the reply into Python format so that I can read it
parsedReply = json.JSONDecoder().decode(reply.read().decode())

# Outputs what is happening
print("Open Weather Map reports: {}".format(parsedReply["weather"][0]["main"]))

# Closes the connection
owm_connection.close()