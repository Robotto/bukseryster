#!/usr/bin/env python
import feedparser
import urllib2
import time
import datetime
import socket
from IPy import IP
import sys

if len(sys.argv) == 1:
	print "Please enter a facebook notifications RSS feed url as argument to this program.. "
	print "Usage: bukseryster.py <url>"
	sys.exit(1)

feedUrl=sys.argv[1]
loopDelay=15

#get the feed and parse it:
def getNotifications(url):
	notifications=[]
	try:
		rawFeed=urllib2.urlopen(url)
		parsedFeed=feedparser.parse(rawFeed)

		#sort notifications into a set:
		for i in range(len(parsedFeed.entries)):
			notifications.append(parsedFeed.entries[i].published)
			#notifications.append(parsedFeed.entries[i].title)

	except Exception as e:
		print "hmm.. It looks like there was a network timeout: " + str(e)

	return notifications

#create timestamps from notification dates:
def getStamps(notifications):
	stamps=[]
	for i in range(len(notifications)):
									#rfc2822#section-3.3 format: "Tue, 26 May 2015 15:58:39 -0100"
		stamps.append(time.mktime(time.strptime(notifications[i], "%a, %d %b %Y %H:%M:%S -0100")))
	return stamps

#STARTUP:

print time.ctime(),"startup!"

TCP_IP = '62.212.66.171'
TCP_PORT = 31337
BUFFER_SIZE = 20  # Normally 1024, but we want fast response

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.bind((TCP_IP, TCP_PORT))
s.listen(1)

print 'Poking facebook...'
oldTimeStamps=getStamps(getNotifications(feedUrl))
lastTime = int(time.time()) #Update time
newNotifications=0
print 'Done.. Opening TCP port', TCP_PORT

while True:
    try:
        conn, addr = s.accept()
        print time.ctime(),'Connection from:', addr
        while 1:
            data = conn.recv(BUFFER_SIZE)
            if not data: break
            print "RX:", data
            print "poking facebook..."
            newTimeStamps=getStamps(getNotifications(feedUrl))
            diff=[]
            if len(newTimeStamps)>0:
                diff = list(set(newTimeStamps) - set(oldTimeStamps)) #sets are subtractable, yay!
                oldTimeStamps=newTimeStamps
            newNotifications+=len(diff)
            #lastTime=time.time()
            print "New notifications:", newNotifications
            conn.send(str(newNotifications))  # echo
            print 'TX:',newNotifications
            newNotifications=0
            
        print 'Client disconnected.'
        print '--------------------------'
        conn.close()
    except Exception as e:
        print "hmm.. It looks like there was an error: " + str(e)
