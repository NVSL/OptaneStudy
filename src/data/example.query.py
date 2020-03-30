#!/usr/bin/python3
import csv
import os
import pandas as pd
from google.cloud import datastore
from pivottablejs import pivot_ui
import subprocess

client = datastore.Client()
query = client.query(kind='aepsweep')
query.add_filter('baddata', '=', 0)
query.add_filter('task', '=', 'Rand')
#query.add_filter('avgpower', '=', 15000)
#query.add_filter('interleaving', '=', 1)
#query.add_filter('task_type', '=', 'RandBW')
query.add_filter('operation', '=', 'Store+Flush')
query.add_filter('tag', '=', 'storeflush')
data = []
results = list(query.fetch())

df = pd.DataFrame(results)
df = df.loc[:, (df != df.iloc[0]).any()] 
#df = df.drop(columns=['tag', 'date', 'avgpower', 'peakpower'])
pivot_ui(df)

with open('output.csv', 'w') as writeFile:
    writer = csv.writer(writeFile)
    writer.writerows(data)
