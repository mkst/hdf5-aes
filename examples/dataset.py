"""read/write a dataset using the aes filter"""
import os
import sys
import numpy
import h5py

HDF5_AES_FILTER = 444

def write(filename, compression=None):
    """write example.h5 using aes filter"""
    with h5py.File(filename, 'w') as f:
        f.create_dataset('my_dataset',
                         data=numpy.arange(64*1024*1024).reshape(1024, 64*1024),
                         chunks=True,
                         compression=compression)

def read(filename):
    """read back example.h5"""
    with h5py.File(filename, 'r') as f:
        dset = f['my_dataset']
        print(dset[()])

def main():
    """main function"""
    if len(sys.argv) >= 2 and sys.argv[1] in ['r', 'w', 'x', 'z']:

        if sys.argv[1] in ['r', 'w']:
            os.environ['HDF5_AES_IV'] = 'please_change_me'
            os.environ['HDF5_AES_KEY'] = sys.argv[2]

        filename = 'example.h5'

        if sys.argv[1] == 'r':
            read(filename)
        elif sys.argv[1] == 'w':
            write(filename, HDF5_AES_FILTER)
        elif sys.argv[1] == 'x':
            write(filename, None)
        elif sys.argv[1] == 'z':
            write(filename, 'gzip')
    else:
        print('usage:\n  python3 example (r|w|x|z) [KEY]')

if __name__ == '__main__':
    main()
