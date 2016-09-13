#!/usr/bin/env python
import feedparser
import urllib2
import time
import datetime
from dateutil.parser import parse #to handle timezone offset.
import socket
from IPy import IP
import sys

if len(sys.argv) == 1:
    print "Please enter a facebook notifications RSS feed url as argument to this program.. "
    print "Usage: bukseryster.py <url>"
    sys.exit(1)

feedUrl=sys.argv[1]

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
        #stamps.append(time.mktime(time.strptime(notifications[i], "%a, %d %b %Y %H:%M:%S %z")))
        dateTimeObj=parse(notifications[i]) #the dateutil.parser handles %z (timezone) nicely, but the resulting datetime.datetime object needs to be converted to a tuple to be parsed as an epoch.. sigh...
        epoch = time.mktime(dateTimeObj.timetuple())
        #print epoch
        stamps.append(epoch)
    return stamps

#STARTUP:

print time.ctime(),"startup!"

TCP_IP = '62.212.66.171'
TCP_PORT = 31337
BUFFER_SIZE = 20  # Normally 1024, but we want fast response

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1) #make socket reuseable, for debugging (enables you to rerun the program before the socket has timed out)
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
        while True: #looks like connection timeout is ~60 seconds.

            print time.ctime(),'Poking facebook...'
            newTimeStamps=getStamps(getNotifications(feedUrl))
            diff=[]
            if len(newTimeStamps)>0:
                diff = list(set(newTimeStamps) - set(oldTimeStamps)) #sets are subtractable, yay!
                oldTimeStamps=newTimeStamps
            newNotifications+=len(diff)
            #lastTime=time.time()
            print "New notifications:", newNotifications
            conn.send(str(newNotifications))+'\r')  # echo
            print 'TX:',newNotifications
            newNotifications=0
            time.sleep(30)

        #print 'Client disconnected.'
        #print '--------------------------'
        #conn.close()
    except Exception as e:
        #print "hmm.. It looks like there was an error: " + str(e)
        print time.ctime(),'Client disconnected... :',str(e)
        print '--------------------------'
        conn.close()
