#include "tcprop.h"

namespace  ITEM_ATTR {
	extern const char* item_id = "item_id";
	extern const char* current_revision_id = "current_revision_id";
	extern const char* item_revision_id = "item_revision_id";
	extern const char* release_status_list = "release_status_list";
	extern const char* d9_EnglishDescription = "d9_EnglishDescription";
	extern const char* d9_DescriptionSAP = "d9_DescriptionSAP";
	extern const char* d9_ManufacturerID = "d9_ManufacturerID";
	extern const char* d9_ManufacturerPN = "d9_ManufacturerPN";
	extern const char* d9_MaterialGroup = "d9_MaterialGroup";
	extern const char* d9_MaterialType = "d9_MaterialType";
	extern const char* d9_Un = "d9_Un";
	extern const char* d9_ProcurementMethods = "d9_ProcurementMethods";
	extern const char* d9_TempPN = "d9_TempPN";
	extern const char* d9_SAPRev = "d9_SAPRev";
	extern const char* d9_SupplierZF = "d9_SupplierZF";
	extern const char* object_name = "object_name";
	extern const char* d9_HasSourceBOM_REL = "D9_HasSourceBOM_REL";
	extern const char* d9_BU = "d9_BU";
}

namespace  BOM_ATTR {
	extern const char* d9_Location = "bl_occ_d9_Location";
	extern const char* sequence_no = "bl_sequence_no";
	extern const char* ref_designator = "bl_occ_ref_designator";
	extern const char* quantity = "bl_quantity";
	extern const char* d9_PackageType = "bl_occ_d9_PackageType";
	extern const char* d9_Side = "bl_occ_d9_Side";
	extern const char* d9_ReferenceDimension = "bl_occ_d9_ReferenceDimension";
	extern const char* line_object = "bl_line_object";
	extern const char* d9_AltGroup = "bl_occ_d9_AltGroup";
	extern const char* indented_title = "bl_indented_title";
	extern const char* bl_item_item_id = "bl_item_item_id";
	extern const char* fnd0objectId = "bl_occ_fnd0objectId";
}

namespace MNTEBOM_ATTR {
	extern const char* IsSecondSource = "isSecondSource";
	extern const char* MainSource = "mainSource";
	extern const char* ParentRevUid = "parentRevUid";
	extern const char* ParentItem = "parentItem";
	extern const char* BomId = "bomId";
	extern const char* BomLineUId = "bomLineUid";
	extern const char* itemRevUid = "itemRevUid";
	extern const char* childs = "childs";
	extern const char* secondSource = "secondSource";
	extern const char* isJumpLine = "isJumpLine";
	extern const char* alternativeCode = "alternativeCode";
}

namespace MNTEBOM_2ndSource_ATTR_NAME {
	extern const char* findNum = "findNum";
	extern const char* item = "item";
	extern const char* parentItem = "parentItem";
	extern const char* version = "version";
	extern const char* description = "description";
	extern const char* sapDescription = "sapDescription";
	extern const char* mfg = "mfg";
	extern const char* mfgPn = "mfgPn";
	extern const char* location = "location";
	extern const char* alternativeGroup = "alternativeGroup";
	extern const char* alternativeCode = "alternativeCode";
	extern const char* usageProb = "usageProb";
	extern const char* materialGroup = "materialGroup";
	extern const char* materialType = "materialType";
	extern const char* procurementType = "procurementType";
	extern const char* qty = "qty";
	extern const char* unit = "unit";
	extern const char* status = "status";
	extern const char* referenceDimension = "referenceDimension";
	extern const char* sapRev = "sapRev";
	extern const char* packageType = "packageType";
	extern const char* side = "side";
	extern const char* supplierZF = "supplierZF";
	extern const char* isSub = "isSub";
	extern const char* checkStates = "checkStates";
	extern const char* sourceSystem = "sourceSystem";
	extern const char* itemRevUid = "itemRevUid";
	extern const char* bomLineUid = "bomLineUid";
	extern const char* verNote = "verNote";
	extern const char* subExistBom = "subExistBom";
	extern const char* materialGroupItemId = "materialGroupItemId";
	extern const char* syncCheckFlag = "syncCheckFlag";
	extern const char* enable = "enable";
	extern const char* childs = "childs";
	extern const char* substitutesList = "substitutesList";
}


namespace PROJECT {
	extern const char* project_list = "project_list";
}