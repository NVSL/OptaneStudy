#!/usr/bin/env python3

from google.cloud import datastore

client = datastore.Client()
query = client.query(kind='aepsweep')
query.add_filter('avgpower', '=', 12500)
query.add_filter('machine', '=', 'AEP1')
query.add_filter('baddata', '=', '1')

l = []
result = list(query.fetch())
print(len(result))
for item in result:
    l.append(item.key)
    if len(l) >= 500:
        client.delete_multi(l)
        l = []
client.delete_multi(l)



