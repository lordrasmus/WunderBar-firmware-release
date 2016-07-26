#include <ctype.h>


#define DBL_MIN 1.1754945e-38
#define DBL_MAX 3.40282347e+38
#define DBL_DIG 6
#define DBL_MAX_10_EXP 38
#define	NDIG	DBL_DIG+2 	// Number of digits (ANSI 5.2.4.2.2)

//define's for strtod() function
#define	ISNEG	1	// Number is negative
#define	DIGSEEN	2	// We saw at least one digit
#define	EXPNEG	8	// Exponent is negative
#define	DOTSEEN	16	// We have seen a dot


//-------------- Converts	string to number of type <float>
static float strtod(unsigned char * s, unsigned char ** res) {
	unsigned int    flags;
	signed int	expon;
	signed int	eexp;

	union {			              // A union to hold the integer
		float	_l;	            //  component and then the resultant
		long	_v;
	}	_u;

#define	v	_u._v
#define	l	_u._l

	if(res)
		*res = s;
	while(isspace(eexp = *s))
		s++;
	flags = 0;			// Reset flags
	if(eexp == '-') {		// Check for sign
		flags = ISNEG;
		s++;
	}
  else
    if(eexp == '+')
		  s++;
	eexp = 0;			// Clear digit count
	v = 0;				// Clear integer component
	expon = 0;			// Total exponent for integer

	for(;;) {
		if(!(flags & DOTSEEN) && *s == '.') {
			flags |= DOTSEEN;		// If into decimal
			s++;				// set flag
			continue;
		}
		if(!isdigit(*s))			// If end of valid
			break;				// sequence end
		flags |= DIGSEEN;			// else set flag
		if(eexp != NDIG) {
			if(flags & DOTSEEN)		// Count decimal
				expon--;		// places
			eexp++;
			v *= 10;
			__asm ("nop");
			__asm ("nop");
			v += (*s - '0'); // problem u (unsigned char)
			__asm ("nop");
			__asm ("nop");
    } else if(!(flags & DOTSEEN))
			expon++;
		s++;
	}

	eexp = 0;				// Zero users exponent
	if(*s == 'e' || *s == 'E') {		// Look at exponent
		if(*++s == '-') {		// Check sign
			flags |= EXPNEG;
			s++;
		}
    else
      if(*s == '+')
			  s++;
		while(*s == '0')
      s++;		          // Skip leading zeros
		if(isdigit(*s)) {		// Read three digits
			eexp = (unsigned char)(*s++ - '0');   // problem u (unsigned char)
			if(isdigit(*s)) {
				eexp = eexp*10 + (unsigned char)(*s++ - '0');    // problem u (unsigned char)
				if(isdigit(*s))
					eexp = eexp*10 + (unsigned char)(*s - '0');    // problem u (unsigned char)
			}
		}
		if(flags & EXPNEG)
			eexp = -eexp;
	}
	expon += eexp;			// Add user and integer exponents

	if((res!=0) && (flags & DIGSEEN))
        	*res = s;
	l = (float)v;
	if(l == 0.0)			// simply return zero
		return 0.0;
	if(expon < 0) {
		expon = -expon;
#if	DBL_MAX_10_EXP >= 100
		while(expon >= 100) {
			l *= 1e-100;
			expon -= 100;
		}
#endif
		while(expon >= 10) {
			l *= 1e-10;
			expon -= 10;
		}
		while(expon != 0) {
			l *= 1e-1;
			expon--;
		}
		if(l < DBL_MIN)
		{
			if (flags & ISNEG)
				return -DBL_MIN;
			else
				return  DBL_MIN;
		}
	}
  else
    if(expon > 0) {
#if	DBL_MAX_10_EXP >= 100
	    while(expon >= 100) {
			  l *= 1e100;
			  expon -= 100;
		  }
#endif
		  while((unsigned char)expon >= 10) {
			  l *= 1e10;
			  expon -= 10;
		  }
		  while(expon != 0) {
			  l *= 1e1;
			  expon--;
		  }
    if(l > DBL_MAX)
    	{
        if (flags & ISNEG)
          return -DBL_MAX;
        else
          return  DBL_MAX;
    	}
    }
    if (flags & ISNEG)
      return -l;
    else return  l;
}

//-------------- Converts string to float
float atof(char * s) {
	return strtod((unsigned char *) s, (unsigned char **) 0);
}
