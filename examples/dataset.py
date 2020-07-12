"""read/write a dataset to /dev/shm using the different filters"""
import os
import sys
import numpy
import h5py

HDF5_AES_FILTER = 444

def write(filename, compression=None):
    """write example.h5 using aes filter"""
    with h5py.File(filename, 'w') as f:
        f.create_dataset('my_dataset',
                         data=numpy.arange(128*1024*1024).reshape(1024, 128*1024),
                         chunks=True,
                         compression=compression)

def read(filename):
    """read back example.h5"""
    with h5py.File(filename, 'r') as f:
        dset = f['my_dataset']
        print(dset[()])

def main():
    """main function"""
    if len(sys.argv) >= 2 and sys.argv[1] in ['g', 'l', 'r', 'w', 'x']:

        if sys.argv[1] in ['r', 'w'] and len(sys.argv) == 3:
            os.environ['HDF5_AES_KEY'] = sys.argv[2]

        filename = '/dev/shm/example.h5'

        if sys.argv[1] == 'r':
            read(filename)
        elif sys.argv[1] == 'w':
            write(filename, HDF5_AES_FILTER)
        elif sys.argv[1] == 'x':
            write(filename)
        elif sys.argv[1] == 'g':
            write(filename, 'gzip')
        elif sys.argv[1] == 'l':
            write(filename, 'lzf')
    else:
        print('usage:\n  python3 example (r|w|g|l|x) [KEY]')
        print('where:')
        print('  r => read')
        print('  w => write with AES filter')
        print('  g => write with gzip filter')
        print('  l => write with lzf filter')
        print('  x => write with no filter')

if __name__ == '__main__':
    main()
