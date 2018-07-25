#!/usr/bin/env python

import json
import socket
import requests

def create_group( owner_id, group_name ):
    return json.dumps( { 'user_id' : owner_id,
                         'group_name' : group_name } )

def create_user( uid ):
    return json.dumps( { 'user_id' : uid } )

def add_user2group( group_name, uid ):
    return json.dumps( { 'group_name' : group_name,
                         'new_member_id' : uid } )

def delete_userFgroup( group_name, uid ):
    return json.dumps( { 'group_name' : group_name,
                         'revoke_member_id' : uid } )

def get_envelope( uid, bucket, key ):
    return json.dumps( { 'user_id' : uid, 'bucket_id' : bucket,
                         'bucket_key': key } )

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
r = requests.post( URL + '/access/user',
                   data=create_user('user01') )
print(r.text)
r = requests.post( URL + '/access/user',
                   data=create_user('user02') )
print(r.text)
r = requests.post( URL + '/access/user',
                   data=create_user('user03') )
print(r.text)
r = requests.post( URL + '/access/group',
                   data=create_group('user02', 'group02') )
print(r.text)
r = requests.put( URL + '/access/usergroup',
                  data=add_user2group( 'group02', 'user01' ) )
print(r.text)
r = requests.put( URL + '/access/usergroup',
                  data=add_user2group( 'group02', 'user03' ) )
print(r.text)
r = requests.delete( URL + '/access/usergroup',
                  data=delete_userFgroup( 'group01', 'user01' ) )
print(r.text)
r = requests.delete( URL + '/access/usergroup',
                  data=delete_userFgroup( 'group02', 'user01' ) )
print(r.text)
r = requests.delete( URL + '/access/usergroup',
                  data=delete_userFgroup( 'group02', 'user01' ) )
print(r.text)
r = requests.get( URL + '/verifier/envelope',
                  data=get_envelope( 'user01', 'group02', 'abacaxi' ) )
print(r.text)
r = requests.get( URL + '/verifier/envelope',
                  data=get_envelope( 'user03', 'group02', 'jabuticaba' ) )
print(r.text)

