# cwg
CWG version awesome


## Installing Safir
You need to have the latest version of the Safir SDK Core. 
You can download it from [here](http://sourceforge.net/projects/safir/files/).
Just folow the instructions in `INSTALL.Linux.txt` or `INSTALL.Windows.txt` to build and install.

Note that on some linux machines you will have to fix te newline delimiters in some of the `.py` files to make
them executable.
To fix run the folowing from the directory to wich you unpacked safir:
```
dos2unix build/build.py /src/dots/src/dots/dots_v.ss/data/dobmake.py
```

## Dependencies
To be able to build the tank game one is reqiered to do some things.

First one needs to get the propper QT 5 dependencies. On ubuntu this can be done by running:
```
sudo apt-get install qt5multimedia-dev libqt5multimedia5-plugins
```

one also needs to copy the `.dou` files in `dou/` to `<path to safir>/sdk/dots/dots_generated/` 
and then run `dobmake.py`
