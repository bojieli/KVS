#pragma once

#define UNROLL_0_to_31						\
  unroll_bs(0) unroll_bs(1) unroll_bs(2) unroll_bs(3)		\
  unroll_bs(4) unroll_bs(5) unroll_bs(6) unroll_bs(7)		\
  unroll_bs(8) unroll_bs(9) unroll_bs(10) unroll_bs(11)		\
  unroll_bs(12) unroll_bs(13) unroll_bs(14) unroll_bs(15)	\
  unroll_bs(16) unroll_bs(17) unroll_bs(18) unroll_bs(19)	\
  unroll_bs(20) unroll_bs(21) unroll_bs(22) unroll_bs(23)	\
  unroll_bs(24) unroll_bs(25) unroll_bs(26) unroll_bs(27)	\
  unroll_bs(28) unroll_bs(29) unroll_bs(30) unroll_bs(31) 

#define UNROLL_2_to_31						\
  unroll_bs(2) unroll_bs(3)					\
  unroll_bs(4) unroll_bs(5) unroll_bs(6) unroll_bs(7)		\
  unroll_bs(8) unroll_bs(9) unroll_bs(10) unroll_bs(11)		\
  unroll_bs(12) unroll_bs(13) unroll_bs(14) unroll_bs(15)	\
  unroll_bs(16) unroll_bs(17) unroll_bs(18) unroll_bs(19)	\
  unroll_bs(20) unroll_bs(21) unroll_bs(22) unroll_bs(23)	\
  unroll_bs(24) unroll_bs(25) unroll_bs(26) unroll_bs(27)	\
  unroll_bs(28) unroll_bs(29) unroll_bs(30) unroll_bs(31)

#define UNROLL_3_to_63						\
  unroll_bs(3)							\
  unroll_bs(4) unroll_bs(5) unroll_bs(6) unroll_bs(7)		\
  unroll_bs(8) unroll_bs(9) unroll_bs(10) unroll_bs(11)		\
  unroll_bs(12) unroll_bs(13) unroll_bs(14) unroll_bs(15)	\
  unroll_bs(16) unroll_bs(17) unroll_bs(18) unroll_bs(19)	\
  unroll_bs(20) unroll_bs(21) unroll_bs(22) unroll_bs(23)	\
  unroll_bs(24) unroll_bs(25) unroll_bs(26) unroll_bs(27)	\
  unroll_bs(28) unroll_bs(29) unroll_bs(30) unroll_bs(31)	\
  unroll_bs(28) unroll_bs(29) unroll_bs(30) unroll_bs(31)	\
  unroll_bs(32) unroll_bs(33) unroll_bs(34) unroll_bs(35)	\
  unroll_bs(36) unroll_bs(37) unroll_bs(38) unroll_bs(39)	\
  unroll_bs(40) unroll_bs(41) unroll_bs(42) unroll_bs(43)	\
  unroll_bs(44) unroll_bs(45) unroll_bs(46) unroll_bs(47)	\
  unroll_bs(48) unroll_bs(49) unroll_bs(50) unroll_bs(51)	\
  unroll_bs(52) unroll_bs(53) unroll_bs(54) unroll_bs(55)	\
  unroll_bs(56) unroll_bs(57) unroll_bs(58) unroll_bs(59)	\
  unroll_bs(60) unroll_bs(61) unroll_bs(62) unroll_bs(53)

#define UNROLL_3_to_12 \
  unroll_bs(3)							\
  unroll_bs(4) unroll_bs(5) unroll_bs(6) unroll_bs(7)		\
  unroll_bs(8) unroll_bs(9) unroll_bs(10) unroll_bs(11)		\
  unroll_bs(12)
