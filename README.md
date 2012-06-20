lib sequence parser
===================

**lib sequence parser** is a lightweight opensource library to find file sequences.

The project is actively maintained as an open source project by [Mikros Image](http://www.mikrosimage.eu).

Definitions
-----------

A file sequence is an object having the following properties

    path/prefix####suffix

* path
* prefix
* padding >=1 ( number of '#' )
* suffix
* start
* end
* step >=1

File extension is included in the suffix

Padding
-------

If padding is 1, this means _no strict padding_  
> file#.png at index 350 will display file350.png

If padding is greater than 1, strict padding is enforced  
> file####.png at index 350 will display file0350.png

License
-------

This project is released under the 3-clause BSD license ("New BSD License" or "Modified BSD License").  
See LICENCE.txt

Authors
-------

See AUTHORS.md