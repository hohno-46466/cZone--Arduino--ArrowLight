#ifndef __COMMON_MYSELF_H__
#define __COMMON_MYSELF_H__

// myself.h

// First version:
// Last update:

// ---------------------------------------------------------

#define PNAME   "Arduino/20YY/sketch_20YYMMDDa_PPPP_QQQQ"
#define PVER    "20YY-MM-DD-A"
#define PMAGIC  "000000"
#define PMEMO   "(none)"

// Note: The PMAGIC is a magic number, which can be calculated using the following command line with "PNAME" and "PVER".
// echo -n "$PNAME $PVER" | sha256deep | fold -6 | head -1

void showOpeningMessage();

// ---------------------------------------------------------

#endif // __COMMON_MYSELF_H__
