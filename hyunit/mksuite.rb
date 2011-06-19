#!/usr/bin/ruby

# copyright 2010 FUKUZAWA Tadashi

$LOAD_PATH.push File.dirname(__FILE__)
# $LOAD_PATH.push(File.dirname(__FILE__) + "/../compiler_rb")

require 'tempfile'
require 'fileutils'

outDir = 'out'

def outSuite (fname, klass, methods, io)
  return if methods.size == 0
  io.puts "require \"#{fname.sub('.hy','')}\""
  methods.each do |m|
    io.puts "_hy_test(#{klass}, :#{m})"
  end
end


def diffWrite(file)
  begin
    tmpFile = Tempfile.open(File.basename(file), File.dirname(file))
    yield tmpFile
    tmpFile.flush
    FileUtils.install(tmpFile.path, file)
  ensure
    tmpFile.close(true)
  end
end


$loadPath = ['.']

def load(f)
  $loadPath.each do |lp|
    path = lp + '/' + f
    fh = File.open(path, 'r')
    if fh
      begin
        yield fh
      ensure
        fh.close
      end
      return
    end
  end
  STDERR.puts "file #{f} not found in loadpath [#{$loadPath.join(', ')}]"
end


diffWrite('hy_suite.hy') do |out|
  out.puts "// this file is automatically created by #{$0}"
  out.puts "require \"testCount\""
  ARGV.each do |arg|
    if /^-I/ =~ arg
      $loadPath.push arg[2..-1]
      next
    end
    load(arg) do |inp|
      klass = nil
      methods = []
      inp.each_line do |line|
        if /^\s*class\s+(HyTest_\w+)\s*<.*HyUnit/ =~ line
          outSuite(arg, klass, methods, out) if (klass)
          klass = $1
          methods = []
        elsif /^\s*def\s+(hytest_\w+)\s*(\(\s*\)\s*)?(\{.*)?$/ =~ line
          methods.push $1
        end
      end
      outSuite(arg, klass, methods, out) if (klass)
    end
  end
  out.puts "_hy_result()"
end
