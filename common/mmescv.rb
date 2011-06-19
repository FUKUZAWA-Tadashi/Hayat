#!/usr/bin/ruby

# copyright 2010 FUKUZAWA Tadashi

# convert message text to binary
#
# to convert SJIS text, use command:
# $ ruby -Ks mmescv.rb mm_sjis.txt > mm_sjis.mm
# to convert EUC text, use command:
# $ ruby -Ke mmescv.rb mm_euc.txt > mm_euc.mm



def unesc(m)
  a = m.split("\\")
  x = a.shift
  skip = false
  while (a.size > 0)
    b = a.shift
    c = ''
    if skip
      skip = false
      c = b
    else
      case b
      when ''
        c = "\\"
        skip = true
      when /^\d\d\d/
        c = $&.oct.chr + $'
      when /^x([0-9a-fA-F][0-9a-fA-F])/
        c = $1.hex.chr + $'
      when /^n/
        c = "\n" + $'
      when /^r/
        c = "\r" + $'
      when /^t/
        c = "\t" + $'
      when /^0/
        c = "\0" + $'
      else
        c = b
      end
    end
    x += c
  end
  x + "\0"
end


mess = []

mode = :sep
orgtxt = nil
while line = gets
  line.chomp!
  case line
  when /^$/
    mode = :sep
  when /^\/\//
    mode = :sep
  else
    if mode == :org
      mess.push [orgtxt, line]
      mode = :sep
    else
      orgtxt = line
      mode = :org
    end
  end
end


print "Mm"
print [mess.size].pack('n')     # big endian unsigned 16bit

print "abcdABCD" * mess.size    # used as MMes::m_mes_tbl

mess.sort_by{|x| x[0]}.each do |ms|
  print unesc(ms[0])
  print unesc(ms[1])
end
