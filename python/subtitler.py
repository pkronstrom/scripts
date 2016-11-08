"""
	This is a crude script to download the first subtitle query from SubScene.

	Install:
		- python 2.7+ required
		- pip install beautifulsoup4

	How to use:
	1) copy the filename to clipboard
		- Windows: select the file, press F2, CTRL+C
	2) run the python file (doubleclicking should work fine, if python is in PATH)
	3) the subtitle file should be downloaded and unzipped to execution folder

	@author Bembu
"""

from StringIO import StringIO
from bs4 import BeautifulSoup
from Tkinter import Tk
import urllib2
import zipfile
import sys
import os

FILETYPES = [".avi", ".mp4", ".mkv", ".mov"]
query = ""

# if there is only one video file in the folder, use its name
ctr = 0
folder_files = next(os.walk("."))[2]
for f in folder_files:
	file_split = os.path.splitext(f)
	if file_split[-1].lower() in FILETYPES:
		ctr += 1
		query = file_split[0]

if (ctr > 1) or (not query):
	# otherwise get the query from cliboard
	query = Tk().clipboard_get()

print "Searching for " + query

# Subscene requires headers now, otherwise it returns Error 403
hdr = {'User-Agent': 'Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.11 (KHTML, like Gecko) Chrome/23.0.1271.64 Safari/537.11',
       'Accept': 'text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8',
       'Accept-Charset': 'ISO-8859-1,utf-8;q=0.7,*;q=0.3',
       'Accept-Encoding': 'none',
       'Accept-Language': 'en-US,en;q=0.8',
       'Connection': 'keep-alive'}

url = "http://subscene.com/subtitles/release?q=" + str(query)

# Fetch the query blob
html = urllib2.urlopen(urllib2.Request(url, headers=hdr)).read()
soup = BeautifulSoup(html, "html.parser")

# Parse all subtitle links with an english language tag
dl_link = ""
for div in soup.find_all('td', 'a1'):
	try:
		link = div.a['href']
		if "English" in div.find('span', attrs={'class':'l'}).text.strip():
			url = "http://subscene.com" + str(link)
			html = urllib2.urlopen(urllib2.Request(url, headers=hdr)).read()
			soup2 = BeautifulSoup(html, "html.parser")
			dl_link = soup2.find_all(id='downloadButton')[0]["href"]
			break;
	except:
		continue
	
if not dl_link:
	print "No subtitles found."
	sys.exit()

# fetch the download link, and filename from the Header's meta
url = "http://subscene.com" + str(dl_link)
u = urllib2.urlopen(urllib2.Request(url, headers=hdr))
meta = u.info()
file_name =  meta.getheaders("Content-Disposition")[0].split("filename=")[-1]

# download the file to StringIO and extract the zip contents to the execution folder folder
zipfile = zipfile.ZipFile(StringIO(u.read())).extractall()

print "Done."
