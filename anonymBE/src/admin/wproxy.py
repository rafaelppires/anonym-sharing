#!/usr/bin/env python

import json
import socket
import requests

def create_user( uid ):
    return json.dumps( { 'user_id' : uid } )

def groupuser( group_name, uid ):
    return json.dumps( { 'group_name' : group_name,
                         'user_id' : uid } )

def get_envelope( uid, bucket, key ):
    return json.dumps( { 'user_id' : uid, 'group_id' : bucket,
                         'bucket_key': key } )

URL='https://127.0.0.1:4445'
r = requests.post( URL + '/access/user',
                   data=create_user('user01'), verify=False )
print(r.text)

