#!/usr/bin/ruby

# copyright 2010 FUKUZAWA Tadashi

# message extractor for hyMMes

require "find"


dirs = ARGV.size > 0 ? ARGV : ['.']

puts "// made by: #{$0} #{ARGV.join(' ')}"
puts ''

mess = {}

dirs.each do |dir|
  Find.find(dir) do |f|
    next unless /\.(cpp|h)$/ =~ f
    next unless File.file?(f)
    File.open(f) do |inp|
      inp.each_line do |line|
        if /M_M\(\s*"(.*)"\s*\)/ =~ line
          m = $1
          mess[m] ||= []
          mess[m].push "#{f}:#{inp.lineno}"
        end
      end
    end
  end
end

mess.keys.sort.each do |k|
  mp = mess[k]
  mp.each do |pos|
    puts "// #{pos}"
  end
  puts k
  puts ''
end
