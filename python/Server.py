import pandas as pd
import socket
import time
import os

HOST = "192.168.4.100"  # Standard loopback interface address (localhost)
PORT = 8090  # Port to listen on (non-privileged ports are > 1023)

collection_start_time = time.time()
# collection_end_time = 60 * 60 * 24
collection_end_time = None
filename = 'Weather_Stick_Data.csv'
timeout = 5

def transmission_to_dataframe(transmission_saved, address_saved):

    if transmission_saved is None:
        return None

    ip_address = address_saved[0]

    transmission = transmission_saved.decode('UTF-8')

    # Clean transmission:
    transmission = transmission.split(',')
    transmission = [x.lstrip(' ') for x in transmission]

    # Confirm start and end of transmission
    check1 = transmission[0] == 'SOT'
    check2 = transmission[-1] == 'EOT'
    transmission_status = True if check1 and check2 else False

    # Remove SOT and EOT
    transmission = transmission[1:-1]

    # Convert to dictionary
    keys = [x.split(' ')[0] for x in transmission]
    values = [x.split(' ')[1] for x in transmission]
    weather_data = {}
    weather_data['ip_address'] = ip_address
    weather_data['Timestamp'] = time.time()
    weather_data['transmission_status'] = transmission_status

    for key, value in zip(keys, values):
        divisor = 1
        for factor in [10, 100, 1000, 10000]:
            if '_%s_' % factor in key:
                divisor = factor
                key = key.replace('_%s' % factor, '')
        weather_data[key] = [float(value) / divisor]

    df_data = pd.DataFrame.from_dict(weather_data)
    return df_data


# https://realpython.com/python-sockets/#tcp-sockets
# https://techtutorialsx.com/2018/05/17/esp32-arduino-sending-data-with-socket-client/


filepath = os.path.join('.', filename)

df_data = None

while True:

    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.bind((HOST, PORT))
        s.listen()
        # Time out options
        
        transmission_saved = None

        conn, address = s.accept()
        with conn:

            connection_start_time = time.time()

            print(f"Connected by {address}")
            while True:

                try:
                    conn.settimeout(5)

                    connection_time = time.time() - connection_start_time

                    transmission = conn.recv(1024)
                    if not transmission:
                        break
                    print(transmission)
                    transmission_saved = transmission
                    address_saved = address

                    # Break from connection if it's taking too long
                    if connection_time > timeout:
                        transmission_saved = None
                        break
                except:
                    transmission_saved = None
                    break

    df_transmission = transmission_to_dataframe(transmission_saved, address_saved)
    if df_data is None and df_transmission is not None:
        try:
            df_data = pd.read_csv(filepath)
            df_data = pd.concat([df_data, df_transmission]) 
        except:
            df_data = df_transmission
        df_data.to_csv(filepath, index=False)
    elif df_transmission is not None:
        df_data = pd.concat([df_data, df_transmission]) 
        df_data.to_csv(filepath, index=False)

    if collection_end_time is not None:
        if time.time() - collection_start_time > collection_end_time:
            break

    print(" ")