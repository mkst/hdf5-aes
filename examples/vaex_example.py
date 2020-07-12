import os

import pandas as pd
import numpy as np
import vaex as vx
import h5py

import time

HDF5_AES_FILTER = 444

t1 = time.time()

# create data 4 cols x10mio rows
a = np.random.uniform(-1, 1, 10000000)
b = np.random.uniform(-1, 1, 10000000)
c = np.random.uniform(-1, 1, 10000000)
d = np.random.uniform(-1, 1, 10000000)
table = dict(zip(['a', 'b', 'c', 'd'], [a, b, c, d]))
# print(pd.DataFrame(data=table))

t2 = time.time()
# set key
os.environ['HDF5_AES_KEY'] = 'your_secret_aes256_key_goes_here'

filename = '/dev/shm/vaex_example.h5'
# write down using h5py
with h5py.File(filename, 'w') as f:
    for column_name, data in table.items():
        f.create_dataset(column_name,
                         data=data,
                         chunks=True,
                         compression=HDF5_AES_FILTER)
t3 = time.time()

# read back in vaex
df = vx.open(filename)
print(df)
t4 = time.time()

print("Timings (seconds)")
print("=================")
print("Create data:", t2 - t1)
print("Write data: ", t3 - t2)
print("Read data:  ", t4 - t3)
