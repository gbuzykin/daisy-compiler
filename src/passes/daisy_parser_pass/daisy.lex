%start string

dig       [0-9]
odig      [0-7]
hdig      [0-9a-fA-F]
letter    [a-zA-Z]
dec       {dig}+
hex       0(x|X){hdig}+
itype     (i|u)(8|16|32|64)?
real      (({dig}+(\.{dig}*)?)|(\.{dig}+))((e|E)(\+?|\-?){dig}+)?
id        ({letter}|_)({letter}|{dig}|_)*
ws        [ \r\t]

%%

# String literal
escape_oct      <string> \\{odig}{1,3}
escape_hex      <string> \\x{hdig}{1,2}
escape_a        <string> \\a
escape_b        <string> \\b
escape_f        <string> \\f
escape_r        <string> \\r
escape_n        <string> \\n
escape_t        <string> \\t
escape_v        <string> \\v
escape_other    <string> \\.
string_seq      <string> [^"\\\n]+
string_close    <string> \"
string_nl       <string> \n
string_ln_wrap  <string> \\\n

# Operators
inc           "++"
dec           "--"
shl           "<<"
shr           ">>"
eq            "=="
ne            "!="
le            "<="
ge            ">="
and           "&&"
or            "||"
add_assign    "+="
sub_assign    "-="
mul_assign    "*="
div_assign    "/="
mod_assign    "%="
and_assign    "&="
or_assign     "|="
xor_assign    "^="
shl_assign    "<<="
shr_assign    ">>="
arrow         "->"

# Boolean literals
true_literal     "true"
false_literal    "false"

# Numerical literals
dec_literal      {dec}{itype}?
hex_literal      {hex}{itype}?
real_literal     {real}

# Identifier
id               {id}

# Comments
comment1         "//"
comment2         "/*"

# Other
whitespace       {ws}+
nl               \n
fake_nl          \\\n
esc_char         \\.
scope_resolution "::"
ellipsis         "..."
sharp            #
concatenate      ##
string           \"
other_char       .

%%
