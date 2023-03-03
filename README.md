# FileManager
Simple file manager made with ncurses library

Сделал следующие пункты:
Базовая часть.
1, 2, 4, 5, 6, 7 из доп. части.

How to build and run:
```
mkdir build
cd build
cmake ..
make
./file_manager
```

Supported features:
1. Enter - open directory or file. Supported extensions are in dynamic library.
2. X, V - cut and paste.
3. C, V - copy and paste.
4. H - hide/unhide .file_name files.
5. D - delete file.

Warning: this programm is extremly dangerous as it can contain bugs which can cause data loss.
