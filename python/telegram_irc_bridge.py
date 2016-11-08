"""
	A script that acts as a messaging bridge between Telegram and IRC.
	Works in Python 3.x

	Installation:
	1.	pip install pydle
	2.	go create a Telegram bot by talking to BotFather (https://telegram.me/botfather)
	3.	create a telegram group, and send a curl to:
		curl -s -X POST "https://api.telegram.org/bot<BOT_TOKEN>w/getUpdates"
	4.	get the group_id from the response
	4.	fill in the static variables below (including the newly acquired group_id)
	5.	run the script!
"""

import json
import logging
import threading

import pydle

import requests


USER = 'username'
BOT_TOKEN = 'my-tg-bot-token'
GROUP_ID = -12345
TG_API_URL = 'https://api.telegram.org/bot' + BOT_TOKEN
MSG_ON_JOIN = "Hello! I am a bot and will act as a bridge for my host :)"
CHANNEL = '#my-channel'
IRC_SERVER = 'eu.ircnet.org'

logging.basicConfig(filename='ircbot.log', format='%(asctime)s.%(msecs)03d %(levelname)s %(funcName)s %(message)s',
                    datefmt='%Y-%m-%d,%H:%M:%S', level=logging.DEBUG)

class MyClient(pydle.Client):
    def on_connect(self):
        super().on_connect()
        self.join(CHANNEL)
        logging.info('Connected. Joining channel {}'.format(CHANNEL))

    def on_join(self, channel, user):
        super().on_join(channel, user)
        logging.info("Joined to channel: {} - {}".format(channel, user))

    def on_message(self, target, by, message):
        logging.info("Got a message: <{}> {}".format(by, message))
        msg = "<b>&lt;{}&gt;</b> {}".format(by, message)
        params = {'chat_id': GROUP_ID, 'parse_mode': 'HTML', 'text': msg}
        requests.post(TG_API_URL + '/sendMessage', params=params)
        logging.info("Message <{}> forwarded to Telegram group {} as HTML.".format(msg, GROUP_ID))

    def process_tg_forever(self):
        logging.info("Now in process_tg_forever")
        offset = 0
        while True:
            res = requests.post(TG_API_URL + '/getUpdates', params={'offset': offset, 'timeout': 120, 'limit': 10}).text
            if res:
                data = json.loads(res)
                if data.get('ok'):
                    logging.info("TG: {}".format(json.dumps(data)))
                    for msg in data.get('result'):
                        if msg.get('update_id') >= offset:
                            logging.info("New update ID: {} + 1".format(msg.get('update_id')))
                            offset = msg["update_id"] + 1
                        logging.info("<{}> {}".format(msg.get('message').get('from').get('username'), msg.get('message').get('text')))
                        self.message(CHANNEL, msg.get('message').get('text'))
                else:
                    logging.info("Weird TG data: {}".format(json.dumps(data)))


def main():
    logging.debug("Creating a client")
    client = MyClient(USER)
    logging.debug("Connecting to a server")
    client.connect(IRC_SERVER, tls=False)

    thread = None
    if client.connected:
        logging.debug("Starting the IRC bot thread")
        thread = threading.Thread(target=client.handle_forever)
        thread.start()

    logging.debug("Now serving the telegram process on main thread")
    client.process_tg_forever()

    client.quit('EOF on standard input')
    if thread:
        thread.join()

    logging.debug("Quit the process.")

if __name__ == '__main__':
    main()