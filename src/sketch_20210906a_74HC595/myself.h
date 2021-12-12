#ifndef __MYSELF_H__
#define __MYSELF_H__

// home/hohno46466/GitHub/cZone--Arduino--ArrowLight/src/sketch_20210906a_74HC595//myself.h

// First version: Mon Sep 6 00:00:00 JST 2021
// Last update:

// ---------------------------------------------------------

#define PNAME   "Arduino/2021/sketch_20210906a_74HC595"
#define PVER    "2021-09-06-A"
#define PMAGIC  "67feb5"
#define PMEMO   "(none)"

// Note: The PMAGIC is a magic number, which can be calculated using the following command line with "PNAME" and "PVER".
// echo -n "$PNAME $PVER" | sha256deep | fold -6 | head -1

//  Note: echo -n  "Arduino/2021/sketch_20210906a_74HC595 2021-09-06-A" | sha256sum | fold -6 | head -1
//
void showOpeningMessage();

// ---------------------------------------------------------

#endif // __MYSELF_H__
