#include "stdafx.h"
#include "kyuubi_vm_types.h"

using namespace std;
using namespace xiyue;
using namespace kyuubi;

const wchar_t* kyuubi::VmDirective_toString(VmDirective d)
{
	switch (d)
	{
	case NOOP:
		return L"NOOP";
	case PRNT:
		return L"PRNT";
	case TVAN:
		return L"TVAN";
	case POPV:
		return L"POPV";
	case TVAI:
		return L"TVAI";
	case PRVA:
		return L"PRVA";
	case LODS:
		return L"LODS";
	case LODI:
		return L"LODI";
	case LODA:
		return L"LODA";
	case LODF:
		return L"LODF";
	case LODB:
		return L"LODB";
	case ADDT:
		return L"ADDT";
	case SUBT:
		return L"SUBT";
	case MULT:
		return L"MULT";
	case DIVT:
		return L"DIVT";
	case NOTT:
		return L"NOTT";
	case ANDT:
		return L"ANDT";
	case ORST:
		return L"ORST";
	case GOZF:
		return L"GOZF";
	case GOTO:
		return L"GOTO";
	case SAVE:
		return L"SAVE";
	case LODV:
		return L"LODV";
	case CKNF:
		return L"CKNF";
	case FORS:
		return L"FORS";
	case LDFI:
		return L"LDFI";
	case LDFK:
		return L"LDFK";
	case LDFV:
		return L"LDFV";
	case FILT:
		return L"FILT";
	case SFLT:
		return L"SFLT";
	case EFLT:
		return L"EFLT";
	case SBLK:
		return L"SBLK";
	case EBLK:
		return L"EBLK";
	case NEXT:
		return L"NEXT";
	case SUCC:
		return L"SUCC";
	default:
		return L"NOOP";
	}
}
