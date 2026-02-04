import pygame
import paho.mqtt.client as mqtt
import json
import time

# --- Configuration ---
BROKER = "localhost"
PORT = 1883
USER = "myuser"
PASS = "123"
TOPIC_CONTROL = "controller/xbox"
FREQ = 30  # 30Hz
INTERVAL = 1.0 / FREQ

# Initialize Pygame and Joystick
pygame.init()
pygame.joystick.init()

if pygame.joystick.get_count() == 0:
    print("No joystick detected!")
    exit()

joystick = pygame.joystick.Joystick(0)
joystick.init()
print(f"Connected joystick: {joystick.get_name()}")

# MQTT Setup
client = mqtt.Client(callback_api_version=mqtt.CallbackAPIVersion.VERSION2)
client.username_pw_set(USER, PASS)
client.connect(BROKER, PORT)
client.loop_start()

try:
    print(f"Publishing to {TOPIC_CONTROL} at {FREQ}Hz...")
    while True:
        start_time = time.time()
        
        # Process pygame events
        pygame.event.pump()
        
        # Standard Xbox Mapping:
        # Axis 0,1: Left Stick | Axis 3,4: Right Stick
        # Axis 5: Right Trigger (RT)
        # Button 0: A | Button 1: B | Button 2: X | Button 3: Y
        data = {
            "lx": round(joystick.get_axis(0), 2),
            "ly": round(joystick.get_axis(1), 2),
            "rx": round(joystick.get_axis(3), 2),
            "ry": round(joystick.get_axis(4), 2),
            "rt": round(joystick.get_axis(5), 2), # Right Trigger
            "btnA": joystick.get_button(0),
            "btnB": joystick.get_button(1),
            "btnX": joystick.get_button(2),       # X Button
            "btnY": joystick.get_button(3)        # Y Button
        }
        
        client.publish(TOPIC_CONTROL, json.dumps(data), qos=0)
        
        # Maintain 30Hz
        elapsed = time.time() - start_time
        sleep_time = max(0, INTERVAL - elapsed)
        time.sleep(sleep_time)

except KeyboardInterrupt:
    print("\nStopping...")
    client.loop_stop()
    client.disconnect()
    pygame.quit()
