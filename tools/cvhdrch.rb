#!/usr/bin/ruby -Ku
# -*- coding: utf-8-unix -*-

# cygwin1.7.1のiconv.hなど、gccで --input-charset=CP932 とかつけると
# コンパイルできなくなるヘッダを何とかするためのツール。
# マルチバイト文字を \uxxxx 表記に変換する。

# -l : マルチバイト文字の一覧表示
# -c : マルチバイト文字を \uxxxx に変換したものを標準出力に出力

# 使用例:  cvhdrch.rb -c /usr/include/iconv.h > mydir/iconv.h


require 'kconv'

mode = :list
if ARGV[0] == '-l'
  ARGV.shift
elsif ARGV[0] == '-c'
  mode = :conv
  ARGV.shift
end


if mode == :list
  while line = ARGF.gets
    line.split('').each do |c|
      if c.size > 1
        printf "%s  (\\u%s)\n", c, c.toutf16.unpack('H*')[0]
      end
    end
  end
elsif mode == :conv
  while line = ARGF.gets
    line.split('').each do |c|
      if c.size > 1
        print "\\u" + c.toutf16.unpack('H*')[0]
      else
        print c
      end
    end
  end
end
