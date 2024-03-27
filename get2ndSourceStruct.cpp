#include "secondSourceCommonUtils.h"
#include "get2ndSourceStruct.h"
using namespace MNTEBOM2ndSource;
#ifdef __cplusplus
extern "C" {
#endif

	static string
		curProjectID,
		syncFrom,
		level,
		groupName;

	static map<string, string>
		sourceBomMap;

	static vector<string> 
		containSubItemList; // 包含替代料的主料集合

	static vector<MatGroupInfo>
		matInfoList; // 替代料群组信息集合

	int recurse2ndSourceBOM(SecondSourceInfo& rootBean, tag_t& topLine, bool enable);
	int getSubBeanList(SecondSourceInfo& bomBean, tag_t& bomLine, vector<SecondSourceInfo>& subList, string& parentItem, string alternativeGroup, string& matGroupItemId, bool enable);
	int getTotalSourceBomMap(tag_t& topItemRev);
	int getSourceBomMaps(tag_t& topLine);


	extern __declspec(dllexport) string get2ndSourceStruct(string uid) {
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

		json
			data,
			result;

		string
			jsonFileName,
			jsonFileNamePath;

		createLogFile("2ndSourceService_get2ndSourceStruct");
		writeLog("uid == %s\n", uid.c_str());

		try {
			ITK__convert_uid_to_tag(uid.c_str(), &topLine); // 获取顶层BOMLine
			ITK_THROW_ERROR(AOM_ask_value_tag(topLine, bomAttr_lineObjectTag, &topItemRev));
			ITK_THROW_ERROR(ITEM_ask_item_of_rev(topItemRev, &topItem));
//			ITK_THROW_ERROR(AOM_ask_value_tag(topLine, bomAttr_lineItemTag, &topItem));

			ITK_THROW_ERROR(AOM_ask_value_string(topItemRev, "item_id", &itemId));
			ITK_THROW_ERROR(AOM_ask_value_string(topItemRev, "item_revision_id", &version));

			if (startWithStr(itemId, "79")) {
				level = enum_to_string(L6);
			} else if (startWithStr(itemId, "8")) {
				level = enum_to_string(L10);
			}

			jsonFileName = level + ("2ndSourceService_get2ndSourceStruct");
			jsonFileNamePath = createJsonFile(jsonFileName.c_str());

			ITK_THROW_ERROR(BOM_line_ask_window(topLine, &window)); // 通过BOMLine获取window
			ITK_THROW_ERROR(BOM_set_window_pack_all(window, false)); // 解包

			curProjectID = getCurProjectId(topItem);
			writeLog("curProjectID == %s\n", curProjectID.c_str());

			groupName = getCurGroup(); // 获取当前登录用户所在的组
			writeLog("groupName == %s\n", groupName.c_str());

			sourceBomMap.clear();
			containSubItemList.clear();
			matInfoList.clear();
			getTotalSourceBomMap(topItemRev); // 获取所有的源机种BOM集合
			getItemListByContainSub(topLine, containSubItemList); // 获取含有替代料的料号集合
			for (auto str : containSubItemList) {
				writeLog("subItemId == %s\n", str.c_str());
			}

			distinctValue(containSubItemList); // 去重
			getMatGroupInfoList(containSubItemList, matInfoList); // 查询DB获取替代料群组信息
			recurse2ndSourceBOM(rootBean, topLine, true); // 递归遍历BOMLine	
			rootBean.enable = false;
			
			rootBean.curProjectID = curProjectID;
			rootBean.level = level;
			rootBean.groupName = groupName;
			rootBean.sourceBomMap = sourceBomMap;
			
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
			result["msg"] = getExternCode(("BOM加载2nd Source失败,错误信息如下: " + (string)e.what()).c_str());
		}

	CLEANUP:
		appendJsonData(jsonFileNamePath.c_str(), data.dump().c_str()); // 将结构体转换为json对象保存到json中
		return result.dump();
	}

	/*
	* 递归遍历BOMLine
	* enable 判断是否可以右键和选中复选框, true代表可以
	*/
	int recurse2ndSourceBOM(SecondSourceInfo& rootBean, tag_t& topLine, bool enable) {
		int
			ifail = ITK_ok,
			child_count = 0,
			sub_child_count = 0;

		char
			* childItemRevUid = NULL;

		tag_t
			* children = NULLTAG,
			* sub_children = NULLTAG;

		tag_t
			childBomLine = NULLTAG,
			subBomLine = NULLTAG,
			childItemRev = NULLTAG,
			subItemRev = NULLTAG;

		string
			verCheckResult;

		getTcProp(rootBean, topLine);

		ITK_THROW_ERROR(BOM_line_ask_child_lines(topLine, &child_count, &children));
		if (child_count > 0 && isExpandChilds(topLine, rootBean.materialGroup)) {
			for (int i = 0; i < child_count; i++) {
				logical
					isSub = false,
					hasSub = false;

				/*string
					altGroup,
					altCode;*/

				childBomLine = children[i];
				ITK_THROW_ERROR(AOM_ask_value_tag(childBomLine, bomAttr_lineObjectTag, &childItemRev));
				ITK__convert_tag_to_uid(childItemRev, &childItemRevUid);

				ITK_THROW_ERROR(BOM_line_ask_is_substitute(childBomLine, &isSub)); // 判断是否为替代料
				if (isSub) {
					continue;
				}

				if (!sourceBomMap.empty()) {
					if (isBom(childItemRev)) { // 判断子对象版本是否含有BOM
						auto it = sourceBomMap.find(childItemRevUid); // 如果源机种BOM集合不为空, 并且此物料存在源机种BOM中
						if (it != sourceBomMap.end()) {
							enable = false;
						} else {
							enable = true;
						}
					}
				}

				SecondSourceInfo childBean;
				recurse2ndSourceBOM(childBean, childBomLine, enable);
				childBean.enable = enable;
//				childBean.parentItem = rootBean.parentItem;
				childBean.parentItem = rootBean.item;

				ITK_THROW_ERROR(BOM_line_ask_has_substitutes(children[i], &hasSub)); // 判断是否含有替代料
				if (hasSub) {
					if (verCheckResult.empty()) {
						tag_t
							parentItemRev = NULLTAG;
						ITK_THROW_ERROR(AOM_ask_value_tag(topLine, line_object, &parentItemRev));
						checkVerCount(parentItemRev, verCheckResult);
						writeLog("%s\n", verCheckResult);
					}

					childBean.verNote = getExternCode(verCheckResult.c_str());
					childBean.alternativeGroup = getAltGroupByFindNum(childBean);
					childBean.alternativeCode = PRI;
					childBean.usageProb = "100";
					ITK_THROW_ERROR(BOM_line_list_substitutes(children[i], &sub_child_count, &sub_children)); // 获取替代料BOM集合
					vector<SecondSourceInfo> subList;
					string matGroupItemId = "";
					getSubBeanList(childBean, children[i], subList, rootBean.item, childBean.alternativeGroup, matGroupItemId, enable); // 获取替代料Bean集合						
					childBean.substitutesList = subList;
					childBean.materialGroupItemId = matGroupItemId;
				}
				rootBean.childs.push_back(childBean);
			}
		}

	CLEANUP:
		DOFREE(children);
		DOFREE(sub_children);
		DOFREE(childItemRevUid);
		return ifail;
	}

	/*
	* 获取替代料bean集合
	*/
	int getSubBeanList(SecondSourceInfo& bomBean, tag_t& bomLine, vector<SecondSourceInfo>& subList, string& parentItem, string alternativeGroup, string& matGroupItemId, bool enable) {
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

		ITK_THROW_ERROR(BOM_line_list_substitutes(bomLine, &sub_child_count, &sub_children)); // 获取替代料BOM集合
		for (int i = 0; i < sub_child_count; i++) {
			subBomLine = sub_children[i];
			SecondSourceInfo subBean;
			getTcProp(subBean, subBomLine); // 获取替代料属性
//			subBean.parentItem = item;

			if (matGroupItemId.empty()) {
//				getCurMatGroupItemId(subBean, matGroupItemId, level, curProjectID); // 获取当前替代料群组ID
				if (!matInfoList.empty()) {
					getCurMatGroupItemIdByMatch(matInfoList, matGroupItemId, parentItem, bomBean.item, level, curProjectID); // 通过匹配返回当前主料料所在的群组ID
					if (matGroupItemId.empty()) {
						getCurMatGroupItemIdByMatch(matInfoList, matGroupItemId, parentItem, subBean.item, level, curProjectID); // 通过匹配返回当前替代料所在的群组ID
					}					
				}				
			}

			subBean.materialGroupItemId = matGroupItemId;
			subBean.alternativeGroup = alternativeGroup;
			subBean.alternativeCode = ALT;
			subBean.location = "";
			subBean.isSub = true;
			subBean.checkStates = true;
			subBean.enable = enable;
			subBean.parentItem = parentItem;
			ITK_THROW_ERROR(BOM_line_ask_child_lines(subBomLine, &child_count, &children)); // 判断替代料下是否含有BOM
			if (child_count > 0) {
				SecondSourceInfo subChildBean;
				recurse2ndSourceBOM(subChildBean, subBomLine, enable);
				vector<SecondSourceInfo> subChilds = subChildBean.childs;
				subBean.childs.insert(subBean.childs.end(), subChilds.begin(), subChilds.end());
			}
			subList.push_back(subBean);
		}

		if (!matGroupItemId.empty()) { // 重新设置一些替代料群组ID
			for (auto& subBean : subList) {
				subBean.materialGroupItemId = matGroupItemId;
			}
		}
	CLEANUP:
		DOFREE(sub_children);
		DOFREE(children);
		return ifail;
	}

	/*
	* 获取所有的源机种BOM集合
	*/
	int getTotalSourceBomMap(tag_t& topItemRev) {
		int
			ifail = ITK_ok,
			num = 0;

		tag_t
			* tag_list = NULLTAG;

		tag_t
			sourceTopItem = NULLTAG,
			sourceTopItemRev = NULLTAG,
			rev_rule = NULLTAG,
			window = NULLTAG,
			topLine = NULLTAG;

		ITK_THROW_ERROR(AOM_ask_value_tags(topItemRev, d9_HasSourceBOM_REL, &num, &tag_list));
		for (int i = 0; i < num; i++) {
			logical
				isRev = false;

			GTCTYPE_is_type_of(tag_list[i], ItemRevision, &isRev);
			if (!isRev) {
				continue;
			}
			sourceTopItemRev = tag_list[i];

			ITK_THROW_ERROR(ITEM_ask_item_of_rev(sourceTopItemRev, &sourceTopItem));
			ITK_THROW_ERROR(CFM_find(Latest_Working, &rev_rule));
			ITK_THROW_ERROR(BOM_create_window(&window));
			ITK_THROW_ERROR(BOM_set_window_config_rule(window, rev_rule));
			ITK_THROW_ERROR(BOM_set_window_pack_all(window, false)); // 解包

			ITK_THROW_ERROR(BOM_set_window_top_line(window, sourceTopItem, sourceTopItemRev, NULL, &topLine));

			try {
				getSourceBomMaps(topLine);
			} catch (int& error_code) {
				ifail = error_code;
				goto CLEANUP;
			}			
		}

	CLEANUP:
		DOFREE(tag_list);
		if (window != NULLTAG) {
			ITK_THROW_ERROR(BOM_close_window(window));
		}		
		if (ifail != ITK_ok) {
			throw ifail;
		}		
		return ifail;
	}


	/*
	* 获取单个源机种BOM所有含有子对象集合
	*/
	int getSourceBomMaps(tag_t& topLine) {
		int
			ifail = ITK_ok,
			child_count = 0;

		tag_t
			* children = NULLTAG;

		tag_t
			itemRev = NULLTAG,
			targetObj = NULLTAG;

		char
			* itemRevUid = NULL;

		ITK_THROW_ERROR(BOM_line_ask_child_lines(topLine, &child_count, &children));

		for (int i = 0; i < child_count; i++) {
			targetObj = children[i];
			ITK_THROW_ERROR(AOM_ask_value_tag(targetObj, bomAttr_lineObjectTag, &itemRev));

			string itemId = getPropValue(itemRev, item_id);
			string version = getPropValue(itemRev, current_revision_id);
			ITK__convert_tag_to_uid(itemRev, &itemRevUid);

			sourceBomMap.insert(pair<string, string>(itemRevUid, itemId + "/" + version));

			getSourceBomMaps(targetObj);
		}

	CLEANUP:
		DOFREE(children);
		DOFREE(itemRevUid);
		return ifail;
	}


#ifdef __cplusplus
}
#endif
