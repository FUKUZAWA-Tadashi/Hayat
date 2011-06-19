#!/usr/bin/ruby

# copyright 2010 FUKUZAWA Tadashi

# �W�����͂���1�s���ǂݍ���ŁAHayat�����Ŏg�����ʎq�R�[�h�ɃG���R�[�h����
# -d �I�v�V�������w�肷��ƁA�f�R�[�h����


mode = :decode
if ARGV[0] == '-d'
  mode = :encode
  ARGV.shift
end


if mode == :decode
  while (s = gets)
    s.chomp!
    if s == ''
      //
    elsif /^[A-Za-z0-9_]+$/ =~ s
      puts 'a_' + s
    else
      puts 'x_' + s.unpack('H*')[0]
    end
  end
else
  while (s = gets)
    s.chomp!
    if /^a_/ =~ s
      puts $'
    elsif /^x_([a-z0-9]+)$/ =~ s
      puts [$1].pack('H*')
    else
      puts "** cannot decode **"
    end
  end
end
