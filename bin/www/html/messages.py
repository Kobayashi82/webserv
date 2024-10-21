import os
import json

msg_directory = 'messages'

messages = {}

if os.path.exists(msg_directory):
    for filename in os.listdir(msg_directory):
        filepath = os.path.join(msg_directory, filename)
        if os.path.isfile(filepath):
            with open(filepath, 'r') as file:
                messages[filename] = file.read()

print(json.dumps(messages))