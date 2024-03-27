#include "secondSourceCommonUtils.h"
#include "getSync2ndSourceStruct.h"
using namespace MNTEBOM2ndSource;
#ifdef __cplusplus
extern "C" {
#endif

	static string		
		projectID,
		syncFrom,
		level,
		groupName;

	static map<string, string>
		sourceBomMap;

	static vector<string>
		leafMainItemList; // 叶子节点主料集合

	static vector<MatGroupInfo>
		matInfoList; // 替代料群组信息集合

	int recurseSync2ndSourceBOM(SecondSourceInfo& rootBean, tag_t& topLine, boolean syncFlag, boolean enable);
	int getSyncSubBOMStruct(tag_t& bomLine, vector<SecondSourceInfo>& subList, bool syncFlag, bool enable);
	int addSubBOMChildList(vector<SecondSourceInfo>& subList, vector<SecondSourceInfo> subChilds, SecondSourceInfo findBean);

	extern __declspec(dllexport) string getSync2ndSourceStruct(string params) {
		int
			ifail = ITK_ok;
		char
			* itemId = NULL,
			* version = NULL;
		tag_t
			topItem = NULLTAG,
			topItemRev = NULLTAG,
			rev_rule = NULLTAG,
			topLine = NULLTAG,
			window = NULLTAG;

		SecondSourceInfo
			rootBean;

		TotalSync2ndSource
			totalSyncParams;

		json
			paramsData,
			data,
			result;

		string
			uid,
			jsonFileName,
			jsonFileNamePath;

		createLogFile("2ndSourceService_getSync2ndSourceStruct");
		writeLog("params == %s\n", params.c_str());
//		paramsData = json::parse(string2wstring(params));
		

		try {
			totalSyncParams = paramsData = json::parse(params).get<TotalSync2ndSource>();
			uid = totalSyncParams.uid;
			projectID = totalSyncParams.projectID;
			syncFrom = totalSyncParams.syncFrom;
			syncFrom = getTcCode(syncFrom.c_str());
			level = totalSyncParams.level;
			groupName = totalSyncParams.groupName;
			sourceBomMap = totalSyncParams.sourceBomMap;

			writeLog("uid == %s\n", uid.c_str());
			writeLog("projectID == %s\n", projectID.c_str());
			writeLog("syncFrom == %s\n", syncFrom.c_str());
			writeLog("level == %s\n", level.c_str());
			writeLog("groupName == %s\n", groupName.c_str());

			ITK__convert_uid_to_tag(uid.c_str(), &topLine); // 获取顶层BOMLine
			jsonFileName = level + ("2ndSourceService_getSync2ndSourceStruct");
			jsonFileNamePath = createJsonFile(jsonFileName.c_str());

			ITK_THROW_ERROR(BOM_line_ask_window(topLine, &window)); // 通过BOMLine获取window
//			ITK_THROW_ERROR(AOM_ask_value_tag(topLine, bomAttr_lineWindowTag, &window)); // 通过顶阶BOMLine获取window对象
			ITK_THROW_ERROR(BOM_set_window_pack_all(window, false)); // 解包

			leafMainItemList.clear();
			matInfoList.clear();

			getLeafNoSubItemList(topLine, leafMainItemList); // 获取叶子节点的料号集合
			distinctValue(leafMainItemList); // 去重
			getMatGroupInfoList(leafMainItemList, matInfoList); // 查询DB获取替代料群组信息
			recurseSync2ndSourceBOM(rootBean, topLine, true, true);
			rootBean.enable = false;

			data = rootBean;
			result["code"] = "0000";
			result["msg"] = getExternCode("成功");
			result["data"] = data;
		} catch (int& error_code) {
			ifail = error_code;
			throw exception(getErrorMsg(ifail).c_str());
		} catch (const std::exception& e) {
			writeLog("error == %s\n", e.what());
			result["code"] = "4002";
			result["msg"] = getExternCode(("BOM同步2nd Source失败,错误信息如下: " + (string)e.what()).c_str());
		}

	CLEANUP:
		appendJsonData(jsonFileNamePath.c_str(), data.dump().c_str()); // 将结构体转换为json对象保存到json中
		return result.dump();
	}

	/*
	* 递归遍历同步2nd Source BOMLine
	* syncFlag 用作是否需要同步2nd Source标识, true代表需要同步
	* enable 判断是否可以右键和选中复选框, true代表可以
	*/
	int recurseSync2ndSourceBOM(SecondSourceInfo& rootBean, tag_t& topLine, boolean syncFlag, boolean enable) {
		int
			ifail = ITK_ok,
			child_count = 0;

		char
			* childItemRevUid = NULL;

		tag_t
			* children = NULLTAG;

		tag_t
			childBomLine = NULLTAG,
			childItemRev = NULLTAG;

		string
			verCheckResult;

		getTcProp(rootBean, topLine);
		ITK_THROW_ERROR(BOM_line_ask_child_lines(topLine, &child_count, &children));
		if (child_count > 0 && isExpandChilds(topLine, rootBean.materialGroup)) {
			for (int i = 0; i < child_count; i++) {
				logical
					isSub = false,
					hasSub = false;
				childBomLine = children[i];
				ITK_THROW_ERROR(AOM_ask_value_tag(childBomLine, bomAttr_lineObjectTag, &childItemRev));
				ITK__convert_tag_to_uid(childItemRev, &childItemRevUid);

				ITK_THROW_ERROR(BOM_line_ask_is_substitute(childBomLine, &isSub)); // 判断是否为替代料
				if (isSub) {
					continue;
				}
				if (!sourceBomMap.empty()) {
					if (isBom(childItemRev)) {
						auto it = sourceBomMap.find(childItemRevUid); // 如果源机种BOM集合不为空, 并且此物料存在源机种BOM中
						if (it != sourceBomMap.end()) {
							syncFlag = false;
							enable = false;
						} else {
							syncFlag = true;
							enable = true;
						}
					}
				}
				SecondSourceInfo childBean;
				recurseSync2ndSourceBOM(childBean, childBomLine, syncFlag, enable);
				childBean.enable = enable;
				childBean.parentItem = rootBean.item;
//				childBean.parentItem = getPropValue(topLine, bomAttr_itemId);
				vector<SecondSourceInfo> subList;
				if (childBean.childs.empty()) {
					getTotal2ndSourceSubList(matInfoList, subList, childBomLine, childBean, syncFrom, groupName, level, projectID, syncFlag, enable); // 获取执行同步2nd Source后的替代料集合
				}				
				
				if (!subList.empty()) {
					if (verCheckResult.empty()) {
						tag_t
							parentItemRev = NULLTAG;
						ITK_THROW_ERROR(AOM_ask_value_tag(topLine, bomAttr_lineObjectTag, &parentItemRev)); // 通过BOMLine获取对象版本
						checkVerCount(parentItemRev, verCheckResult);
						writeLog("%s\n", verCheckResult);
					}
					childBean.verNote = getExternCode(verCheckResult.c_str());
					childBean.alternativeGroup = getAltGroupByFindNum(childBean);
				}

				getSyncSubBOMStruct(childBomLine, subList, syncFlag, enable); // 获取同步2nd Source替代料BOM的子阶

				childBean.substitutesList = subList;
				rootBean.childs.push_back(childBean);
			}
		}

	CLEANUP:
		DOFREE(children);
		DOFREE(childItemRevUid);
		return ifail;
	}


	/*
	* 获取同步2nd Source替代料BOM的子阶
	*/
	int getSyncSubBOMStruct(tag_t& bomLine, vector<SecondSourceInfo>& subList, bool syncFlag, bool enable) {
		int
			ifail = ITK_ok,
			child_count = 0,
			sub_child_count = 0;
		tag_t
			* children = NULLTAG,
			* sub_children = NULLTAG;
		tag_t
			subBomLine = NULLTAG,
			subItemRev = NULLTAG;
		logical
			hasSub = false;

		ITK_THROW_ERROR(BOM_line_ask_has_substitutes(bomLine, &hasSub)); // 判断是否含有替代料
		if (hasSub) {
			ITK_THROW_ERROR(BOM_line_ask_child_lines(bomLine, &sub_child_count, &sub_children)); // 获取替代料BOM集合
			for (int i = 0; i < sub_child_count; i++) {
				subBomLine = sub_children[i];
				SecondSourceInfo subChildBean;
				recurseSync2ndSourceBOM(subChildBean, subBomLine, syncFlag, enable);
				vector<SecondSourceInfo> subChilds = subChildBean.childs;
				addSubBOMChildList(subList, subChilds, subChildBean); // 替代料BOM添加子料
			}
		}

	CLEANUP:
		DOFREE(sub_children);
		return ifail;
	}

	/*
	* 替代料BOM添加子料
	*/
	int addSubBOMChildList(vector<SecondSourceInfo>& subList, vector<SecondSourceInfo> subChilds, SecondSourceInfo findBean) {
		int
			ifail = ITK_ok,
			index = -1;
		auto it = find_if(subList.begin(), subList.end(), [&](SecondSourceInfo& bean) {
			return bean.itemRevUid.compare(findBean.itemRevUid) == 0;
			});

		if (it != subList.end()) {
			index = it - subList.begin();
		}
		if (index == -1) {
			goto CLEANUP;
		}

		subList[index].childs.insert(subList[index].childs.end(), subChilds.begin(), subChilds.end());

	CLEANUP:
		return ifail;
	}


#ifdef __cplusplus
}
#endif
