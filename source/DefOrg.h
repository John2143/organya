
#define MAXNOTELENGTH	255//Maximum note length
#define MAXMELODY		8
#define MAXDRAM			8
#define MAXTRACK		16

#define KEYDUMMY		255//Musical notes that do not change the key
#define PANDUMMY		255//Do not change the bread note
#define VOLDUMMY		255//Musical notes that do not change the volume
//Flag when setting song information
#define SETALL		0xffffffff//Set all
#define SETWAIT		0x00000001
#define SETGRID		0x00000002
#define SETALLOC	0x00000004
#define SETREPEAT	0x00000008
#define SETFREQ		0x00000010
#define SETWAVE		0x00000020
#define SETPIPI		0x00000040


#define ALLOCNOTE	4096
#define MODEPARADD	0
#define MODEPARSUB	1
#define MODEMULTIPLY 10 //2014.04.30 A
#define MODEDECAY   64  //2014.05.01 A


//#define NUMDRAMITEM		39	//12 + 5 + 9 + 1(Cat)+11	// 2011.10.17 D
#define NUMDRAMITEM		42	//12 + 5 + 9 + 1(Cat)+ 11 + 3	// 2011.10.17 A

