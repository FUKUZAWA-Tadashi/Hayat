// -*- coding: utf-8 -*-

require "hyUnit"


class HyTest_StringReader < HyUnit

	def hytest_base ()
	{
		sr = new StringReader("1234 5678 90abc def")
		test_assert_equal(19, sr.length())
		test_assert_equal(0, sr.pos)
		test_assert_equal(0x31, sr.getc())
		test_assert_equal(0x32, sr.getc())
		test_assert_equal(2, sr.pos)
		sr.pos = 5
		test_assert_equal(0x35, sr.getc())
		test_assert_equal("34 5", sr.substr(2,6))
		test_assert(! sr.search(0x78))  // x は無い
		test_assert(sr.search(0x37))
		test_assert_equal(0x37, sr.getc())
		test_assert_equal(8, sr.pos)
		test_assert_equal(8, sr.skip(" ")) // " " ではないのでスキップしない
		test_assert_equal(0x38, sr.getc())
		test_assert_equal(9, sr.pos)
		test_assert_equal(10, sr.skip(" ")) // " " をスキップ
		test_assert_equal(0x39, sr.getc())
		test_assert_equal("0abc d", sr.read(6))
		test_assert_equal("ef", sr.read(6))

		sr = new StringReader(「x=10,y=20」)
// 」)  for emacs hayataro-mode bug
		sr.skip(0x61, 0x7a)
		sr.skip("=")
		pos = sr.pos
		sr.skip(0x30,0x39)
		x = sr.substr(pos, sr.pos)
		sr.skip(0x2c)  // ","
		sr.skip(0x61, 0x7a)
		sr.skip("=")
		pos = sr.pos
		sr.skip(0x30,0x39)
		y = sr.substr(pos, sr.pos)

		test_assert_equal("10", x)
		test_assert_equal("20", y)
	}

	def hytest_getVal ()
	{
		sr = new StringReader("123 -456 78.875 -12.25 20e3 -2.5e+2 12.5e-2")
		test_assert_equal(123, sr.getInt())
		sr.skip(" ")
		test_assert_equal(-456, sr.getInt())
		sr.skip(" ")
		test_assert_equal(78.875, sr.getFloat())
		sr.skip(" ")
		test_assert_equal(-12.25, sr.getFloat())
		sr.skip(" ")
		test_assert_equal(20000.0, sr.getFloat())
		sr.skip(" ")
		test_assert_equal(-250.0, sr.getFloat())
		sr.skip(" ")
		test_assert_equal(0.125, sr.getFloat())

		sr = new StringReader("0x234,-0o345,-0b11010")
		test_assert_equal(0x234, sr.getInt())
		sr.skip(",")
		test_assert_equal(-(3*64+4*8+5), sr.getInt())
		sr.skip(",")
		test_assert_equal(-0x1a, sr.getInt())

		sr = new StringReader("--3---4:::5")
		test_assert_equal(3, sr.getInt())
		sr.skip("-")
		test_assert_equal(4, sr.getInt())
		p = sr.pos
		test_assert_equal(0, sr.getInt())
		test_assert_equal(p, sr.pos)
		sr.skip(":")
		p = sr.pos
		test_assert_equal(5, sr.getInt())
		test_assert(p < sr.pos)
	}

end
