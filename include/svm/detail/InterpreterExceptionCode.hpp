#pragma once

#define SVM_IEC_TYPE_OUTOFRANGE					0x00000000

#define SVM_IEC_STACK_OVERFLOW					0x00000001
#define SVM_IEC_STACK_EMPTY						0x00000002
#define SVM_IEC_STACK_DIFFERENTTYPE				0x00000003

#define SVM_IEC_CONSTANTPOOL_OUTOFRANGE			0x00000004

#define SVM_IEC_ARITHMETIC_DIVIDEBYZERO			0x00000005

#define SVM_IEC_LOCALVARIABLE_OUTOFRANGE		0x00000006
#define SVM_IEC_LOCALVARIABLE_INVALIDINDEX		0x00000007

#define SVM_IEC_LABEL_OUTOFRANGE				0x00000008

#define SVM_IEC_FUNCTION_OUTOFRANGE				0x00000009
#define SVM_IEC_FUNCTION_NORETINSTRUCTION		0x0000000A

#define SVM_IEC_POINTER_NULLPOINTER				0x0000000B
#define SVM_IEC_POINTER_NOTPOINTER				0x0000000C
#define SVM_IEC_POINTER_INVALIDFORPOINTER		0x0000000D
#define SVM_IEC_POINTER_UNKNOWNADDRESS			0x0000000E

#define SVM_IEC_STRUCTURE_FIELD_OUTOFRANGE		0x0000000F
#define SVM_IEC_STRUCTURE_NOTSTRUCTURE			0x00000010
#define SVM_IEC_STRUCTURE_INVALIDFORSTRUCTURE	0x00000011

#define SVM_IEC_ARRAY_LENGTH_CANNOTBEZERO		0x00000012
#define SVM_IEC_ARRAY_INDEX_OUTOFRANGE			0x00000013
#define SVM_IEC_ARRAY_NOTARRAY					0x00000014
#define SVM_IEC_ARRAY_INVALIDFORARRAY			0x00000015