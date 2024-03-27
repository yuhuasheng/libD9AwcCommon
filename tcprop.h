#ifndef _TCPROP_H_
#define _TCPROP_H_
namespace  ITEM_ATTR {
	extern const char* item_id;
	extern const char* current_revision_id;
	extern const char* item_revision_id;
	extern const char* release_status_list;
	extern const char* d9_EnglishDescription;
	extern const char* d9_DescriptionSAP;
	extern const char* d9_ManufacturerID;
	extern const char* d9_ManufacturerPN;
	extern const char* d9_MaterialGroup;
	extern const char* d9_MaterialType;
	extern const char* d9_Un;
	extern const char* d9_ProcurementMethods;
	extern const char* d9_TempPN;
	extern const char* d9_SAPRev;
	extern const char* d9_SupplierZF;
	extern const char* object_name;
	extern const char* d9_HasSourceBOM_REL;
	extern const char* d9_BU;
}

namespace  BOM_ATTR {
	extern const char* d9_Location;
	extern const char* sequence_no;
	extern const char* ref_designator;
	extern const char* quantity;
	extern const char* d9_PackageType;
	extern const char* d9_Side;
	extern const char* d9_ReferenceDimension;
	extern const char* line_object;
	extern const char* d9_AltGroup;
	extern const char* indented_title;
	extern const char* bl_item_item_id;
	extern const char* fnd0objectId;
}

namespace MNTEBOM_ATTR {
	extern const char* IsSecondSource;
	extern const char* MainSource;
	extern const char* ParentRevUid;
	extern const char* ParentItem;
	extern const char* BomId;
	extern const char* BomLineUId;
	extern const char* itemRevUid;
	extern const char* childs;
	extern const char* secondSource;
	extern const char* isJumpLine;
	extern const char* alternativeCode;
}

//namespace MNTEBOM_2ndSource_ATTR_NAME {
//	extern const char* findNum;
//	extern const char* item;
//	extern const char* parentItem;
//	extern const char* version;
//	extern const char* description;
//	extern const char* sapDescription;
//	extern const char* mfg;
//	extern const char* mfgPn;
//	extern const char* location;
//	extern const char* alternativeGroup;
//	extern const char* alternativeCode;
//	extern const char* usageProb;
//	extern const char* materialGroup;
//	extern const char* materialType;
//	extern const char* procurementType;
//	extern const char* qty;
//	extern const char* unit;
//	extern const char* status;
//	extern const char* referenceDimension;
//	extern const char* sapRev;
//	extern const char* packageType;
//	extern const char* side;
//	extern const char* supplierZF;
//	extern const char* isSub;
//	extern const char* checkStates;
//	extern const char* sourceSystem;
//	extern const char* itemRevUid;
//	extern const char* bomLineUid;
//	extern const char* verNote;
//	extern const char* subExistBom;
//	extern const char* materialGroupItemId;
//	extern const char* syncCheckFlag;
//	extern const char* enable;
//	extern const char* childs;
//	extern const char* substitutesList;
//}

namespace PROJECT {
	extern const char* project_list;
}

enum BOMTYPE {
	MNT_L10,
	MNT_L6,
};

enum BUEnum {
	monitor,
	mnt,
	printer,
	prt
};

enum Level {
	L6,
	L10
};
#endif

