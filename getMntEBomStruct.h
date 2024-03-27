#ifndef D9_mntEbom
#define D9_mntEbom

#include <string>
#include <mutex>
#include <future>
#include <iostream>
#include <stdlib.h>
#include <sa/tcfile.h>
#include <tccore/workspaceobject.h>
#include <ae/ae.h>
#include <user_exits/user_exits.h>
#include <property/prop.h>
#include <sa/tcvolume.h>
#include <tccore/aom.h>
#include <ss/ss_const.h>
#include <stdarg.h>
#include <base_utils/Mem.h>
#include <property/prop.h>
#include <epm/epm.h>
#include <nls/nls.h>
#include <tccore/aom_prop.h>
#include <cfm/cfm.h>
#include <bom/bom.h>
#include "json.hxx"
#include "tcprop.h"
#include "util.h"

using namespace std;
using json = nlohmann::json;


#ifdef __cplusplus
extern "C" {
#endif
	namespace MNTEBOM {
		struct EBOMLineBean {
			string parentItem,
				parentRevUid,
				parentUid,
				itemRevUid,
				findNum,
				item,
				version,
				status,
				description,
				sapDescription,
				mfg,
				mfgPn,
				location,
				qty,
				packageType,
				side,
				alternativeGroup,
				materialGroup,
				materialType,
				unit,
				alternativeCode,
				procurementType,
				referenceDimension,
				tempPN,
				sapRev,
				supplierZF,
				uid,
				mainSource,
				bomId,
				findId,
				userBu;
			bool isSecondSource = false,
				isBOMViewWFTask = false,
				isCanDcn = true,
				canEditPIEE = false,
				showDifference = false,
				isNewVersion = false;
			vector<EBOMLineBean> childs,
				secondSource;
			//NLOHMANN_DEFINE_TYPE_INTRUSIVE
		};

		static void to_json(json& j, const  EBOMLineBean& bomPojo) {
			j = json{
				{"parentItem",bomPojo.parentItem},
				{"parentRevUid",bomPojo.parentRevUid},
				{"parentUid",bomPojo.parentUid},
				{"itemRevUid",bomPojo.itemRevUid},
				{"findNum",bomPojo.findNum},
				{"item",bomPojo.item},
				{"version",bomPojo.version},
				{"status",bomPojo.status},
				{"description",bomPojo.description},
				{"sapDescription",bomPojo.sapDescription},
				{"mfg",bomPojo.mfg},
				{"mfgPn",bomPojo.mfgPn},
				{"location",bomPojo.location},
				{"qty",bomPojo.qty},
				{"packageType",bomPojo.packageType},
				{"side",bomPojo.side},
				{"alternativeGroup",bomPojo.alternativeGroup},
				{"materialGroup",bomPojo.materialGroup},
				{"materialType",bomPojo.materialType},
				{"unit",bomPojo.unit},
				{"alternativeCode",bomPojo.alternativeCode},
				{"procurementType",bomPojo.procurementType},
				{"referenceDimension",bomPojo.referenceDimension},
				{"tempPN",bomPojo.tempPN},
				{"sapRev",bomPojo.sapRev},
				{"supplierZF",bomPojo.supplierZF},
				{"uid",bomPojo.uid},
				{"mainSource",bomPojo.mainSource},
				{"bomId",bomPojo.parentItem + "$" + bomPojo.item + "$" + bomPojo.mainSource },
				{"findId",bomPojo.findId},
				{"userBu",bomPojo.userBu},
				//
				{"isSecondSource",bomPojo.isSecondSource},
				{"isBOMViewWFTask",bomPojo.isBOMViewWFTask},
				{"isCanDcn",bomPojo.isCanDcn},
				{"canEditPIEE",bomPojo.canEditPIEE},
				{"isNewVersion",bomPojo.isNewVersion},
				{"showDifference",bomPojo.showDifference},

				//
				{"secondSource",bomPojo.secondSource},
				{"childs",bomPojo.childs} };
		}


		static void from_json(const json& j, EBOMLineBean& bomPojo) {
			j.at("parentItem").get_to(bomPojo.parentItem);
			j.at("parentUid").get_to(bomPojo.parentUid);
			j.at("itemRevUid").get_to(bomPojo.itemRevUid);
			j.at("findNum").get_to(bomPojo.findNum);
			j.at("item").get_to(bomPojo.item);
			j.at("version").get_to(bomPojo.version);
			j.at("status").get_to(bomPojo.status);
			j.at("description").get_to(bomPojo.description);
			j.at("sapDescription").get_to(bomPojo.sapDescription);
			j.at("mfg").get_to(bomPojo.mfg);
			j.at("mfgPn").get_to(bomPojo.mfgPn);
			j.at("location").get_to(bomPojo.location);
			j.at("qty").get_to(bomPojo.qty);
			j.at("packageType").get_to(bomPojo.packageType);
			j.at("side").get_to(bomPojo.side);
			j.at("alternativeGroup").get_to(bomPojo.alternativeGroup);
			j.at("materialGroup").get_to(bomPojo.materialGroup);
			j.at("materialType").get_to(bomPojo.materialType);
			j.at("unit").get_to(bomPojo.unit);
			j.at("alternativeCode").get_to(bomPojo.alternativeCode);
			j.at("procurementType").get_to(bomPojo.procurementType);
			j.at("referenceDimension").get_to(bomPojo.referenceDimension);
			j.at("tempPN").get_to(bomPojo.tempPN);
			j.at("sapRev").get_to(bomPojo.sapRev);
			j.at("supplierZF").get_to(bomPojo.supplierZF);
			j.at("uid").get_to(bomPojo.uid);
			j.at("mainSource").get_to(bomPojo.mainSource);
			j.at("bomId").get_to(bomPojo.bomId);
			//
			j.at("isSecondSource").get_to(bomPojo.isSecondSource);
			j.at("isBOMViewWFTask").get_to(bomPojo.isBOMViewWFTask);
			j.at("isCanDcn").get_to(bomPojo.isCanDcn);
			j.at("canEditPIEE").get_to(bomPojo.canEditPIEE);
			j.at("showDifference").get_to(bomPojo.showDifference);
			//
			j.at("secondSource").get_to(bomPojo.secondSource);
			j.at("childs").get_to(bomPojo.childs);
		}
		tag_t getNotReleasedDCNItemRev(tag_t itemRev);
	}
	extern __declspec(dllexport) string getMntEBomStruct(string param);

#ifdef __cplusplus
}
#endif

#endif 