/*
 * Copyright (c) 1981, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef lint
//static char sccsid[] = "@(#)unctrl.c	8.1 (Berkeley) 6/4/93";
#endif /* not lint */

char *__unctrl[256] = {
	"^@",  "^A",  "^B",  "^C",  "^D",  "^E",  "^F",  "^G",
	"^H",  "^I",  "^J",  "^K",  "^L",  "^M",  "^N",  "^O",
	"^P",  "^Q",  "^R",  "^S",  "^T",  "^U",  "^V",  "^W",
	"^X",  "^Y",  "^Z",  "^[", "^\\",  "^]",  "^~",  "^_",
	 " ",   "!",  "\"",   "#",   "$",   "%",   "&",   "'",
	 "(",   ")",   "*",   "+",   ",",   "-",   ".",   "/",
	 "0",   "1",   "2",   "3",   "4",   "5",   "6",   "7",
	 "8",   "9",   ":",   ";",   "<",   "=",   ">",   "?",
	 "@",   "A",   "B",   "C",   "D",   "E",   "F",   "G",
	 "H",   "I",   "J",   "K",   "L",   "M",   "N",   "O",
	 "P",   "Q",   "R",   "S",   "T",   "U",   "V",   "W",
	 "X",   "Y",   "Z",   "[",  "\\",   "]",   "^",   "_",
	 "`",   "a",   "b",   "c",   "d",   "e",   "f",   "g",
	 "h",   "i",   "j",   "k",   "l",   "m",   "n",   "o",
	 "p",   "q",   "r",   "s",   "t",   "u",   "v",   "w",
	 "x",   "y",   "z",   "{",   "|",   "}",   "~",   "^?",

	"0x80", "0x81",	"0x82", "0x83", "0x84", "0x85", "0x86", "0x87",
	"0x88", "0x89",	"0x8a", "0x8b", "0x8c", "0x8d", "0x8e", "0x8f",
	"0x90", "0x91",	"0x92", "0x93", "0x94", "0x95", "0x96", "0x97",
	"0x98", "0x99",	"0x9a", "0x9b", "0x9c", "0x9d", "0x9e", "0x9f",
	"0xa0", "0xa1",	"0xa2", "0xa3", "0xa4", "0xa5", "0xa6", "0xa7",
	"0xa8", "0xa9",	"0xaa", "0xab", "0xac", "0xad", "0xae", "0xaf",
	"0xb0", "0xb1",	"0xb2", "0xb3", "0xb4", "0xb5", "0xb6", "0xb7",
	"0xb8", "0xb9",	"0xba", "0xbb", "0xbc", "0xbd", "0xbe", "0xbf",
	"0xc0", "0xc1",	"0xc2", "0xc3", "0xc4", "0xc5", "0xc6", "0xc7",
	"0xc8", "0xc9",	"0xca", "0xcb", "0xcc", "0xcd", "0xce", "0xcf",
	"0xd0", "0xd1",	"0xd2", "0xd3", "0xd4", "0xd5", "0xd6", "0xd7",
	"0xd8", "0xd9",	"0xda", "0xdb", "0xdc", "0xdd", "0xde", "0xdf",
	"0xe0", "0xe1",	"0xe2", "0xe3", "0xe4", "0xe5", "0xe6", "0xe7",
	"0xe8", "0xe9",	"0xea", "0xeb", "0xec", "0xed", "0xee", "0xef",
	"0xf0", "0xf1",	"0xf2", "0xf3", "0xf4", "0xf5", "0xf6", "0xf7",
	"0xf8", "0xf9",	"0xfa", "0xfb", "0xfc", "0xfd", "0xfe", "0xff",
};

char __unctrllen[256] = {
	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 2, 
	4, 4, 4, 4, 4, 4, 4, 4,
	4, 4, 4, 4, 4, 4, 4, 4,
	4, 4, 4, 4, 4, 4, 4, 4,
	4, 4, 4, 4, 4, 4, 4, 4,
	4, 4, 4, 4, 4, 4, 4, 4,
	4, 4, 4, 4, 4, 4, 4, 4,
	4, 4, 4, 4, 4, 4, 4, 4,
	4, 4, 4, 4, 4, 4, 4, 4,
	4, 4, 4, 4, 4, 4, 4, 4,
	4, 4, 4, 4, 4, 4, 4, 4,
	4, 4, 4, 4, 4, 4, 4, 4,
	4, 4, 4, 4, 4, 4, 4, 4,
	4, 4, 4, 4, 4, 4, 4, 4,
	4, 4, 4, 4, 4, 4, 4, 4,
	4, 4, 4, 4, 4, 4, 4, 4,
	4, 4, 4, 4, 4, 4, 4, 4,
};
