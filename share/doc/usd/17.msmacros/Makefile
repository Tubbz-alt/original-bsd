#	@(#)Makefile	8.1 (Berkeley) 08/14/93

DIR=	usd/17.msmacros
SRCS=	ms
MACROS=	-msU

paper.ps: ${SRCS}
	${SOELIM} ${SRCS} | ${PIC} | ${TBL} | ${ROFF} > ${.TARGET}

.include <bsd.doc.mk>
