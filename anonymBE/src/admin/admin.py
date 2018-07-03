#!/usr/bin/env python

import json
import socket
import requests

def create_group( owner_id, group_name ):
    return json.dumps( { 'user_id' : owner_id,
                         'group_name' : group_name } )

def sync_reqrep( message ):
    TCP_IP = '127.0.0.1'
    TCP_PORT = 4444
    BUFFER_SIZE = 1024
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((TCP_IP, TCP_PORT))
    s.send( message )
    data = s.recv(BUFFER_SIZE)
    s.close()
    return data

URL='http://127.0.0.1:4444'
r = requests.post( URL + '/access/group',
                   data=create_group('user01', 'group01') )
print(r.text)
