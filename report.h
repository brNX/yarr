#ifndef REPORT_H_
#define REPORT_H_

#ifndef uchar
#define uchar   unsigned char
#endif

typedef struct {
	char	reportid;	// 0
	char	x;			// 1
	char	y;			// 2
	char	rx;			// 3
	char	ry;			// 4
	uchar	hat;		// 5
	uchar	b1;			// 6
	uchar	b2;			// 7
} report_t;

typedef struct{
	char	reportid;
    uchar   b1;
    char    x;
    char    y;
    char    w;
}reportMouse_t;

#define START_PRESSED(REPORT) 	((REPORT.b2 & (1<<0)) == (1<<0))
#define Z_PRESSED(REPORT) 		((REPORT.b2 & (1<<1)) == (1<<1))
#define A_PRESSED(REPORT) 		((REPORT.b1 & (1<<0)) == (1<<0))
#define B_PRESSED(REPORT) 		((REPORT.b1 & (1<<1)) == (1<<1))
#define X_PRESSED(REPORT) 		((REPORT.b1 & (1<<3)) == (1<<3))
#define Y_PRESSED(REPORT) 		((REPORT.b1 & (1<<4)) == (1<<4))
#define L_PRESSED(REPORT)		((REPORT.b1 & (1<<6)) == (1<<6))
#define R_PRESSED(REPORT)		((REPORT.b1 & (1<<7)) == (1<<7))


//digital pad
#define UP_PRESSED(REPORT)		(REPORT.hat==0)
#define DOWN_PRESSED(REPORT)	(REPORT.hat==4)
#define LEFT_PRESSED(REPORT)	(REPORT.hat==6)
#define RIGHT_PRESSED(REPORT)	(REPORT.hat==2)
#define URIGHT_PRESSED(REPORT)	(REPORT.hat==1)
#define DRIGHT_PRESSED(REPORT)	(REPORT.hat==3)
#define ULEFT_PRESSED(REPORT)	(REPORT.hat==7)
#define DLEFT_PRESSED(REPORT)	(REPORT.hat==5)

#endif
