import pyaudio, socket

IP = '10.0.0.7'
PORT = 9999

RATE = 10240
CHUNK = 1024

server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

py = pyaudio.PyAudio()
speaker = py.open(format=pyaudio.paUInt8, output=True, channels=1, rate=RATE)

print(f'Binding Server To: {IP}:{PORT}')
server.bind((IP, PORT))
server.listen(5)
print(f'Binding Was Successful')

while(True):
    client, addr = server.accept()
    print(f'{addr} Connected')
    while(client):
        data = client.recv(CHUNK)
        print(data)
        speaker.write(data)
