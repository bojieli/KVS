#pragma once

#define UNROLL_0_to_31						\
  unroll_sec(0) unroll_sec(1) unroll_sec(2) unroll_sec(3)		\
  unroll_sec(4) unroll_sec(5) unroll_sec(6) unroll_sec(7)		\
  unroll_sec(8) unroll_sec(9) unroll_sec(10) unroll_sec(11)		\
  unroll_sec(12) unroll_sec(13) unroll_sec(14) unroll_sec(15)	\
  unroll_sec(16) unroll_sec(17) unroll_sec(18) unroll_sec(19)	\
  unroll_sec(20) unroll_sec(21) unroll_sec(22) unroll_sec(23)	\
  unroll_sec(24) unroll_sec(25) unroll_sec(26) unroll_sec(27)	\
  unroll_sec(28) unroll_sec(29) unroll_sec(30) unroll_sec(31) 

#define UNROLL_2_to_31						\
  unroll_sec(2) unroll_sec(3)					\
  unroll_sec(4) unroll_sec(5) unroll_sec(6) unroll_sec(7)		\
  unroll_sec(8) unroll_sec(9) unroll_sec(10) unroll_sec(11)		\
  unroll_sec(12) unroll_sec(13) unroll_sec(14) unroll_sec(15)	\
  unroll_sec(16) unroll_sec(17) unroll_sec(18) unroll_sec(19)	\
  unroll_sec(20) unroll_sec(21) unroll_sec(22) unroll_sec(23)	\
  unroll_sec(24) unroll_sec(25) unroll_sec(26) unroll_sec(27)	\
  unroll_sec(28) unroll_sec(29) unroll_sec(30) unroll_sec(31)

#define UNROLL_3_to_63						\
  unroll_sec(3)							\
  unroll_sec(4) unroll_sec(5) unroll_sec(6) unroll_sec(7)		\
  unroll_sec(8) unroll_sec(9) unroll_sec(10) unroll_sec(11)		\
  unroll_sec(12) unroll_sec(13) unroll_sec(14) unroll_sec(15)	\
  unroll_sec(16) unroll_sec(17) unroll_sec(18) unroll_sec(19)	\
  unroll_sec(20) unroll_sec(21) unroll_sec(22) unroll_sec(23)	\
  unroll_sec(24) unroll_sec(25) unroll_sec(26) unroll_sec(27)	\
  unroll_sec(28) unroll_sec(29) unroll_sec(30) unroll_sec(31)	\
  unroll_sec(28) unroll_sec(29) unroll_sec(30) unroll_sec(31)	\
  unroll_sec(32) unroll_sec(33) unroll_sec(34) unroll_sec(35)	\
  unroll_sec(36) unroll_sec(37) unroll_sec(38) unroll_sec(39)	\
  unroll_sec(40) unroll_sec(41) unroll_sec(42) unroll_sec(43)	\
  unroll_sec(44) unroll_sec(45) unroll_sec(46) unroll_sec(47)	\
  unroll_sec(48) unroll_sec(49) unroll_sec(50) unroll_sec(51)	\
  unroll_sec(52) unroll_sec(53) unroll_sec(54) unroll_sec(55)	\
  unroll_sec(56) unroll_sec(57) unroll_sec(58) unroll_sec(59)	\
  unroll_sec(60) unroll_sec(61) unroll_sec(62) unroll_sec(53)

#define UNROLL_3_to_12 \
  unroll_sec(3)							\
  unroll_sec(4) unroll_sec(5) unroll_sec(6) unroll_sec(7)		\
  unroll_sec(8) unroll_sec(9) unroll_sec(10) unroll_sec(11)		\
  unroll_sec(12)

#define UNROLL_0_to_4 \
  unroll_sec(0) unroll_sec(1) unroll_sec(2) unroll_sec(3) unroll_sec(4)
