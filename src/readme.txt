== Changelog ==
 * 01/08/2014
 	- Multiple memory fixes
 * 11/06/2013
	- Final 2.x.x release
	- Database header
		- Magic checksum
		- Timestamp
		- Version
	- File refactor
 * 11/05/2013
	- 64/32 bit fix
	- Option flags
 * 11/03/2013
	- Final 1.x.x release
	- Passed test
	- Add truncate option
 * 10/31/2013
	- Left, right shift solved
	- Commands uppercase fix
	- Complete I/O stats
	- Auto sequence

== Datatypes ==
 Numeric
  * INT		Integer

 Real
  * FLOAT	Floating point precision
  * DOUBLE	Double floating point percision

 Text
  * CHAR	Fixed character length
  * VARCHAR	Variable character length

 Other
  * BOOL	Logical switch
  * NULL	Empty datafield

== Design ==

[database]
.{lock}
    |
    |-->[column]-->[column*]
    |   .{name}
    |   .{size}
    |   .{type}
    |   .{signness}
    |
    `-->[node]-->[node5]
        .{key}
        .{count}
           |
           |
           `-->[field]-->[field*]
               .{size}
                  |
              {datafield}
