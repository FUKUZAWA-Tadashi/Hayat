#!/usr/bin/ruby

#
# gather PEG rules from cpp file
# line start with '//@' is assumed as rule
#

while ARGF.gets
  if /^\/\/@\s*/ =~ $_
    puts $'
  end
end
