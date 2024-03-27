
/*=================================================================================
* @file					epm_register_handler.h
* @brief				itk user exits function declation, to register custom handlers
* @date					2009/2/13
* @author				Ray Li
* @history
* ===================================================================================
* Date					Name				Description
* 13-Feb-2009			Ray				created
*===================================================================================*/

#ifndef SECONDSOURCECOMMONUTILS
#define SECONDSOURCECOMMONUTILS

#include "json.hxx"
#include <string>
#include <iostream>
#include "search.h"
#include "util.h"
#include "tcprop.h"
#include <time.h>
#include "preference.h"
#include "version_pattern.h"
#define FIRSTVERNOTE "初版";
#define NOTFIRSTVERNOTE "非初版"
#define GLOBAL2NDSOURCE "全局2nd Source"
#define PROJECT2NDSOURCE "專案2nd Source"
#define PRI "PRI"
#define ALT "ALT"
#define PNMS "PNMS"
#define TC "TC"
#define MGT "MGT"
#define EDAComPart "EDAComPart"
#define D9_MaterialGroup "D9_MaterialGroup"
#define D9_MaterialGroupRevision "D9_MaterialGroupRevision"
#define ItemRevision "ItemRevision"
#define Latest_Working "Latest Working"
#define enum_to_string(x) #x
using json = nlohmann::json;
using namespace std;
using namespace ITEM_ATTR;
using namespace BOM_ATTR;
using namespace PROJECT;
namespace MNTEBOM2ndSource {
	struct SecondSourceInfo {
		string findNum,
			item,
			parentItem,
			version,
			description,
			sapDescription,
			mfg,
			mfgPn,
			location,
			alternativeGroup,
			alternativeCode,
			usageProb = "0",
			materialGroup,
			materialType,
			procurementType,
			qty,
			unit,
			status,
			referenceDimension,
			sapRev,
			packageType,
			side,
			supplierZF,
			sourceSystem = TC,
			itemRevUid,
			bomLineUid,
			uid,
			verNote,
			materialGroupItemId, // 替代料群组ItemID
			curProjectID,
			projectID,
			syncFrom,
			level,
			groupName;

		bool isSub = false, // 判断是否为替代料
			checkStates = false, // 判断是否为选中状态       
			subExistBom = true, // 替代料是否存在BOMLine中，默认替代料存在BOMLine中        
			syncCheckFlag = false, // 用作是否发生同步的标识
			enable = true, // 默认设置为可以右键和取消勾选框
			isDelete = false, // 判断是否需要删除，默认为不需要删除
			isNewVersion = false, // 判断是否是最新版
			hasMerge = false, // 用量和是否发生合并, 默认为没有发生合并
			hasRevise = false; // 代表是否发生升版, 默认为没有发生升版
		vector<SecondSourceInfo>
			childs, // 子对象集合
			substitutesList; // 替代料集合  
		map<string, string> sourceBomMap;

	public:
		NLOHMANN_DEFINE_TYPE_INTRUSIVE(SecondSourceInfo, findNum, item, parentItem, version, description, sapDescription, mfg, mfgPn, location,
			alternativeGroup, alternativeCode, usageProb, materialGroup, materialType, procurementType, qty, unit, status, referenceDimension, sapRev,
			packageType, side, supplierZF, sourceSystem, itemRevUid, bomLineUid, uid, verNote, materialGroupItemId, curProjectID, projectID, syncFrom, level,
			groupName, isSub, checkStates, subExistBom, syncCheckFlag, enable, isDelete, isNewVersion, hasMerge, hasRevise, childs, substitutesList, sourceBomMap);
	};

	struct SelectSync2ndSourceInfo {
		string
			parentUid,
			bomUid;
	public:
		NLOHMANN_DEFINE_TYPE_INTRUSIVE(SelectSync2ndSourceInfo, parentUid, bomUid);
	};

	struct SelectSync2ndSourceParams {
		string
			projectID,
			syncFrom,
			level,
			groupName;
		map<string, string>
			sourceBomMap;
		vector<SelectSync2ndSourceInfo> selectData;
	public:
		NLOHMANN_DEFINE_TYPE_INTRUSIVE(SelectSync2ndSourceParams, projectID, syncFrom, level, groupName, sourceBomMap, selectData);
	};

	struct TotalSync2ndSource {
		string
			uid,
			projectID,
			syncFrom,
			level,
			groupName;
		map<string, string>
			sourceBomMap;
	public:
		NLOHMANN_DEFINE_TYPE_INTRUSIVE(TotalSync2ndSource, uid, projectID, syncFrom, level, groupName, sourceBomMap);
	};

	struct MatGroupInfo {
		tag_t
			matGroupItemRev;
		string			
			matGroupItemId,
			matGroupVersion,
			childItemId;
	};
}
#ifdef __cplusplus
extern "C" {
#endif	

	namespace MNTEBOM2ndSource {

		//struct SecondSourceInfo {
		//	string findNum,
		//		item,
		//		parentItem,
		//		version,
		//		description,
		//		sapDescription,
		//		mfg,
		//		mfgPn,
		//		location,
		//		alternativeGroup,
		//		alternativeCode,
		//		usageProb,
		//		materialGroup,
		//		materialType,
		//		procurementType,
		//		qty,
		//		unit,
		//		status,
		//		referenceDimension,
		//		sapRev,
		//		packageType,
		//		side,
		//		supplierZF,
		//		sourceSystem = "TC",
		//		itemRevUid,
		//		bomLineUid,
		//		verNote,
		//		materialGroupItemId, // 替代料群组ItemID
		//		curProjectID,
		//		projectID,
		//		syncFrom,
		//		level,
		//		groupName;

		//	bool isSub = false, // 判断是否为替代料
		//		checkStates = false, // 判断是否为选中状态       
		//		subExistBom = true, // 替代料是否存在BOMLine中，默认替代料存在BOMLine中        
		//		syncCheckFlag = false, // 用作是否发生同步的标识
		//		enable = true; // 默认设置为可以右键和取消勾选框
		//	vector<SecondSourceInfo>
		//		childs, // 子对象集合
		//		substitutesList; // 替代料集合  
		//	map<string, string> sourceBomMap;

		//public:
		//	NLOHMANN_DEFINE_TYPE_INTRUSIVE(SecondSourceInfo, findNum, item, parentItem, version, description, sapDescription, mfg, mfgPn, location,
		//		alternativeGroup, alternativeCode, usageProb, materialGroup, materialType, procurementType, qty, unit, status, referenceDimension, sapRev, 
		//		packageType, side, supplierZF, sourceSystem, itemRevUid, bomLineUid, verNote, materialGroupItemId, curProjectID, projectID, syncFrom, level, 
		//		groupName, isSub, checkStates, subExistBom, syncCheckFlag, enable, childs, substitutesList, sourceBomMap);
		//};
		
		 /*public:NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(SecondSourceInfo, findNum, item,parentItem, version, description, sapDescription, mfg, mfgPn, location, 
			alternativeGroup, alternativeCode, usageProb, materialGroup, materialType, procurementType, qty, unit, status, referenceDimension, sapRev,packageType,			side,			supplierZF,			sourceSystem,
			itemRevUid,bomLineUid,verNote,materialGroupItemId,curProjectID,projectID,syncFrom,level,groupName, isSub, 
			checkStates,  subExistBom, syncCheckFlag, enable, childs, substitutesList, sourceBomMap);*/

		

//		struct SelectSync2ndSourceParams {
//			string
//				projectID,
//				syncFrom,
//				level,
//				groupName;
////			vector<SelectSync2ndSourceInfo> selectData;
//		public:
//			NLOHMANN_DEFINE_TYPE_INTRUSIVE(SelectSync2ndSourceParams, projectID, syncFrom, level);
//		};

//		NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(SelectSync2ndSourceParams, projectID, syncFrom, level);

		
		/*static void to_json(json& j, const SecondSourceInfo& rootInfo) {
			j = json{
				{"findNum",rootInfo.findNum},
				{"item",rootInfo.item},
				{"parentItem",rootInfo.parentItem},
				{"version",rootInfo.version},
				{"description",rootInfo.description},
				{"sapDescription",rootInfo.sapDescription},
				{"mfg",rootInfo.mfg},
				{"mfgPn",rootInfo.mfgPn},
				{"location",rootInfo.location},
				{"alternativeGroup",rootInfo.alternativeGroup},
				{"alternativeCode",rootInfo.alternativeCode},
				{"usageProb",rootInfo.usageProb},
				{"materialGroup",rootInfo.materialGroup},
				{"materialType",rootInfo.materialType},
				{"procurementType",rootInfo.procurementType},
				{"qty",rootInfo.qty},
				{"unit",rootInfo.unit},
				{"status",rootInfo.status},
				{"referenceDimension",rootInfo.referenceDimension},
				{"sapRev",rootInfo.sapRev},
				{"packageType",rootInfo.packageType},
				{"side",rootInfo.side},
				{"supplierZF",rootInfo.supplierZF},
				{"sourceSystem",rootInfo.sourceSystem},
				{"itemRevUid",rootInfo.itemRevUid},
				{"bomLineUid",rootInfo.bomLineUid},
				{"verNote",rootInfo.verNote},
				{"materialGroupItemId",rootInfo.materialGroupItemId},
				{"curProjectID",rootInfo.curProjectID},
				{"projectID",rootInfo.projectID},
				{"syncFrom",rootInfo.syncFrom},
				{"level",rootInfo.level},
				{"groupName",rootInfo.groupName},
				{"isSub",rootInfo.isSub},
				{"checkStates",rootInfo.checkStates},
				{"subExistBom",rootInfo.subExistBom},
				{"syncCheckFlag",rootInfo.syncCheckFlag},
				{"enable",rootInfo.enable},
				{"childs",rootInfo.childs},
				{"sourceBomMap",rootInfo.sourceBomMap},
				{"substitutesList",rootInfo.substitutesList} };
		}*/		


		BOMTYPE getBOMType(tag_t bomLine); // 判断BOMLine的类型是79开头还是8开头

		bool isExpandChilds(tag_t bom_line, string materialGroup);

		int getTcProp(MNTEBOM2ndSource::SecondSourceInfo& info, tag_t& bomLine); //  获取TC属性

		int checkVerCount(tag_t& itemRev, string& checkResult); // 判断此对象是否为初版biax

		string getAltGroupByFindNum(MNTEBOM2ndSource::SecondSourceInfo& bean); // 根据FindNum获取替代料群组编号

		string getAltGroupByRule(int findNum); // 根据查找编号获取物料群主编号

		int getCurMatGroupItemIdByMatch(vector<MatGroupInfo>& matInfoList, string& matGroupItemId, string& parentItem, string& item, string level, string projectId); // 通过匹配返回当前替代料所在的群组ID

		int getCurMatGroupItemId(MNTEBOM2ndSource::SecondSourceInfo& info, string& matGroupItemId, string level, string projectId); // 返回当前替代料所在专案群组ItemId

		bool checkJsonItemExist(json& data, json& jsonItem, string key); // 判断Json Item是否已经存在

		int removeSameBean(vector<MNTEBOM2ndSource::SecondSourceInfo>& list, string uid); // 移除相同的bean

		bool compareItemUid(const MNTEBOM2ndSource::SecondSourceInfo& firstInfo, const MNTEBOM2ndSource::SecondSourceInfo& secondInfo); // 对比bean的对象版本Uid

		int distinctBeanByUid(vector<MNTEBOM2ndSource::SecondSourceInfo>& list); // 对集合数据进行去重

		int getTotal2ndSourceSubList(vector<MatGroupInfo>& matInfoList, vector<MNTEBOM2ndSource::SecondSourceInfo>& totalSubList, tag_t& bomLine, MNTEBOM2ndSource::SecondSourceInfo& bomBean, string syncFrom, string groupName,
			string level, string projectID, bool syncFlag, bool enable); // 获取执行同步2nd Source后的替代料集合

		int getMatGroupItemRevByMatch(vector<MNTEBOM2ndSource::MatGroupInfo>& matInfoList, vector<tag_t>& list, vector<string>& matGroupList, string& parentItem,
			string& item, string& syncFrom, string& groupName, string& level, string& projectID); // 获取同步替代料群组对象版本和替代料群组ID

		int get2ndSourceGroup(vector<tag_t>& searchList, vector<MNTEBOM2ndSource::SecondSourceInfo>& list, bool allCheck); // 获取2nd Source群组BOM中的物料

		int getExistSubBeanList(tag_t& bomLine, vector<MNTEBOM2ndSource::SecondSourceInfo>& totalSubList); // 获取已经存在BOMLine中的2nd Source的Bean

		void setAltGroup(SecondSourceInfo& rootBean); //设置替代料群组编号

		tag_t checkTopLineRevise(tag_t& window, SecondSourceInfo& rootBean, map<string, tag_t>& bomMap); // 判断顶阶BOMLine是否需要升版

		int checkParentBOMLineStatus(tag_t& window, tag_t& bomLine, map<string, tag_t>& bomMap); // 判断当前替代料的父BOMLine是否已经发行

		bool checkTopLineStatus(SecondSourceInfo& rootBean, tag_t& rootItemRev); // 判断顶层BOMLine的子阶是否发布，假如发布，判断子阶是否发生变化

		bool checkChange(tag_t& bomLine, vector<SecondSourceInfo>& subList); // 判断是否发生改变

		tag_t findBOMLine(tag_t& pLine, tag_t& newItemRev); // 返回在pLine子集合的newItemRev对应的BOMLine对象

		int getBOMMap(tag_t& topLine, map<string, tag_t>& bomMap); // 获取BOMLine uid集合

		void checkMerge(SecondSourceInfo& rootBean);

		int getItemListByContainSub(tag_t& topLine, vector<string>& list); // 获取含有替代料的料号集合

		int getLeafNoSubItemList(tag_t& topLine, vector<string>& list); // 获取叶子节点的料号集合

		int getMatGroupInfoList(vector<string>& list, vector<MatGroupInfo>& matInfoList); // 查询DB获取替代料群组信息
	}
	

#ifdef __cplusplus
}
#endif

#endif 