//==============================================================================
/*
 Copyright (c) 2009, Isaac Marino Bavaresco
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
     * Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
     * Neither the name of the author nor the
       names of its contributors may be used to endorse or promote products
       derived from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE AUTHOR ''AS IS'' AND ANY
 EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
//==============================================================================
// isaacbavaresco@yahoo.com.br
//==============================================================================
// Vers 1.04	2009/05/07	Almost done, lacks parsing of float values.
//==============================================================================
#include <stdio.h>
#include <ctype.h>
//==============================================================================
#ifndef	_PIC14
	#define	bank1
	#define	bank2
	#define	CLRWDT()
#else
	#include <pic.h>
#endif
//==============================================================================
// These are the data types one may chose from

// One-byte integer values, from 0 to 255 or from -128 to 127
#define	TYPE_CHAR	0
// Two-byte integer values, from 0 to 65535 or from -32768 to 32767
#define	TYPE_SHORT	1
// Four-byte integer values, from 0 to 4294967296 or from -2147483648 to 2147483647
#define	TYPE_LONG	2
// Three-byte floating point values
#define	TYPE_FLOAT	3
// Four-byte floating point values
#define	TYPE_DOUBLE	4
//==============================================================================
//==============================================================================
//==============================================================================
//==============================================================================

// Chose the data type you want to use for the calculations
#define	DATA_TYPE			TYPE_DOUBLE
// How many variables you wanto to use
#define	NUMBER_OF_VARIABLES	(10)
// Depth of the stack for the calculations. More complicated expressions need more levels
#define VAL_STACK_DEPTH		(10)
#define OPER_STACK_DEPTH	(10)

// Chose which groups of operators you want to use ( 0 = disabled, 1 = enabled )
// Note: disabling groups will save program memory.

// Multiplicative operators: '*', '/', '%'
#define	INCLUDE_MULT		1
// Aditive operators: '+', '-'
#define	INCLUDE_ADD			1
// Shift operators: '<<', '>>'
#define	INCLUDE_SHIFT		1
// Binary boolean operators: '&', '^', '|', '~'
#define	INCLUDE_BOOLEAN		1
// Relational operators: '<', '<=', '=', '==', '>=', '>', '<>', '!='
#define	INCLUDE_RELATIONAL	1
// Logical operators: '&&', '||', '!'
#define	INCLUDE_LOGICAL		1
//==============================================================================
//==============================================================================
//==============================================================================
//==============================================================================
#if		DATA_TYPE == TYPE_CHAR
	#define	BASE_TYPE		char
	#define	INT_EQUIV_TYPE	char
#elif	DATA_TYPE == TYPE_SHORT
	#define	BASE_TYPE		short
	#define	INT_EQUIV_TYPE	short
#elif	DATA_TYPE == TYPE_LONG
	#define	BASE_TYPE		long
	#define	INT_EQUIV_TYPE	long
#elif	DATA_TYPE == TYPE_FLOAT
	#define	BASE_TYPE		float
	#define	INT_EQUIV_TYPE	long
#elif	DATA_TYPE == TYPE_DOUBLE
	#define	BASE_TYPE		double
	#define	INT_EQUIV_TYPE	long
#else
	#error "No valid type defined!"
#endif
//==============================================================================
bank1 unsigned short	OPStack[OPER_STACK_DEPTH];
bank1 BASE_TYPE			VLStack[VAL_STACK_DEPTH];

#if			NUMBER_OF_VARIABLES > 0
// Variables from 'a' to 'j' with some arbitrary values for testing purposes...
bank2 BASE_TYPE			Vars[NUMBER_OF_VARIABLES]	= { 10, 15, 30, 50, 100, -1, -2, 1000, 1, 0 };
#endif	//	NUMBER_OF_VARIABLES > 0
//==============================================================================
#define	OPCODE(c)		((unsigned char)(c))
#define	PRECEDENCE(c)	((unsigned char)((c)>>8))
#define	PUSH_VAL(v)		(VLStack[VLPtr++]=(v))
#define POP_VAL()		(VLStack[--VLPtr])
#define	PUSH_OP(v)		(OPStack[OPPtr++]=(v))
#define POP_OP()		(OPStack[--OPPtr])
#define	PEEK_OP()		(OPStack[OPPtr-1])
//==============================================================================
#define	TOKEN_INVALID	(0)
#define	TOKEN_VALUE		(1)
#define	TOKEN_OPER		(2)
//==============================================================================
#define	OPER_OPEN		(0x0218)	/* Open parenthesis */
#define	OPER_CLOSE		(0x0317)	/* Close parenthesis */

#define	OPER_CPL		(0x0e16)	/* Unary boolean complement */
#define	OPER_NOT		(0x0e15)	/* Unary logical complement */
#define	OPER_NEG		(0x0e14)	/* Unary negation (change signal) */

#define	OPER_MUL		(0x0d01)	/* Multiplication */
#define	OPER_DIV		(0x0d02)	/* Division */
#define	OPER_MOD		(0x0d03)	/* Modulus (division remainder) */

#define	OPER_ADD		(0x0c04)	/* Addition */
#define	OPER_SUB		(0x0c05)	/* Subtraction */

#define	OPER_LSL		(0x0b06)	/* Logical shift left */
#define	OPER_LSR		(0x0b07)	/* Logical shift right */

#define	OPER_ANDB		(0x0a08)	/* Boolean AND */
#define	OPER_XOR		(0x0909)	/* Boolean XOR */
#define	OPER_ORB		(0x080a)	/* Boolean OR */

#define	OPER_LT			(0x070b)	/* Less than */
#define	OPER_LE			(0x070c)	/* Less or equal */
#define	OPER_GE			(0x070d)	/* Greater or equal */
#define	OPER_GT			(0x070e)	/* Greater than */

#define	OPER_EQ			(0x060f)	/* Equality */
#define	OPER_NE			(0x0610)	/* Inequality */

#define	OPER_ANDL		(0x0511)	/* Logical AND */
#define	OPER_ORL		(0x0412)	/* Logical OR */

#define	OPER_START		(0x0113)	/* Start of expression */
#define	OPER_EOL		(0x0000)	/* End of expression */

//==============================================================================
typedef union
	{
	BASE_TYPE		val;
	unsigned short	op;
	} TS_Token;
//==============================================================================
unsigned char GetToken( const unsigned char **p, TS_Token *v )
	{
	const unsigned char	*q;
	BASE_TYPE			val;
	unsigned char		c;
	unsigned short		op;

	q	= *p;

	while( isspace( c = *q ))
		q++;

	if( c == 0 )
		{
		v->op	= OPER_EOL;
		return TOKEN_OPER;
		}

#if			NUMBER_OF_VARIABLES > 0
	// Not the right place to get the variables values, but makes things easier...
	else if( isalpha( c ))
		{
		c	= tolower( c );
		if( c >= 'a' && c <= 'a' + NUMBER_OF_VARIABLES - 1 )
			{
			q++;
			if( isalnum( *q ))
				return TOKEN_INVALID;
			v->val	= Vars[c-'a'];
			*p		= q;
			return TOKEN_VALUE;
			}
		return TOKEN_INVALID;
		}
#endif	//	NUMBER_OF_VARIABLES > 0

	else if( isdigit( c ))	// TODO: Parse float numbers
		{
		val	= 0;
		while( isdigit( c = *q ))
			{
			val *= 10;
			val	+= c - '0';
			q++;
			}
		v->val	= val;
		*p		= q;
		return TOKEN_VALUE;
		}

	switch( c )
		{
		case '(':
			op	= OPER_OPEN;
			q++;
			break;
		case ')':
			op	= OPER_CLOSE;
			q++;
			break;

#if			INCLUDE_MULT == 1
		case '*':
			op	= OPER_MUL;
			q++;
			break;
		case '/':
			op	= OPER_DIV;
			q++;
			break;
		case '%':
			op	= OPER_MOD;
			q++;
			break;
#endif	//	INCLUDE_MULT == 1

#if			INCLUDE_ADD == 1
		case '+':
			op	= OPER_ADD;
			q++;
			break;
		case '-':
			op	= OPER_SUB;
			q++;
			break;
#endif	//	INCLUDE_ADD == 1

#if			INCLUDE_BOOLEAN == 1 || INCLUDE_LOGICAL == 1
		case '&':
			q++;
	#if			INCLUDE_LOGICAL == 1
			if( *q == '&' )
				{
				op	= OPER_ANDL;
				q++;
				}
			else
	#endif	//	INCLUDE_LOGICAL == 1

	#if			INCLUDE_BOOLEAN == 1
				op	= OPER_ANDB;
	#else	//	INCLUDE_BOOLEAN == 1
				return TOKEN_INVALID;
	#endif	//	INCLUDE_BOOLEAN == 1

			break;

		case '|':
			q++;
	#if			INCLUDE_LOGICAL == 1
			if( *q == '|' )
				{
				op	= OPER_ORL;
				q++;
				}
			else
	#endif	//	INCLUDE_LOGICAL == 1

	#if			INCLUDE_BOOLEAN == 1
				op	= OPER_ORB;
	#else	//	INCLUDE_BOOLEAN == 1
				return TOKEN_INVALID;
	#endif	//	INCLUDE_BOOLEAN == 1

			break;
#endif	//	INCLUDE_BOOLEAN == 1 || INCLUDE_LOGICAL == 1

#if			INCLUDE_BOOLEAN == 1
		case '~':
			op	= OPER_CPL;
			q++;
			break;
		case '^':
			op	= OPER_XOR;
			q++;
			break;
#endif	//	INCLUDE_BOOLEAN == 1

#if			INCLUDE_RELATIONAL == 1 || INCLUDE_SHIFT == 1
		case '<':
			q++;
	#if			INCLUDE_RELATIONAL == 1
			if( *q == '=' )
				{
				op	= OPER_LE;
				q++;
				}
			else if( *q == '>' )
				{
				op	= OPER_NE;
				q++;
				}
			else
	#endif	//	INCLUDE_RELATIONAL == 1

	#if			INCLUDE_SHIFT == 1
			if( *q == '<' )
				{
				op	= OPER_LSL;
				q++;
				}
			else
	#endif	//	INCLUDE_SHIFT == 1

	#if			INCLUDE_RELATIONAL == 1
				op	= OPER_LT;
	#else	//	INCLUDE_RELATIONAL == 1
				return TOKEN_INVALID;
	#endif	//	INCLUDE_RELATIONAL == 1

			break;

		case '>':
			q++;
	#if			INCLUDE_RELATIONAL == 1
			if( *q == '=' )
				{
				op	= OPER_GE;
				q++;
				}
			else
	#endif	//	INCLUDE_RELATIONAL == 1

	#if			INCLUDE_SHIFT == 1
			if( *q == '>' )
				{
				op	= OPER_LSR;
				q++;
				}
			else
	#endif	//	INCLUDE_SHIFT == 1

	#if			INCLUDE_RELATIONAL == 1
				op	= OPER_GT;
	#else	//	INCLUDE_RELATIONAL == 1
				return TOKEN_INVALID;
	#endif	//	INCLUDE_RELATIONAL == 1

			break;
#endif	//	INCLUDE_RELATIONAL == 1 || INCLUDE_SHIFT == 1

#if			INCLUDE_RELATIONAL == 1
		case '=':
			q++;
			if( *q == '=' )
				q++;
			op	= OPER_EQ;
			break;
#endif	//	INCLUDE_RELATIONAL == 1

#if			INCLUDE_RELATIONAL == 1 || INCLUDE_BOOLEAN == 1
		case '!':
			q++;
	#if			INCLUDE_RELATIONAL == 1
			if( *q == '=' )
				{
				op	= OPER_NE;
				q++;
				}
			else
	#endif	//	INCLUDE_RELATIONAL == 1

	#if			INCLUDE_BOOLEAN == 1
				op	= OPER_NOT;
	#else	//	INCLUDE_BOOLEAN == 1
				return TOKEN_INVALID;
	#endif	//	INCLUDE_BOOLEAN == 1
			break;
#endif	//	INCLUDE_RELATIONAL == 1 || INCLUDE_BOOLEAN == 1

		default:
			return TOKEN_INVALID;
		}
	v->op	= op;
	*p		= q;
	return TOKEN_OPER;
	}
//==============================================================================
unsigned char Eval( const unsigned char *s, BASE_TYPE *result )
	{
	unsigned char	OPPtr		= 0;
	unsigned char	VLPtr		= 0;
	BASE_TYPE		a, b;
	unsigned char	c, t;
#if			DATA_TYPE == TYPE_FLOAT
	INT_EQUIV_TYPE	d, e;
#elif		DATA_TYPE == TYPE_DOUBLE
	INT_EQUIV_TYPE	d;
#endif	//	DATA_TYPE == TYPE_FLOAT
	TS_Token		v;

	VLPtr		= 0;
	OPPtr		= 0;
	PUSH_OP( OPER_START );

	while( 1 )
		{
		do
			{
			t	= GetToken( &s, &v );

			if( t == TOKEN_VALUE )
				{
				// Stack overflow...
				if( VLPtr >= sizeof VLStack / sizeof VLStack[0] )
					return -1;
				PUSH_VAL( v.val );
				}
			else if( t == TOKEN_OPER )
				{
				// Stack overflow...
				if( OPPtr >= sizeof OPStack / sizeof OPStack[0] )
					return -1;
				if( v.op == OPER_OPEN )
					PUSH_OP( OPER_OPEN );
				else
#if			INCLUDE_ADD == 1
				// Unary minus
				if( v.op == OPER_SUB )
					PUSH_OP( OPER_NEG );
				// Unary plus
				else if( v.op == OPER_ADD )
					{}	// Do nothing
				else
#endif	//	INCLUDE_ADD == 1

#if			INCLUDE_LOGICAL == 1 || INCLUDE_BOOLEAN == 1
	#if			INCLUDE_LOGICAL != 1
				if( v.op == OPER_CPL )
	#elif		INCLUDE_BOOLEAN != 1
				if( v.op == OPER_NOT )
	#else	//	INCLUDE_BOOLEAN != 1
				if( v.op == OPER_NOT || v.op == OPER_CPL )
	#endif	//	INCLUDE_LOGICAL != 1
					PUSH_OP( v.op );
				else
#endif	//	INCLUDE_LOGICAL == 1 || INCLUDE_BOOLEAN == 1
					return -1;
				}
			else
				return -1;
			}
		while( t != TOKEN_VALUE );
NextOper:
		if( GetToken( &s, &v ) != TOKEN_OPER )
			return -1;

		while( PRECEDENCE( v.op ) <= PRECEDENCE( PEEK_OP() ))
			{
			if( v.op == OPER_EOL && VLPtr == 1 && OPPtr == 1 )
				{
				*result	= POP_VAL();
				return 0;
				}

			// Stack underflow...
			if( VLPtr < 1 || OPPtr < 2 )
				return -1;

#if			INCLUDE_ADD == 1
			if( OPCODE( PEEK_OP() ) == OPCODE( OPER_NEG ))
				{
				POP_OP();
				b	= -POP_VAL();
				}
			else
#endif	//	INCLUDE_ADD == 1

#if			INCLUDE_LOGICAL == 1
			if( OPCODE( PEEK_OP() ) == OPCODE( OPER_NOT ))
				{
				POP_OP();
				b	= !POP_VAL();
				}
			else
#endif	//	INCLUDE_LOGICAL == 1

#if			INCLUDE_BOOLEAN == 1
			if( OPCODE( PEEK_OP() ) == OPCODE( OPER_CPL ))
				{
				POP_OP();
				b	= ~(INT_EQUIV_TYPE)POP_VAL();
				}
			else
#endif	//	INCLUDE_BOOLEAN == 1

				{
				// Stack underflow...
				if( VLPtr < 2 )
					return -1;
				a	= POP_VAL();
				b	= POP_VAL();
				switch( OPCODE( POP_OP() ))
					{
#if			INCLUDE_MULT == 1
					case OPCODE( OPER_MUL ):
						b	*= a;
						break;
					case OPCODE( OPER_DIV ):
						b	/= a;
						break;
#endif	//	INCLUDE_MULT == 1

#if			INCLUDE_ADD == 1
					case OPCODE( OPER_ADD ):
						b	+= a;
						break;
					case OPCODE( OPER_SUB ):
						b	-= a;
						break;
#endif	//	INCLUDE_ADD == 1

#if			DATA_TYPE == TYPE_DOUBLE

	#if			INCLUDE_MULT == 1
					case OPCODE( OPER_MOD ):
						d	= (INT_EQUIV_TYPE)b % (INT_EQUIV_TYPE)a;
						b	= d;
						break;
	#endif	//	INCLUDE_MULT == 1

	#if			INCLUDE_SHIFT == 1
					case OPCODE( OPER_LSL ):
						c	= (char)a;	// Must split this operation or PICC can't generate code
						b	= (INT_EQUIV_TYPE)b << c;
						break;
					case OPCODE( OPER_LSR ):
						c	= (char)a;	// Must split this operation or PICC can't generate code
						b	= (unsigned INT_EQUIV_TYPE)b >> c;
						break;
	#endif	//	INCLUDE_SHIFT == 1

	#if			INCLUDE_BOOLEAN == 1
					case OPCODE( OPER_ANDB ):
						b	= (INT_EQUIV_TYPE)b & (INT_EQUIV_TYPE)a;
						break;
					case OPCODE( OPER_XOR ):
						b	= (INT_EQUIV_TYPE)b ^ (INT_EQUIV_TYPE)a;
						break;
					case OPCODE( OPER_ORB ):
						b	= (INT_EQUIV_TYPE)b | (INT_EQUIV_TYPE)a;
						break;
	#endif	//	INCLUDE_BOOLEAN == 1

#elif		DATA_TYPE == TYPE_FLOAT

	#if			INCLUDE_MULT == 1
					case OPCODE( OPER_MOD ):
						d	= (INT_EQUIV_TYPE)a;
						d	= (INT_EQUIV_TYPE)b & d;
						b	= d;
						break;
	#endif	//	INCLUDE_MULT == 1

	#if			INCLUDE_SHIFT == 1
					case OPCODE( OPER_LSL ):
						c	= (char)a;
						d	= (INT_EQUIV_TYPE)b;	// Must split this operation or PICC can't generate code
						d	= d << c;
						b	= d;
						break;
					case OPCODE( OPER_LSR ):
						c	= (char)a;
						d	= (unsigned INT_EQUIV_TYPE)b;	// Must split this operation or PICC can't generate code
						b	= d >> c;
						break;
	#endif	//	INCLUDE_SHIFT == 1

	#if			INCLUDE_BOOLEAN == 1
					case OPCODE( OPER_ANDB ):
						d	= (INT_EQUIV_TYPE)b;
						b	= d & (INT_EQUIV_TYPE)a;
						break;
					case OPCODE( OPER_XOR ):
						d	= (INT_EQUIV_TYPE)b;
						b	= d ^ (INT_EQUIV_TYPE)a;
						break;
					case OPCODE( OPER_ORB ):
						d	= (INT_EQUIV_TYPE)b;
						b	= d | (INT_EQUIV_TYPE)a;
						break;
	#endif	//	INCLUDE_BOOLEAN == 1

#else	//	DATA_TYPE == TYPE_FLOAT

	#if			INCLUDE_MULT == 1
					case OPCODE( OPER_MOD ):
						b	= b % a;
						break;
	#endif	//	INCLUDE_MULT == 1

	#if			INCLUDE_SHIFT == 1
					case OPCODE( OPER_LSL ):
						b	= b << a;
						break;
					case OPCODE( OPER_LSR ):
						b	= (unsigned INT_EQUIV_TYPE)b >> a;
						break;
	#endif	//	INCLUDE_SHIFT == 1

	#if			INCLUDE_BOOLEAN == 1
					case OPCODE( OPER_ANDB ):
						b	= b & a;
						break;
					case OPCODE( OPER_XOR ):
						b	= b ^ a;
						break;
					case OPCODE( OPER_ORB ):
						b	= b | a;
						break;
	#endif	//	INCLUDE_BOOLEAN == 1

#endif	//	DATA_TYPE == TYPE_DOUBLE

#if			INCLUDE_RELATIONAL == 1
					case OPCODE( OPER_LT ):
						b	= b < a;
						break;
					case OPCODE( OPER_LE ):
						b	= b <= a;
						break;
					case OPCODE( OPER_GE ):
						b	= b >= a;
						break;
					case OPCODE( OPER_GT ):
						b	= b > a;
						break;
					case OPCODE( OPER_EQ ):
						b	= b == a;
						break;
					case OPCODE( OPER_NE ):
						b	= b != a;
						break;
#endif	//	INCLUDE_RELATIONAL == 1

#if			INCLUDE_LOGICAL == 1
					case OPCODE( OPER_ANDL ):
						b	= a && b;
						break;
					case OPCODE( OPER_ORL ):
						b	= a || b;
						break;
#endif	//	INCLUDE_LOGICAL == 1

					// Something very wrong happened...
					default:
						return -1;
					}
				}
			PUSH_VAL( b );
			}

		// We have found a close parenthesis
		if( OPCODE( v.op ) == OPCODE( OPER_CLOSE ))
			{
			if( OPCODE( PEEK_OP() ) == OPCODE( OPER_OPEN ))
				{
				POP_OP();
				// OK, ugly but the easyest way to handle this.
				goto NextOper;
				}
			else
				return -1;
			}
		else
			{
			if( OPPtr >= sizeof OPStack / sizeof OPStack[0] )
				return -1;
			PUSH_OP( v.op );
			}
		}

	return -1;
	}
//==============================================================================
#ifdef	_PIC14

// The library functions need a 'putch' to work
void putch( char c )
	{
	// Wait until the USART transmits the previous character
	while( !TXIF )
		CLRWDT();
	// Send the character
	TXREG	= c;
	}
/*============================================================================*/
unsigned short TMR1U;

void interrupt ISR( void )
	{
	if( TMR1IE && TMR1IF )
		{
		TMR1IF	= 0;
		TMR1U++;
		}
	else
		{
		GIE	= 0;
		}
	}
/*============================================================================*/
void StartTimer( void )
	{
	TMR1ON	= 0;
	TMR1L	= 0;
	TMR1H	= 0;
/*
	TMR1U	= 0;
*/
	TMR1IF	= 0;
	TMR1IE	= 1;
	TMR1ON	= 1;
	}
/*============================================================================*/
unsigned short ReadTimer( void )
	{
	TMR1ON	= 0;
/*
	return (unsigned long)TMR1U << 16 | TMR1H << 8 | TMR1L;
*/
	return TMR1H << 8 | TMR1L;
	}

#else	// _PIC14

void StartTimer( void )
	{
	}

unsigned long ReadTimer( void )
	{
	return 0;
	}

#endif	// _PIC14
/*============================================================================*/
#if			DATA_TYPE == TYPE_FLOAT || DATA_TYPE == TYPE_DOUBLE
#define	FORMAT	"%20s\t = %12f"
#elif		DATA_TYPE == TYPE_LONG
#define	FORMAT	"%20s\t = %12ld"
#else	//	DATA_TYPE == TYPE_LONG
#define	FORMAT	"%20s\t = %12d"
#endif	//	DATA_TYPE == TYPE_FLOAT || DATA_TYPE == TYPE_DOUBLE

const char * const	Expressions[]	=
	{
	"()",
	"-(1)",
	"(-1)",
	"(((1+2)))",
	"(",
	")",
	"1(",
	"(1",
	")1",
	"1)",
	"(-1)",
	"((1<<2)+3)*3",
	"(1<<2+3)*3",
	"(1+2)*(2+3)",
	"1",
	"A",
	"+1",
	"-1",
	"~0",
	"~-1",
	"!1",
	"!0",
	"3*5",
	"30/5",
	"5%3",
	"3+5",
	"3-5",
	"1<<3",
	"8>>3",
	"65535&255",
	"65535^255",
	"65280|255",
	"1<2 ",
	"2<1",
	"1<=2",
	"1=2",
	"1=1",
	"1==1",
	"1!=1",
	"1!=2",
	"1>=2",
	"2>=1",
	"2>1",
	"2>2",
	"1!=1",
	"1!=2",
	"1<>1",
	"1<>2",
	"1<2&&4>3",
	"1>2&&4>3",
	"1<2&&4<3",
	"1>2&&4<3",
	"1<2||4>3",
	"1>2||4>3",
	"1<2||4<3",
	"1>2||4<3",
	"100+100/3"
	};
/*

	"100 + 100 / c",
	"100 + 100 / 3",
	"A * b + C / d",

	"-A",
	"b * -c",
	"1 +* 2",
	"! 1",
	"! 0",
	"~ 0",
	"~ -1",
	"X"
	};
*/

void main( void )
	{
	unsigned short	t;
	BASE_TYPE		Val;
	char			i, status;

#ifdef	_PIC14
	// Initialize port pins
	CMCON	= 0x07;
	PORTB	= 0x04;
	TRISB	= 0x02;

	// Initialize USART
	SPBRG	= 16;
	TXSTA	= 0x24;
	RCSTA	= 0x90;

	T1CON	= 0x00;
	INTCON	= 0xc0;

	while( 1 )
#endif	//_PIC14

		{
		CLRWDT();
		for( i = 0; i < sizeof Expressions / sizeof Expressions[0]; i++ )
			{
			StartTimer();
			status	= Eval( Expressions[i], &Val );
			t		= ReadTimer();
			if( status )
				{
				printf( "Error: %20s", Expressions[i] );
				printf( "\t(%u tcy)\n", t );
				}
			else
				{
				printf( FORMAT, Expressions[i], Val);
				printf( "\t(%u tcy)\n", t );
				}
			}
		}
	}
//==============================================================================
