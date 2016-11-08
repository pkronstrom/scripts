"""
Fetch Telegram bot's new messages and
parse and print all group_id's.

>> python get_group_id.py

@author Peter Kronstrom
"""

import json
import requests

BOT_TOKEN = 'my-api-token-here'
TG_URL = 'https://api.telegram.org/bot' + BOT_TOKEN

def get_group_id():
    """Skripti noutaa botin päivitykset ja parsii vastauksesta kaikkien ryhmien group_id:t"""
    data = requests.post(TG_URL + '/getUpdates', params={'timeout': 10}).text
    data = [msg.get('message') for msg in json.loads(data)['result']]
    for msg in data:
        chat = msg.get('chat')
        if chat.get('type') == 'group':
            print('<{}> {}'.format(chat.get('title').encode('utf-8'), chat.get('id')))

if __name__ == '__main__':
    if BOT_TOKEN == 'my-api-token-here':
        print('1. Create a new bot with BotFather')
        print('(2. Give it the approppriate privileges using command "/setprivacy -> Disable)"')
        print('3. Edit the BOT_TOKEN variable in this script')
        print('4. Invite your new bot to a Telegram group')
        print('5. Send a message to your new bot in the group. I.e. "\hello@my-bot"')
        print('6. Rerun this script to see the group_id')
    else:
        get_group_id()