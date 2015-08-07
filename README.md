# cwg
CWG version awesome

## Building
Building CWG is fairly straight forward. Just stand in the root directory and type:
`make all`
to build the entire project.

To see the other targets just run `make targets`

One can compile CWG without the new additions for CWG 16 by running:
`make FLAGS='NOSMOKE;NOLASER;NOREDEEMER' all`
And thus one can choose which features to be disabled


## Installing Safir
You need to have the latest version of the Safir SDK Core. 
You can download it from [here](http://sourceforge.net/projects/safir/files/).
Just follow the instructions in `INSTALL.Linux.txt` or `INSTALL.Windows.txt` to build and install.

Note that on some linux machines you will have to fix the newline delimiters in some of the `.py` files to make
them executable.
To fix run the following from the directory to which you unpacked safir:
```
dos2unix build/build.py /src/dots/src/dots/dots_v.ss/data/dobmake.py
```

## Dependencies
To be able to build the tank game one is required to do some things.

First one needs to get the proper QT 5 dependencies and the libconfuse. On ubuntu this can be done by running:
```
sudo apt-get install qtmultimedia-dev libqt5multimedia5-plugins libconfuse-dev
```

one also needs to copy the `.dou` files in `dou/` to `<path to safir>/sdk/dots/dots_generated/` 
and then run `dobmake.py`. This step is handled by the top level Makefile, and it is included in the `all` target.
