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
