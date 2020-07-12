# HDF5 AES Filter

This is a work-in-progress HDF5 filter that enables [AES256](https://en.wikipedia.org/wiki/Advanced_Encryption_Standard) encryption/decryption of HDF5 datasets.

## Quickstart

Assumes you're running on Ubuntu 20.04 with HDF5 v1.10.4.

### Build
```sh
$ docker build . -t aes
$ mkdir -p build
$ docker run --rm -ti -v $(pwd):/app aes sh -c "cd /app/build && cmake ../src && make"
```
### Run
```sh
$ export HDF5_PLUGIN_PATH=$(pwd)/build
$ python3 examples/dataset.py w your_secret_aes256_key_goes_here
$ python3 examples/dataset.py r your_secret_aes256_key_goes_here
```

## Performance

Testing with a 1GB file against `/dev/shm` via the [dataset.py](/examples/dataset.py) example. It takes `~0.35` seconds to create the dataset before the write; this has been subtracted in the times recorded below.

| Filter  | Operation | Time (S) | Notes         |
|:-------:|:---------:|:--------:|:--------------|
| None    | Write     | 0.50     |               |
| None    | Read      | 0.60     |               |
| **AES** | **Write** | **1.65** |               |
| **AES** | **Read**  | **0.88** |               |
| gzip    | Write     | 12.30    |               |
| gzip    | Read      | 2.52     | Default Level |
| lzf     | Write     | 2.30     |               |
| lzf     | Read      | 1.72     |               |

Writes are `~3.3x` slower than with no filter, reads are `~1.5x` slower. If you are already using a compression filter, the impact of also encrypting/decrypting is fairly low.

## Notes

The 16-byte [IV](https://en.wikipedia.org/wiki/Block_cipher_mode_of_operation#Initialization_vector_(IV)) is created randomly via calls to `rand()` for each *chunk* that is encrypted. Each chunk is prepended with the IV during encryption; this 'dummy' block is then removed as part of the decryption process. This ensures that encrypting the same data with the same key does not generate the same ciphertext.

## TODO

  - [ ] Set KEY via helper function, not environment vars
  - [ ] Allow encryption flavour to be specified via configuration
  - [ ] Add support to h5py to support encryption + compression

## Acknowledgements

With thanks to [derobins](https://github.com/derobins) for their [filter template](https://github.com/derobins/random_hdf5_filters/tree/master/filter_template).
