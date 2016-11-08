"""
A Simple selfiecam server script, as seen in
Mikrobitti Magazine 11/16.
Send a request to address localhost:5000/photo
The Raspberry Pi camera will take a photo
and send it to a Telegram group.

<api-token> and <group-id> need to be changed.

>> python selfie_cam.py

@author Peter Kronstrom
"""

from flask import Flask
import requests
import picamera

app = Flask(__name__)

@app.route('/photo', methods=['GET','POST'])
def handle_photo():
    camera_capture()
    send_to_telegram()
    return '{"success":"true"}'

def camera_capture():
    with picamera.PiCamera() as camera:
        with open('image.jpg', 'wb') as capture_file:
            camera.capture(capture_file)

def send_to_telegram():
    with open('image.jpg', 'rb') as capture_file:
        url = 'https://api.telegram.org/bot<api-token>/sendPhoto'
        f = {"photo": capture_file}
        p = {'chat_id': <group-id>}
        res = requests.post(url, files=f, params=p)

app.run(host="0.0.0.0")