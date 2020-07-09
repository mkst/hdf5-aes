# HDF5 AES Filter

This is a work-in-progress HDF5 filter that enables AES256 encryption/decryption of HDF5 datasets.

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

Example on a ~500MB file. Writes are ~2x slower than with no filter, reads are ~1.4x slower.

| Filter | Operation | Time (S) | Notes           |
|:------:|:---------:|:--------:|:----------------|
| None   | Write     | 0.94     |                 |
| None   | Read      | 0.51     |                 |
| AES    | Write     | 1.86     |                 |
| AES    | Read      | 0.71     |                 |
| gzip   | Write     | 8.86     |                 |
| gzip   | Read      | 1.78     | Default Options |

## TODO

  - [ ] Set IV/KEY via helper function, not environment vars
  - [ ] Allow encryption flavour to be specified via configuration

## Acknowledgements

With thanks to [derobins](https://github.com/derobins) for their [filter template](https://github.com/derobins/random_hdf5_filters/tree/master/filter_template).
