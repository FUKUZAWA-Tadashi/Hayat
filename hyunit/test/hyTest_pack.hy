// -*- coding: utf-8 -*-

require "hyUnit"

// n	big endian unsigned 16bit
// N	big endian 32bit
// v	little endian unsigned 16bit
// V	little endian 32bit
// c 	signed char 8bit
// C	unsigned char 8bit
// s	signed short 16bit
// S	unsigned short 16bit
// i	signed int 32bit
// f	float 32bit
// a	ナルバイト区切り文字列
// A	バイト数区切り文字列 : 最大65535バイトまで
// x	pack:ナルバイト / unpack:1byte読み飛ばし


class HyTest_pack < HyUnit

	def hytest_single ()
	{
		x = [1, 2, 3]
		b = x.pack("nV s")
		test_assert_equal(StringBuffer, b.getClass())
		test_assert_equal(8, b.length())
		y = b.unpack("n Vs")
		test_assert_equal(x, y)

		x = [0x7654, 0x7f8f9faf, 0x1428, 0x71727374,
			0x77, 0xcc, 0x6e5e, 0x5d6d,
			0x6e6d6c6a, 123.675]
		b = x.pack("nNvV cCsS if")
		test_assert_equal(StringBuffer, b.getClass())
		y = b.unpack("n N v V c C s S i f")
		test_assert_equal(x, y)

		x = [0xcdef, -0x7f8f9faf, 0xedcb, -0x71727374,
			-0x77, 0xcc, -0x6e5e, 0x9dad,
			-0x6e6d6c6a, -123.675]
		b = x.pack("nN vV cC sS if")
		test_assert_equal(StringBuffer, b.getClass())
		y = b.unpack("nNvVcCsSif")
		test_assert_equal(x, y)
	}

	def hytest_multi ()
	{
		x = [12345, -12346, 12347,
			78901234, -89012345,
			0x11, -0x22, 0x33, -0x44, 0x55, -0x66]
		b = x.pack("s3 x i2 c*")
		test_assert_equal(StringBuffer, b.getClass())
		y = b.unpack("s3 x i2 c*")
		test_assert_equal(x, y)
		y = b.unpack("s3 c i2 c*")
		test_assert_equal([12345, -12346, 12347,
							  0x00,
							  78901234, -89012345,
							  0x11, -0x22, 0x33, -0x44, 0x55, -0x66],
						  y)
	}

	def hytest_str ()
	{
		x = ["abc", 「def」, "ghi", 「jkl」]
		b = x.pack("a4");
		test_assert_equal(StringBuffer, b.getClass())
		y = b.unpack("aa2a")
		test_assert_equal(["abc", "def", "ghi", "jkl"], y)

		b = x.pack("A2A2");
		test_assert_equal(StringBuffer, b.getClass())
		y = b.unpack("A3A")
		test_assert_equal(["abc", "def", "ghi", "jkl"], y)

		b = [5, "abcdefg"].pack("Sa")
		y = b.unpack("Ac*")
		test_assert_equal(["abcde", 0x66, 0x67, 0x00], y)
	}

end
