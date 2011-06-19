#!/usr/bin/ruby

require 'tempfile'
require 'fileutils'

machdepDir = ARGV[0]
compilerMachdepDir = ARGV[1] || "machdep/unix"


unless FileTest.directory?(machdepDir)
  puts "#{machdepDir} is not a directory"
  exit 1
end
['machdep.h', 'machdep.cpp', 'Makefile', 'mbcsconv.h', 'mbcsconv.cpp'].each do |f|
  unless FileTest.exist?(File.expand_path(f, compilerMachdepDir))
    puts "#{machdepDir} does not have necessary file: #{f}"
    exit 1
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


diffWrite("./compiler/MACHDEP") do |out|
  out.puts "MACHDEP_DIR = ../#{compilerMachdepDir}"
end
diffWrite("./engine/MACHDEP") do |out|
  out.puts "MACHDEP_DIR = ../#{machdepDir}"
end

