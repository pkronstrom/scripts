"""
A bit more advanced server script for the MikroBitti selfie-cam.
localhost:5000/photo takes a photo,
localhost:5000/video takes a short video.

Afterwards, the photo/video is sent to the
predefined Telegram group channel.

Edit the variables
BOT_API_TOKEN ja GROUP_ID
before running the script.

>> python selfie_cam_advanced.py

Plase notice, that the video functionality requires
GPAC-binaries.

>> sudo apt-get update
>> sudo apt-get install gpac

@author Peter Kronstrom
"""

from flask import Flask
import requests
import picamera
import os

BOT_API_TOKEN = 'my-api-token-here'
GROUP_ID = -123456
TG_URL = 'https://api.telegram.org/bot' + BOT_API_TOKEN

app = Flask(__name__)

@app.route('/photo', methods=['GET','POST'])
def handle_photo():
    camera_capture('image.jpg', 'photo')
    send_to_telegram('image.jpg', 'photo')
    return '{"success":"true"}'

@app.route('/video', methods=['GET','POST'])
def handle_video():
    camera_capture('video.h264', 'video', length=3)
    convert_h264_to_mp4()
    send_to_telegram('video.mp4', 'video')
    return '{"success":"true"}'

def convert_h264_to_mp4():
    """ Translate .h264 video to mp4 using MP4Box.
        Requires GPAC binaries.
        >> sudo apt-get update
        >> sudo apt-get install gpac """
    os.system('rm video.mp4')
    os.system('MP4Box -add video.h264:fps=24 video.mp4')
    os.system('rm video.h264')

def camera_capture(filename, type, length=3):
    with picamera.PiCamera() as camera:
        with open(filename, 'wb') as capture_file:
            """ See the following link for more camera settings:
                https://picamera.readthedocs.io/en/release-1.12/api_camera.html """
            camera.hflip = True
            camera.vflip = True
            if type == 'photo':
                camera.resolution = (1296, 972)
                camera.capture(capture_file)
            elif type == 'video':
                camera.resolution = (640, 480)
                camera.framerate = 24
                camera.start_recording(capture_file)
                camera.wait_recording(length)
                camera.stop_recording()

def send_to_telegram(filename, type):
    with open(filename, 'rb') as capture_file:
        if type == 'photo':
            url = TG_URL + '/sendPhoto'
            files = {"photo": capture_file}
        elif type == 'video':
            url = TG_URL + '/sendVideo'
            files = {"video": capture_file}
        res = requests.post(url, files=files, params={'chat_id': GROUP_ID})

if __name__ ==  '__main__':
    app.run(host="0.0.0.0")