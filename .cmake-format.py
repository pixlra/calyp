# ----------------------------------
# Options affecting listfile parsing
# ----------------------------------
with section("parse"):

  # Specify structure for custom cmake functions
  additional_commands = { 'foo': { 'flags': ['BAR', 'BAZ'],
             'kwargs': {'DEPENDS': '*', 'HEADERS': '*', 'SOURCES': '*'}}}

  ## Override configurations per-command where available
  #override_spec = {}

  # Specify variable tags.
  vartags = []

  # Specify property tags.
  proptags = []

# -----------------------------
# Options affecting formatting.
# -----------------------------
with section("format"):

  # How wide to allow formatted cmake files
  line_width = 150

  # How many spaces to tab for indent
  tab_size = 2

  # If an argument group contains more than this many sub-groups (parg or kwarg
  # groups) then force it to a vertical layout.
  max_subgroups_hwrap = 2

  # If a positional argument group contains more than this many arguments, then
  # force it to a vertical layout.
  max_pargs_hwrap = 6

  dangle_parens = True

  # Format command names consistently as 'lower' or 'upper' case
  command_case = 'upper'

  # Format keywords consistently as 'lower' or 'upper' case
  keyword_case = 'upper'


# ----------------------------
# Options affecting the linter
# ----------------------------
with section("lint"):

  # a list of lint codes to disable
  disabled_codes = ["C0103"]

  # regular expression pattern describing valid function names
  function_pattern = '[0-9a-z_]+'

  # regular expression pattern describing valid macro names
  macro_pattern = '[0-9A-Z_]+'

  # regular expression pattern describing valid names for variables with global
  # (cache) scope
  global_var_pattern = '[A-Z][0-9A-Z_]+'

  # regular expression pattern describing valid names for variables with global
  # scope (but internal semantic)
  internal_var_pattern = '_[A-Z][0-9A-Z_]+'

  # regular expression pattern describing valid names for variables with local
  # scope
  local_var_pattern = '[a-z][a-z0-9_]+'

  # regular expression pattern describing valid names for privatedirectory
  # variables
  private_var_pattern = '_[0-9a-z_]+'

  # regular expression pattern describing valid names for public directory
  # variables
  public_var_pattern = '[A-Z][0-9A-Z_]+'

  # regular expression pattern describing valid names for function/macro
  # arguments and loop variables.
  argument_var_pattern = '[a-z][a-z0-9_]+'

  # regular expression pattern describing valid names for keywords used in
  # functions or macros
  keyword_pattern = '[A-Z][0-9A-Z_]+'

  # In the heuristic for C0201, how many conditionals to match within a loop in
  # before considering the loop a parser.
  max_conditionals_custom_parser = 2

  # Require at least this many newlines between statements
  min_statement_spacing = 1

  # Require no more than this many newlines between statements
  max_statement_spacing = 2
  max_returns = 6
  max_branches = 12
  max_arguments = 5
  max_localvars = 15
  max_statements = 50

# -------------------------------
# Options affecting file encoding
# -------------------------------
with section("encode"):

  # If true, emit the unicode byte-order mark (BOM) at the start of the file
  emit_byteorder_mark = False

  # Specify the encoding of the input file. Defaults to utf-8
  input_encoding = 'utf-8'

  # Specify the encoding of the output file. Defaults to utf-8. Note that cmake
  # only claims to support utf-8 so be careful when using anything else
  output_encoding = 'utf-8'
