#!/usr/bin/ruby -Ks

# copyright 2010 FUKUZAWA Tadashi

# 指定ディレクトリにある、FFIの *.h, *.cpp ファイルを
# 収集して、Makefile用 FFI_SRC_FILE を出力する。

# collectFfi.rb [options...] ディレクトリ...
# options:
#   -C workdir                    # 収集前に workdir に chdir する
#   -o FFI_SRC_FILE           # 出力ファイル指定：指定が無ければ標準出力
#   -V FFI_SRCS               # Makefile中の変数名


require 'fileutils'
require 'tempfile'


def expandPath (f, d = nil)
  File.expand_path(f, d).sub(/\/cygdrive\/(.)\//, '\\1:/')
end

def diffWrite (file)
  begin
    tmpFile = Tempfile.open(File.basename(file), File.dirname(file))
    yield tmpFile
    tmpFile.flush
    FileUtils.install(tmpFile.path, file)
  ensure
    tmpFile.close(true)
  end
end


def outSrcs(io, varName, srcs)
  io.puts "#{varName} = \\"
  srcs.each{|f| io.puts "\t#{f} \\"}
  io.puts ''
  io.puts ''
end




workDir = '.'
outFile = nil
varName = 'FFI_SRCS'

while /^-/ =~ ARGV[0]
  opt = ARGV.shift
  case opt
  when '-C'
    workDir = ARGV.shift
  when '-o'
    outFile = ARGV.shift
  when '-V'
    varName = ARGV.shift
  else
    puts "unknown option #{opt}"
    exit 1
  end
end


srcs = []
Dir.chdir(workDir) do
  while ARGV.size > 0
    dir = ARGV.shift
    Dir.foreach(dir) do |f|
      if /^(HSft_.*_tbl|_?HSc._.*)\.cpp$/ =~ File.basename(f)
        srcs.push expandPath(f, dir)
      end
    end
  end
end


if outFile
  File.open(outFile, 'w') do |io|
    outSrcs(io, varName, srcs)
  end
else
  outSrcs(STDOUT, varName, srcs)
end

