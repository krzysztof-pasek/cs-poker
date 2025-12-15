import network


client = network.Client("127.0.0.1", 8080)
print("Starting client...")
client.run()

