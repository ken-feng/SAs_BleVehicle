//********************************************************************************
//
//********************************************************************************
//Modify (Ken):VEHICLE-V0C02 NO.1 -20231218
#if defined __FIT_Aeon_H
typedef union{
	unsigned char Byte;
	struct{
		unsigned char InRange:1;
		unsigned char StartReport:1;
		unsigned char reserved:6;
	}bits;
}KeylessStates;
#endif


//********************************************************************************
//
//********************************************************************************
//#ifdef _RamDefine_H
//
//#else
//
//#endif
