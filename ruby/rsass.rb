#!/usr/local/bin/ruby
# $Id: rsass.rb,v 1.3 2000/07/15 07:45:35 kazuma-t Exp $
#
# rsass: sass for Ruby
#

require 'getopts'
require 'sufary'

def print_usage
  $stderr.puts("Usage: rsass [OPTION]... PATTERN FILE")
  $stderr.puts("Try `rsass --help' for more information")
end

def print_help
  $stderr.print <<HELP
usage: rsass [--array FILE] [--did FILE]
        [-r -b PATTERN -e PATTERN] [-[AB] NUM] PATTERN FILE

      --array FILE
            Use FILE as the array file.
      --did FILE
            Use FILE as the DocId file. If this option is set,
            other options are ignore.
      -r
            Print the region between the patterns(see bellow).
      -b PATTERN
            Use PATTERN as the begin tag; this option should be used
            with -r option.
      -b PATTERN
            Use PATTERN as the end tag; this option should be used
            with -r option.
      -A
            Print  NUM lines of trailing context after matching lines.
      -B
            Print NUM lines of leading context before  matching lines.
HELP
end

unless getopts("r", "array:", "did:", "b:", "e:", "A:0", "B:0", "help")
  $stderr.puts("rsass: illegal option")
  print_usage
  exit(1)
end

if $OPT_help then
  print_help
  exit(1)
end

if ARGV.size != 2 then
  print_usage
  exit(1)
end

pattern = ARGV.shift
text = ARGV.shift
array = if $OPT_array then $OPT_array else nil end
did = if $OPT_did then $OPT_did else nil end

sufary = Sufary.new(text, array)

result = sufary.search(pattern)

if did then
  dd = SufaryDid.new(did, sufary)
  result.each {|i| print dd.get_doc_region(i), "\n"}
  exit(0)
end

if $OPT_r then
  unless $OPT_b and $OPT_e then
    $stderr.puts("rsass: illegal option")
    print_usage
    exit(0)
  end
  result.each {|i|
    print sufary.get_context_region(i, $OPT_b, $OPT_e), "\n"
  }
  exit(0)
end

result.each {|i|
  print sufary.get_context_lines(i, $OPT_B.to_i, $OPT_A.to_i)
}
exit(0)
