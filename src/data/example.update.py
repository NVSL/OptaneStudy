#!/usr/bin/python3
import csv
import os
import pandas as pd
from google.cloud import datastore
from pivottablejs import pivot_ui
import subprocess

client = datastore.Client()
query = client.query(kind='aepsweep')
#query.add_filter('baddata', '=', 0)
#query.add_filter('task', '=', 'Rand')
#query.add_filter('avgpower', '=', 15000)
#query.add_filter('interleaving', '=', 1)
#query.add_filter('task_type', '=', 'RandBW')
#query.add_filter('operation', '=', 'Store+Flush')
query.add_filter('tag', '=', 'storeflush')
data = []
results = list(query.fetch())

count = 0
for item in results:
#    if item['stride_size'] == item['access_size']:
    throughput = item['throughput']
    tag = item.key.name
    line = subprocess.check_output('gsutil cat gs://gcp-bucket/'+tag+'/stdout.txt | grep "media\:imc.write" | awk \'{print $2}\'', shell=True)
    ewr = 1.0/float(line)
    data.append([tag, ewr, throughput])
    count += 1
    print('[{0}/{1}]'.format(count, len(results)), ewr, throughput, tag)
    item.update({'ewr': ewr})
    client.put(item)

