#include "secondSourceCommonUtils.h"
#include "getSelectSync2ndSourceStruct.h"
using namespace MNTEBOM2ndSource;
#ifdef __cplusplus
extern "C" {
#endif

	static string
		projectID,
		syncFrom,
		level,
		groupName;

	static vector<string>
		leafMainItemList; // 叶子节点主料集合

	static vector<MatGroupInfo>
		matInfoList; // 替代料群组信息集合

	int getSelectTotalSync2ndSourceInfo(SecondSourceInfo& bomBean, tag_t& parentBomLine, tag_t& bomLine, boolean syncFlag, boolean enable);

	int getSelectLeafMainItemList(vector<SelectSync2ndSourceInfo>& selectDataList, vector<string>& list);

	extern __declspec(dllexport) string getSelectSync2ndSourceStruct(string params) {
		int
			ifail = ITK_ok;
		char
			* itemId = NULL,
			* version = NULL;

		tag_t
			rev_rule = NULLTAG,
			bomLine = NULLTAG,
			parentBomLine = NULLTAG,
			window = NULLTAG;

		map<string, string> 
			dataMap;

		SecondSourceInfo
			rootBean;

		vector<SelectSync2ndSourceInfo>
			selectDataList;

		SelectSync2ndSourceParams 
			selectSyncParams;
		json
			paramsData,
			data = json::array(),
			jsonData = json::array(),
			result;

		string
			jsonFileName,
			jsonFileNamePath;

		createLogFile("2ndSourceService_getSelectSync2ndSourceStruct");
		writeLog("params == %s\n", params.c_str());
//		paramsData = json::parse(string2wstring(params));		

		try {
			selectSyncParams = json::parse(params).get<SelectSync2ndSourceParams>();
			projectID = selectSyncParams.projectID;
			syncFrom = selectSyncParams.syncFrom;
			syncFrom = getTcCode(syncFrom.c_str());
			level = selectSyncParams.level;
			groupName = selectSyncParams.groupName;
//			auto list = selectSyncParams.selectData;
			selectDataList = selectSyncParams.selectData;

			writeLog("projectID == %s\n", projectID.c_str());
			writeLog("level == %s\n", level.c_str());
			writeLog("syncFrom == %s\n", syncFrom.c_str());
			writeLog("groupName == %s\n", groupName.c_str());

			jsonFileName = level + ("2ndSourceService_getSelectSync2ndSourceStruct");
			jsonFileNamePath = createJsonFile(jsonFileName.c_str());

			leafMainItemList.clear();
			matInfoList.clear();

			getSelectLeafMainItemList(selectDataList, leafMainItemList); // 获取当前选中的根节点主料号集合
			distinctValue(leafMainItemList); // 去重
			getMatGroupInfoList(leafMainItemList, matInfoList); // 查询DB获取替代料群组信息
			for (auto info : selectDataList) {
				string parentUid = info.parentUid;
				string bomUid = info.bomUid;

				writeLog("parentUid == %s\n", parentUid.c_str());
				writeLog("bomUid == %s\n", bomUid.c_str());

				ITK__convert_uid_to_tag(parentUid.c_str(), &parentBomLine); // 获取父BOMLine
				ITK__convert_uid_to_tag(bomUid.c_str(), &bomLine); // 获取子BOMLine

				SecondSourceInfo bomBean;
				getSelectTotalSync2ndSourceInfo(bomBean, parentBomLine, bomLine, true, true);

				json beanJson = bomBean;
				if (!checkJsonItemExist(data, beanJson, "itemRevUid")) { // 判断Json Item是否已经存在
					data.push_back(beanJson);
				}
			}

			result["code"] = "0000";
			result["msg"] = getExternCode("成功");
			result["data"] = data;
		} catch (int& error_code) {
			ifail = error_code;
			throw exception(getErrorMsg(ifail).c_str());
		} catch (const std::exception& e) {
			writeLog("error == %s\n", e.what());
			result["code"] = "4002";
			result["msg"] = getExternCode(("BOM选择性同步2nd Source失败,错误信息如下: " + (string)e.what()).c_str());
		}

	CLEANUP:
		appendJsonData(jsonFileNamePath.c_str(), data.dump().c_str()); // 将结构体转换为json对象保存到json中
		return result.dump();
	}	

	/*
	* 获取选择性同步2nd Source所有的替代料集合
	*/
	int getSelectTotalSync2ndSourceInfo(SecondSourceInfo& bomBean, tag_t& parentBomLine, tag_t& bomLine, boolean syncFlag, boolean enable) {

		int
			ifail = ITK_ok;

		tag_t
			parentItemRev = NULLTAG;

		logical
			isSub = false,
			hasSub = false;

		string
			verCheckResult;

		ITK_THROW_ERROR(BOM_line_ask_is_substitute(bomLine, &isSub)); // 判断是否为替代料
		if (!isSub) {
			getTcProp(bomBean, bomLine); //获取属性
			ITK_THROW_ERROR(AOM_ask_value_tag(parentBomLine, bomAttr_lineObjectTag, &parentItemRev)); // 通过BOMLine获取对象版本			
			bomBean.enable = enable;
			bomBean.parentItem = getPropValue(parentItemRev, item_id);

			vector<SecondSourceInfo> subList;
			getTotal2ndSourceSubList(matInfoList, subList, bomLine, bomBean, syncFrom, groupName, level, projectID, syncFlag, enable); // 获取执行同步2nd Source后的替代料集合
			if (!subList.empty()) {
				if (verCheckResult.empty()) {
					checkVerCount(parentItemRev, verCheckResult);
				}

				bomBean.verNote = getExternCode(verCheckResult.c_str());
				bomBean.alternativeGroup = getAltGroupByFindNum(bomBean);
			}

			bomBean.substitutesList = subList;
		}

	CLEANUP:
		return ifail;
	}


	/*
	* 获取当前选中的根节点主料号集合
	*/
	int getSelectLeafMainItemList(vector<SelectSync2ndSourceInfo>& selectDataList, vector<string>& list) {
		int
			ifail = ITK_ok;

		tag_t
			bomLine = NULLTAG;
		for (auto& info : selectDataList) {
			string bomUid = info.bomUid;
			ITK__convert_uid_to_tag(bomUid.c_str(), &bomLine); // 获取子BOMLine
			string itemId = getPropValue(bomLine, bomAttr_itemId);
			list.push_back(itemId);
		}

		return ifail;
	}


#ifdef __cplusplus
}
#endif
