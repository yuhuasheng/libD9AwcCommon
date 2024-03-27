#include "secondSourceCommonUtils.h"
#include "save2ndSource.h"
using namespace MNTEBOM2ndSource;
#ifdef __cplusplus
extern "C" {
#endif

	static string
		curProjectID,
		level;

	static
		tag_t window = NULLTAG;

	static map<string, tag_t>
		bomMap;	

	static vector<string>
//		preDeleteItemList, // 等待删除的料号集合
		containSubItemList; // 包含替代料的主料集合

	static vector<MatGroupInfo>
		matInfoList; // 替代料群组信息集合

	static void getPreDeleteList(SecondSourceInfo& bomBean, vector<string>& list);
	static void getContainSubItemList(SecondSourceInfo& bomBean, vector<string>& list);
	static void filterGrobalMat(vector<MatGroupInfo>& matInfoList);
	void saveBOMTree(SecondSourceInfo& bomBean, string parentId, vector<SecondSourceInfo>& list);
	int checkExist2ndSourceChange(tag_t& bomLine, vector<SecondSourceInfo>& subList, SecondSourceInfo& bomBean, bool& addSubFlag, vector<SecondSourceInfo>& list);
	int changeExistSubItemRelation(tag_t& bomLine, vector<tag_t>& existSubList, SecondSourceInfo& bomBean, SecondSourceInfo& subBean, bool& addSubFlag, bool& hasDelete, vector<tag_t>& matTagList);
	void addNew2ndSource(tag_t& bomLine, vector<SecondSourceInfo>& subList, SecondSourceInfo& bomBean, bool& addSubFlag, vector<SecondSourceInfo>& list);
	int add2ndSourceAndSetRelation(tag_t& bomLine, SecondSourceInfo& bomBean, SecondSourceInfo& subBean, tag_t& subItemRev, bool& addSubFlag, vector<SecondSourceInfo>& list);
	int createItemFromPNMS(SecondSourceInfo& subBean, string& itemId);
	vector<tag_t> getAllTotalCurProjMatGroupItemRevList(SecondSourceInfo& bomBean, SecondSourceInfo& subBean);
	void setPrimaryMaterial(tag_t& bomLine, vector<tag_t>& existSubList, SecondSourceInfo& subBean);
	tag_t createCurProjectMatGroupItemRev(SecondSourceInfo& bomBean, SecondSourceInfo& subBean, vector<SecondSourceInfo>& list);
	string getMatGroupItemIdByMatInfoList(SecondSourceInfo& bomBean);
	int addOrDelCurProjectMatGroup(tag_t& matGroupItemRev, tag_t& needCheckItemRev, bool operation);
	int checkCurProjectMatGroupItems(tag_t& matGroupItemRev, tag_t& mainItemRev);
	static void filterContailMatGroupID(vector<SecondSourceInfo>& list);
	static int getIndexByItemUid(vector<SecondSourceInfo>& list, SecondSourceInfo& bomBean);
	static string getSelfMatGroupCode(int index);
	static void filterSubList(vector<SecondSourceInfo>& subList);	
	static void filterSubChildEmpty(vector<SecondSourceInfo>& subList);
	static void resetSubsProp(vector<SecondSourceInfo>& subList, SecondSourceInfo& bomBean);
	static bool checkSubListStates(vector<SecondSourceInfo>& subList);
	static void setBomProps(tag_t& bomLine, SecondSourceInfo& subBean);

	extern __declspec(dllexport) string save2ndSource(string params) {
		int
			ifail = ITK_ok;
		char
			* itemId = NULL,
			* version = NULL;

		tag_t
			topItem = NULLTAG,
			topItemRev = NULLTAG,
			rev_rule = NULLTAG,
			topLine = NULLTAG;

		SecondSourceInfo
			rootBean,
			resultBean;

		json			
			data,
			result;

		string
			jsonFileName,
			jsonFileNamePath;

		int markpoint_number = 0;
		try {
			POM_AM__set_application_bypass(true); // 开启旁路
			ITK_THROW_ERROR(POM_place_markpoint(&markpoint_number)); // 设置标记点
			writeLog("设置markpoint[%d]\n", markpoint_number);

			createLogFile("2ndSourceService_save2ndSource");
			writeLog("params == %s\n", params.c_str());
			rootBean = json::parse(params).get<SecondSourceInfo>();
			curProjectID = rootBean.curProjectID;
			level = rootBean.level;

			writeLog("curProjectID == %s\n", curProjectID.c_str());
			writeLog("level == %s\n", level.c_str());
			
			string topLineUid = rootBean.bomLineUid;
			ITK__convert_uid_to_tag(topLineUid.c_str(), &topLine); // 获取顶层BOMLine
//			ITK_THROW_ERROR(AOM_ask_value_tag(topLine, line_object, &topItemRev)); // 获取顶层对象版本

			ITK_THROW_ERROR(BOM_line_ask_window(topLine, &window)); // 通过BOMLine获取window

			bomMap.clear();
			containSubItemList.clear();
			matInfoList.clear();

			setAltGroup(rootBean); // 设置替代料群组ID			
			tag_t rootBomLine = checkTopLineRevise(window, rootBean, bomMap); // // 判断顶阶BOMLine是否需要升版
			if (rootBomLine != NULL) {
				topLine = rootBomLine;
			}

			getTcProp(resultBean, topLine);
			if (rootBomLine != NULL) {
				resultBean.hasRevise = true; // 设置顶阶发生升版的标识为true
			}
			ITK_THROW_ERROR(AOM_ask_value_tag(topLine, line_object, &topItemRev)); // 获取顶层对象版本
			resultBean.isNewVersion = isNewRevision(topItemRev);
			data = resultBean;
			//rootBean.setIsNewVersion(UpdateEBOMService.isNewRevsion(topLine.getItemRevision()));
			
			checkMerge(rootBean); // 判断位置,用量是否需要合并
//			getPreDeleteList(rootBean, preDeleteItemList); // 获取待删除的物料ID集合
			getContainSubItemList(rootBean, containSubItemList); // 获取包含替代料的集合
			distinctValue(containSubItemList); // 去重
			getMatGroupInfoList(containSubItemList, matInfoList); // 查询DB获取替代料群组信息

			saveBOMTree(rootBean, "", rootBean.childs); // 保存BOM结构树
			result["code"] = "0000";
			result["msg"] = getExternCode("替代料保存成功");
			result["data"] = data;

			ITK_THROW_ERROR(POM_forget_markpoint(markpoint_number)); //释放markpoint
			writeLog("释放markpoint[%d]", markpoint_number);
			ITK_THROW_ERROR(BOM_save_window(window)); // 保存窗口
		} catch (int& error_code) {
			ifail = error_code;
			//回滚markpoint
			try {
				logical state_has_changed = true;//默认TC中数据有变化
				ITK_THROW_ERROR(POM_roll_to_markpoint(markpoint_number, &state_has_changed));
				if (!state_has_changed) {
					writeLog("回滚markpoint[%d]成功", markpoint_number);
				} else {
					writeLog("回滚markpoint[%d]失败", markpoint_number);
				}
			} catch (int& rollbackerror_code) {
				ifail = rollbackerror_code;
				writeLog("POM_roll_to_markpoint markpoint[%d] fail", markpoint_number);
			}

			throw exception(getErrorMsg(ifail).c_str());
		} catch (const std::exception& e) {
			writeLog("error == %s\n", e.what());
			result["code"] = "4002";
			result["msg"] = getExternCode(("BOM保存替代料失败,错误信息如下: " + (string)e.what()).c_str());
		}

	CLEANUP:
		appendJsonData(jsonFileNamePath.c_str(), data.dump().c_str()); // 将结构体转换为json对象保存到json中
		POM_AM__set_application_bypass(false); // 关闭旁路
		return result.dump();
	}

	/*
	* 获取包含替代料的集合
	*/
	static void getContainSubItemList(SecondSourceInfo& bomBean, vector<string>& list) {
		vector<SecondSourceInfo> subList = bomBean.substitutesList;
		if (!bomBean.isDelete && !subList.empty()) { // 主料不是等待删除，并且替代料不为空
			list.push_back(bomBean.item);			
		}

		vector<SecondSourceInfo> children = bomBean.childs;
		if (children.empty()) {
			return;
		}

		for (auto& childBean : children) {
			getContainSubItemList(childBean, list);
		}
	}


	/*
	* 获取待删除料集合
	*/
	static void getPreDeleteList(SecondSourceInfo& bomBean, vector<string>& list) {
		bool preDelete = false;
		vector<SecondSourceInfo> subList = bomBean.substitutesList;
		if (!bomBean.isDelete && !subList.empty()) { // 主料不是等待删除，并且替代料不为空
			for (auto& subBean : subList) {
				bool subExistBom = subBean.subExistBom;
				bool checkStates = subBean.checkStates;
				if (subExistBom && !checkStates) { // 本身存在BOMLine中，但是未选中关系
					list.push_back(subBean.item);
					preDelete = true;
				}
			}

			if (preDelete) {
				list.push_back(bomBean.item);
			}
		}

		vector<SecondSourceInfo> children = bomBean.childs;
		if (children.empty()) {
			return;
		}

		for (auto& childBean : children) {
			getPreDeleteList(childBean, list);
		}
	}

	/*
	* 过滤全局替代料群组
	*/
	static void filterGrobalMat(vector<MatGroupInfo>& matInfoList) {
		for (auto it = matInfoList.begin(); it != matInfoList.end();) {
			MatGroupInfo target = *it;
			tag_t itemRev = target.matGroupItemRev;
			string bu = getPropValue(itemRev, d9_BU);
			string itemId = target.matGroupItemId;
			vector<string> splits = split(itemId, "-");
			if (splits.size() == 1) { // 移除物料群组ID不含有"-"的记录
				it = matInfoList.erase(it);
			} else if (isContainsStr(toLowCase(bu), enum_to_string(mnt)) || isContainsStr(toLowCase(bu), enum_to_string(prt))) { // 移除d9_BU属性填写MNT或PRT的记录
				it = matInfoList.erase(it);				
			} else {
				it++;
			}

			if (it == matInfoList.end()) {
				break;
			}			
		}
	}


	/*
	* 保存BOM结构树
	*/
	void saveBOMTree(SecondSourceInfo& bomBean, string parentId, vector<SecondSourceInfo>& list) {
		tag_t
			bomLine = NULLTAG;	
		vector<SecondSourceInfo>
			copySubList;

		bool addSubFlag = false; // 用作是否添加了替代料标识
		string bomLineUid = bomBean.bomLineUid;
		string item = bomBean.item;
		auto iter = bomMap.find(parentId + "$" + item);
		if (iter != bomMap.end()) {
			bomLine = iter->second;
		} else {
			ITK__convert_uid_to_tag(bomLineUid.c_str(), &bomLine); // 获取BOMLine
		}		
		if (bomBean.isDelete) { // 代表需要将此主料从BOM中移除
			ITK_THROW_ERROR(BOM_line_cut(bomLine)); 
			return;
		}

		vector<SecondSourceInfo> subList = bomBean.substitutesList;
		copySubList.assign(subList.begin(), subList.end()); // 数组的复制操作
		resetSubsProp(subList, bomBean); // 重新设置替代料属性
		if (!subList.empty()) {
			checkExist2ndSourceChange(bomLine, subList, bomBean, addSubFlag, list);
		}

		if (!subList.empty()) {
			addNew2ndSource(bomLine, subList, bomBean, addSubFlag, list);
		}
		if (subList.empty() && !addSubFlag && bomBean.alternativeCode.compare(PRI) == 0) { // 如果已經沒有了替代料需要設置，则将属性bl_occ_d9_AltGroup数值设为空
			ITK_THROW_ERROR(AOM_set_value_string(bomLine, d9_AltGroup, ""));
		}
		if (!checkSubListStates(subList) && !addSubFlag && bomBean.alternativeCode.compare(PRI) == 0) {
			ITK_THROW_ERROR(AOM_set_value_string(bomLine, d9_AltGroup, ""));
		}

		vector<SecondSourceInfo> children = bomBean.childs;
		if (children.empty()) {
			return;
		}

		filterSubChildEmpty(copySubList); // 移除替代料下阶不存在的记录
		for (auto& subBean: copySubList) { // 遍历替代料存在下阶BOM的情况
			vector<SecondSourceInfo> subChildList = subBean.childs;
			for (auto& subChildBean: subChildList) {
				saveBOMTree(subChildBean, subChildBean.item, subBean.childs);
			}
		}

		for (auto& childBean : children) {
			saveBOMTree(childBean, bomBean.item, bomBean.childs);
		}
	}

	/*
	* 判断已经存在的替代料是否发生改变
	*/
	int checkExist2ndSourceChange(tag_t& bomLine, vector<SecondSourceInfo>& subList, SecondSourceInfo& bomBean, bool& addSubFlag, vector<SecondSourceInfo>& list) {
		addSubFlag = false;
		bool hasDelete = false; // 判断是否发生移除替代料的动作，默认为没有发生
		int
			ifail = ITK_ok,
			sub_child_count = 0;		
		tag_t
			parentBomLine = NULLTAG,
			subItemRev = NULLTAG,
			mainItemRev = NULLTAG,
			matGroupItemRev = NULLTAG;
		tag_t			
			* sub_children = NULLTAG;
		vector<tag_t> matTagList; // 存放替代料群组对象集合
//		ITK_THROW_ERROR(BOM_line_list_substitutes(bomLine, &sub_child_count, &sub_children)); // 获取替代料BOM集合
//		vector<tag_t> existSubList = getTagList(sub_children, sub_child_count); // 将指针集合转换为vector
		string itemRevUid = bomBean.itemRevUid;		
		if (!subList.empty()) {
			ITK__convert_uid_to_tag(itemRevUid.c_str(), &mainItemRev); // 获取主料对象版本
			SecondSourceInfo subBean;
			matGroupItemRev = createCurProjectMatGroupItemRev(bomBean, subBean, list); // 创建/或查找当前替代料专案群组对象
			if (matGroupItemRev != NULL) {
				addOrDelCurProjectMatGroup(matGroupItemRev, mainItemRev, true); // 为当前替代料群组新增子料
			}
		}
		filterSubList(subList); // 移除不符合要求的记录

		for (auto it = subList.begin(); it != subList.end();) {
			SecondSourceInfo subBean = *it;
			if (!subBean.subExistBom) { // 如果替代料本身没存在BOMLine中，但是为选中状态，直接返回
				it++;
			} else if (subBean.itemRevUid.empty()) {
				it++;
			} else {
//				ITK__convert_uid_to_tag(subBean.itemRevUid.c_str(), &subItemRev); // 获取替代料的版本对象
				if (checkChange(bomLine, vector<SecondSourceInfo> {subBean})) { // 判断已经存在的替代料是否发生改变
					checkParentBOMLineStatus(window, bomLine, bomMap); // 判断父阶是否已经发行
					ITK_THROW_ERROR(AOM_ask_value_tag(bomLine, bomAttr_lineParentTag, &parentBomLine)); // 获取父BOMLine;
					string parentItemId = getPropValue(parentBomLine, bomAttr_itemId);
					auto iter = bomMap.find(parentItemId + "$" + bomBean.item);
					if (iter != bomMap.end()) {
						bomLine = iter->second;
					}					
				}

				// 无论是否发生升版还是没有升版,统一在此处获取替代料BOM集合
				ITK_THROW_ERROR(BOM_line_list_substitutes(bomLine, &sub_child_count, &sub_children)); // 获取替代料BOM集合
				vector<tag_t> existSubList = getTagList(sub_children, sub_child_count); // 将指针集合转换为vector

				int code = changeExistSubItemRelation(bomLine, existSubList, bomBean, subBean, addSubFlag, hasDelete, matTagList); // 改变已经存在的替代料的关系
				if (code == ITK_ok) {
					it = subList.erase(it);
				} else {
					it++;
				}

				if (it == subList.end()) {
					break;
				}
			}
		}

		if (matGroupItemRev != NULL && hasDelete) { // 存在替代料群组对象，并且操作是因为删除了所以的替代料，造成当前替代料群组只剩下一个主料才执行此操作
			checkCurProjectMatGroupItems(matGroupItemRev, mainItemRev); // 判断如果当前替代料群组只存在一个主料，则将直接移除此主料
		}

		//if (!matTagList.empty() && hasDelete) { // 存在替代料群组对象，并且操作是因为删除了所以的替代料，造成当前替代料群组只剩下一个主料才执行此操作
		//	for (auto& matGroupItemRev : matTagList) {
		//		if (matGroupItemRev != NULL) {
		//			checkCurProjectMatGroupItems(matGroupItemRev, mainItemRev); // 判断如果当前替代料群组只存在一个主料，则将直接移除此主料
		//		}
		//	}		
		//}
		
	CLEANUP:
		DOFREE(sub_children);
		return ifail;
	}

	/*
	* 改变已经存在的替代料的关系
	*/
	int changeExistSubItemRelation(tag_t& bomLine, vector<tag_t>& existSubList, SecondSourceInfo& bomBean, SecondSourceInfo& subBean, bool& addSubFlag, bool& hasDelete, vector<tag_t>& matTagList) {
		int
			ifail = ITK_ok;
		logical
			isTemporary = false;
		char			
			* sourceSubItemRevUid = NULL;
		tag_t
			matchSubItemRev = NULLTAG,
			sourceSubItemRev = NULLTAG,
			matGroupItemRev = NULLTAG;
		try {
			bool checkStates = subBean.checkStates;
			string altCode = subBean.alternativeCode;
			string subBeanItemUid = subBean.itemRevUid;
			ITK__convert_uid_to_tag(subBeanItemUid.c_str(), &matchSubItemRev);
			for (auto& subBomLine : existSubList) {
				ITK_THROW_ERROR(AOM_ask_value_tag(subBomLine, bomAttr_lineObjectTag, &sourceSubItemRev));
				ITK__convert_tag_to_uid(sourceSubItemRev, &sourceSubItemRevUid);
				if (tc_strcmp(sourceSubItemRevUid, subBeanItemUid.c_str()) == 0) {
					if (!checkStates) {
						ITK_THROW_ERROR(BOM_line_cut(subBomLine)); // 将此BOMLine移除
						vector<SecondSourceInfo> list;
						matGroupItemRev = createCurProjectMatGroupItemRev(bomBean, subBean, list); // 创建/或查找当前替代料专案群组对象	
						if (matGroupItemRev != NULL) {
							addOrDelCurProjectMatGroup(matGroupItemRev, matchSubItemRev, false); // 为当前替代料群组移除替代料
						}
						hasDelete = true;
						//matTagList = getAllTotalCurProjMatGroupItemRevList(bomBean, subBean); // 获取当前所有的专案群组
						//if (!matTagList.empty()) {
						//	for (auto& matGroupItemRev : matTagList) {
						//		if (matGroupItemRev != NULL) {
						//			addOrDelCurProjectMatGroup(matGroupItemRev, matchSubItemRev, false); // 为当前替代料群组移除替代料
						//		}
						//		hasDelete = true;
						//	}
						//}
					} else {
						if (altCode.compare(PRI) == 0) { // 判断当前替代料是否需要设置为主料
							ITK_THROW_ERROR(BOM_line_prefer_substitute(subBomLine, &isTemporary)); // 将替代料设置为主料
							bomLine = subBomLine;
							//setBomProps(bomLine, subBean); // 设置BOM属性
						} 

						setBomProps(bomLine, subBean); // 设置BOM属性
						/*else {
							setBomProps(bomLine, subBean);
						}*/

						addSubFlag = true;
					}
				}
			}
		 
		} catch (int& error_code) {
			ifail = error_code;
			addSubFlag = false;
		}
		
	CLEANUP:
		DOFREE(sourceSubItemRevUid);
		if (ifail != ITK_ok) {
			throw ifail;
		}
		return ifail;
	}


	/*
	* 添加新的替代料并且将新料加入到专案替代料群组中
	*/
	void addNew2ndSource(tag_t& bomLine, vector<SecondSourceInfo>& subList, SecondSourceInfo& bomBean, bool& addSubFlag, vector<SecondSourceInfo>& list) {
		for (auto it = subList.begin(); it != subList.end();) {
			tag_t subItemRev = NULLTAG;
			SecondSourceInfo subBean = *it;
			string itemId = subBean.item;
			bool checkStates = subBean.checkStates;
			if (subBean.subExistBom) { // 如果替代料本身没存在BOMLine中，但是为选中状态，直接返回
				it++;
			} else {
				if (toUpperCase(subBean.sourceSystem).compare(PNMS) == 0 && checkStates) { // 物料来源于PNMS，并且已经勾选
					createItemFromPNMS(subBean, itemId);
				} 
				ITK__convert_uid_to_tag(subBean.itemRevUid.c_str(), &subItemRev);
				int code = add2ndSourceAndSetRelation(bomLine, bomBean, subBean, subItemRev, addSubFlag, list);
				if (code == ITK_ok) {
					it = subList.erase(it);
				} else {
					it++;
				}

				if (it == subList.end()) {
					break;
				}
			}
		}
	}


	/*
	* 添加2nd Source并且设置关系(是否需要将2nd Source变成主料)
	*/
	int add2ndSourceAndSetRelation(tag_t& bomLine, SecondSourceInfo& bomBean, SecondSourceInfo& subBean, tag_t& subItemRev, bool& addSubFlag, vector<SecondSourceInfo>& list) {
		int
			ifail = ITK_ok,
			sub_child_count = 0;
		tag_t
			matGroupItemRev = NULLTAG,
			subItem = NULLTAG,
			parentBomLine = NULLTAG,
			newSubBomLine = NULLTAG;
		tag_t
			* sub_children = NULLTAG;
		try {
			bool checkStates = subBean.checkStates;
			if (checkStates) {
				checkParentBOMLineStatus(window, bomLine, bomMap); // 判断父阶是否已经发行
				ITK_THROW_ERROR(AOM_ask_value_tag(bomLine, bomAttr_lineParentTag, &parentBomLine)); // 获取父BOMLine;
				string parentItemId = getPropValue(parentBomLine, bomAttr_itemId);
				string mainItem = bomBean.item;
				auto iter = bomMap.find(parentItemId + "$" + mainItem);
				if (iter != bomMap.end()) {
					bomLine = iter->second;
				}

				ITK_THROW_ERROR(ITEM_ask_item_of_rev(subItemRev, &subItem));
				ITK_THROW_ERROR(BOM_line_add_substitute(bomLine, subItem, subItemRev, NULLTAG, &newSubBomLine)); // 为当前BOMLine添加替代料
				setBomProps(bomLine, subBean); // 设置BOM属性

				ITK_THROW_ERROR(BOM_line_list_substitutes(bomLine, &sub_child_count, &sub_children)); // 获取替代料BOM集合
				vector<tag_t> existSubList = getTagList(sub_children, sub_child_count); // 将指针集合转换为vector
				if (!existSubList.empty()) {
					setPrimaryMaterial(bomLine, existSubList, subBean); // 设置替代料为主料
				}			

				matGroupItemRev = createCurProjectMatGroupItemRev(bomBean, subBean, list);
				if (matGroupItemRev != NULL) {
					addOrDelCurProjectMatGroup(matGroupItemRev, subItemRev, true); // 为当前替代料群组新增子料 
				}

				addSubFlag = true;
			}
		} catch (int& error_code) {
			ifail = error_code;
			addSubFlag = false;			 
		}	

	CLEANUP:
		DOFREE(sub_children);
		if (ifail != ITK_ok) {
			throw ifail;
		}

		return ifail;
	}

	/*
	* 在TC中创建来源于PNMS系统的物料
	*/
	int createItemFromPNMS(SecondSourceInfo& subBean, string& itemId) {
		int
			ifail = ITK_ok;
		char* itemRevUidChar = NULL;
		tag_t
			item = NULLTAG,
			itemRev = NULLTAG;

		map<string, string> propMap;
		vector<string>
			propNameList,
			propValueList;
		ITK_THROW_ERROR(ITEM_find_item(itemId.c_str(), &item));
		if (item != NULL) {
			ITK_THROW_ERROR(ITEM_ask_latest_rev(item, &itemRev)); // 根据对象获取最大的对象版本			
		} else {
			writeLog("create pnms -->> %s\n", itemId.c_str());
			ITK_THROW_ERROR(createItem(itemId, itemId, EDAComPart, LetterPattern, &item, &itemRev));

			propMap.insert(pair<string, string>(d9_EnglishDescription, subBean.description));
			propMap.insert(pair<string, string>(d9_ManufacturerID, subBean.mfg));
			propMap.insert(pair<string, string>(d9_ManufacturerPN, subBean.mfgPn));
			propMap.insert(pair<string, string>(d9_Un, subBean.unit));
			propMap.insert(pair<string, string>(d9_MaterialType, subBean.materialType));
			propMap.insert(pair<string, string>(d9_MaterialGroup, subBean.materialGroup));
			propMap.insert(pair<string, string>(d9_ProcurementMethods, subBean.procurementType));

			batchUpdataProp(itemRev, propMap); // 批量更新属性
		}

		ITK__convert_tag_to_uid(itemRev, &itemRevUidChar);
		subBean.itemRevUid = (string)itemRevUidChar;

	CLEANUP:
		DOFREE(itemRevUidChar);
		return ifail;
	}


	/*
	* 设置替代料为主料
	*/
	void setPrimaryMaterial(tag_t& bomLine, vector<tag_t>& existSubList, SecondSourceInfo& subBean) {
		char
			* subItemRevUid = NULL;
		logical
			isTemporary = false;
		tag_t
			subItemRev = NULLTAG;

		string altCode = subBean.alternativeCode;
		for (auto& subBomLine : existSubList) {
			ITK_THROW_ERROR(AOM_ask_value_tag(subBomLine, bomAttr_lineObjectTag, &subItemRev));
			ITK__convert_tag_to_uid(subItemRev, &subItemRevUid);
			if (tc_strcmp(subItemRevUid, subBean.itemRevUid.c_str()) == 0) {
				if (altCode.compare(PRI) == 0) {
					ITK_THROW_ERROR(BOM_line_prefer_substitute(subBomLine, &isTemporary)); // 将替代料设置为主料
					bomLine = subBomLine;
					setBomProps(bomLine, subBean); // 重新设置一下主料群组信息
				}
			}
		}

	CLEANUP:
		DOFREE(subItemRevUid);
	}

	/*
	* 获取当前所有的专案群组
	*/
	vector<tag_t> getAllTotalCurProjMatGroupItemRevList(SecondSourceInfo& bomBean, SecondSourceInfo& subBean) {
		vector<tag_t> tagList;
		int
			ifail = ITK_ok,
			resultLength = 0;
		for (auto& info : matInfoList) {
			tag_t itemRev = info.matGroupItemRev;
			string matGroupItemId = info.matGroupItemId;
			string matchStr = "-" + level + "-" + toUpperCase(curProjectID) + "-" + bomBean.parentItem;
			if (isContainsStr(matGroupItemId, matchStr)) {				
				tagList.push_back(itemRev);
			}
		}
	CLEANUP:
		return tagList;
	}


	/*
	* 创建当前替代料专案群组对象
	*/
	tag_t createCurProjectMatGroupItemRev(SecondSourceInfo& bomBean, SecondSourceInfo& subBean, vector<SecondSourceInfo>& list) {
		int
			ifail = ITK_ok;
		tag_t
			item = NULLTAG,
			itemRev = NULLTAG;
		string matGroupItemId = "";

		matGroupItemId = getMatGroupItemIdByMatInfoList(bomBean);
		if (matGroupItemId.empty()) {
			matGroupItemId = bomBean.materialGroupItemId;
		}
//		string matGroupItemId = bomBean.materialGroupItemId;
		if (matGroupItemId.empty()) {
			if (list.empty()) {
				return NULL;
			}
			filterContailMatGroupID(list); // 过滤包含替代料群组ID属性值
			int index = getIndexByItemUid(list, bomBean); // 返回主料所在的索引
			if (index == -1) {
				return NULL;
			} else {
				string matCode = getSelfMatGroupCode(index);
				matGroupItemId = matCode + "-" + level + "-" + toUpperCase(curProjectID) + "-" + bomBean.parentItem;
			}			
		}

		if (!isContainsStr(matGroupItemId, toUpperCase(curProjectID))) { // 判断替代料群组ID是否含有当前专案ID
			matGroupItemId += "-" + level + "-" + toUpperCase(curProjectID) + "-" + bomBean.parentItem;
		}

		bomBean.materialGroupItemId = matGroupItemId;
		subBean.materialGroupItemId = matGroupItemId;
		ITK_THROW_ERROR(ITEM_find_item(matGroupItemId.c_str(), &item)); // 通过ID查询Item
		if (item != NULL) {
			ITK_THROW_ERROR(ITEM_ask_latest_rev(item, &itemRev)); // 根据对象获取最大的对象版本
		} else {
			ITK_THROW_ERROR(createItem(matGroupItemId, matGroupItemId, D9_MaterialGroup, LetterPattern, &item, &itemRev));
			createBomView(item, itemRev); // 创建BOM视图
		}

		return itemRev;
	}


	/*
	* 通过替代料群组集合查找符合条件的替代料群组ID
	*/
	string getMatGroupItemIdByMatInfoList(SecondSourceInfo& bomBean) {
		string matchGroupItemId = "";
		string matchStr = "-" + level + "-" + toUpperCase(curProjectID) + "-" + bomBean.parentItem;
		auto it1 = find_if(matInfoList.begin(), matInfoList.end(), [&](const MatGroupInfo& matInfo) {
			string matGroupItemId = matInfo.matGroupItemId;
			return startWithStr(matGroupItemId, MGT) && isContainsStr(matGroupItemId, matchStr); // 替代料群组是以自编物料群组MGT开头,并且含有专案ID信息
			//if (startWithStr(matGroupItemId, MGT)) { // 替代料群组是以自编物料群组MGT开头
			//	if (isContainsStr(matGroupItemId, matchStr)) {
			//		return true;
			//	}
			//} else {
			//	return false;
			//}
		});

		if (it1 != matInfoList.end()) {
			MatGroupInfo target1 = *it1;
			matchGroupItemId = target1.matGroupItemId;
			//return target.matGroupItemId;
		} else {
			auto it2 = find_if(matInfoList.begin(), matInfoList.end(), [&](const MatGroupInfo& matInfo) {
				string matGroupItemId = matInfo.matGroupItemId;
				return isContainsStr(matGroupItemId, matchStr);
				/*if (isContainsStr(matGroupItemId, matchStr)) {
					return true;
				} else {
					return false;
				}*/
			});

			if (it2 != matInfoList.end()) {
				MatGroupInfo target2 = *it2;
				matchGroupItemId = target2.matGroupItemId;
				//return target2.matGroupItemId;
			}
		}	

		return matchGroupItemId;
	}


	/*
	* 为当前替代料群组新增或者删除子料(true代表新增, false代表删除)
	*/
	int addOrDelCurProjectMatGroup(tag_t& matGroupItemRev, tag_t& needCheckItemRev, bool operation) {
		int
			ifail = ITK_ok,
			child_count = 0;
		char
			* existUid = NULL,
			* needCheckUid = NULL; // 需要判断是否加入的版本UID
		tag_t
			needCheckItem = NULLTAG,
			existItemRev = NULLTAG,
//			matGroupItem = NULLTAG,
			rev_rule = NULLTAG,
			window = NULLTAG,
			topLine = NULLTAG,
			newBomLine = NULLTAG;
		tag_t
			* children = NULLTAG;

		vector<tag_t> tagList;
		try {
			ITK__convert_tag_to_uid(needCheckItemRev, &needCheckUid); // 获取uid
			ITK_THROW_ERROR(ITEM_ask_item_of_rev(needCheckItemRev, &needCheckItem));
//			ITK_THROW_ERROR(ITEM_ask_item_of_rev(matGroupItemRev, &matGroupItem));
			ITK_THROW_ERROR(CFM_find(Latest_Working, &rev_rule));
			ITK_THROW_ERROR(BOM_create_window(&window));
			ITK_THROW_ERROR(BOM_set_window_config_rule(window, rev_rule));
			ITK_THROW_ERROR(BOM_set_window_pack_all(window, false)); // 解包
			ITK_THROW_ERROR(BOM_set_window_top_line(window, NULLTAG, matGroupItemRev, NULL, &topLine));
			ITK_THROW_ERROR(BOM_line_ask_child_lines(topLine, &child_count, &children));
			tagList = getTagList(children, child_count); // 将指针对象转换集合
			if (operation) { // true代表是新增操作
				if (tagList.empty()) {
					ITK_THROW_ERROR(BOM_line_add(topLine, NULLTAG, needCheckItemRev, NULLTAG, &newBomLine)); // 添加主料
				} else {
					auto flag = find_if(tagList.begin(), tagList.end(), [&](const tag_t& bomLine) {
						ITK_THROW_ERROR(AOM_ask_value_tag(bomLine, bomAttr_lineObjectTag, &existItemRev));
						ITK__convert_tag_to_uid(existItemRev, &existUid); // 获取uid
						return tc_strcmp(existUid, needCheckUid) == 0;
					});

					bool found = flag != tagList.end();
					if (!found) { // 判断一下此料是否已经在替代料群组中
						ITK_THROW_ERROR(BOM_line_add(topLine, NULLTAG, needCheckItemRev, NULLTAG, &newBomLine)); // 添加主料
					}
				}
			} else { // 代表是移除
				if (tagList.empty()) {
					goto CLEANUP;
				} else {
					for (auto childBomLine : tagList) {
						ITK_THROW_ERROR(AOM_ask_value_tag(childBomLine, bomAttr_lineObjectTag, &existItemRev));
						ITK__convert_tag_to_uid(existItemRev, &existUid); // 获取uid
						if (tc_strcmp(existUid, needCheckUid) == 0) { // 判断此料是否已经在替代料群组中
							ITK_THROW_ERROR(BOM_line_cut(childBomLine)); // 剪切
						}
					}
				}
			}
			
		} catch (int& error_code) {
			ifail = error_code;
		}

		if (window != NULLTAG) {
			ITK_THROW_ERROR(BOM_save_window(window)); // 保存窗口
			ITK_THROW_ERROR(BOM_close_window(window)); // 关闭窗口
		}
	CLEANUP:
		DOFREE(existUid);
		DOFREE(needCheckUid);
		DOFREE(children);
		if (ifail != ITK_ok) {
			throw ifail;
		}
		return ifail;
	}


	/*
	* 判断如果当前替代料群主只存在一个主料，则将直接移除此主料
	*/
	int checkCurProjectMatGroupItems(tag_t& matGroupItemRev, tag_t& mainItemRev) {
		int
			ifail = ITK_ok,
			child_count = 0;
		char
			* existUid = NULL,
			* mainItemRevUid = NULL; // 主料UID
		tag_t
			existItemRev = NULL,
			rev_rule = NULLTAG,
			window = NULLTAG,
			topLine = NULLTAG,
			newBomLine = NULLTAG;
		tag_t
			* children = NULLTAG;

		vector<tag_t> tagList;
		try {
//			ITK__convert_tag_to_uid(mainItemRev, &mainItemRevUid); // 获取uid
			ITK_THROW_ERROR(CFM_find(Latest_Working, &rev_rule));
			ITK_THROW_ERROR(BOM_create_window(&window));
			ITK_THROW_ERROR(BOM_set_window_config_rule(window, rev_rule));
			ITK_THROW_ERROR(BOM_set_window_pack_all(window, false)); // 解包
			ITK_THROW_ERROR(BOM_set_window_top_line(window, NULLTAG, matGroupItemRev, NULL, &topLine));
			ITK_THROW_ERROR(BOM_line_ask_child_lines(topLine, &child_count, &children));
			tagList = getTagList(children, child_count); // 将指针对象转换集合
			if (tagList.size() == 1) { // 判断当前替代料群组ID只剩下一颗料
				tag_t childBomLine = tagList[0];
				ITK_THROW_ERROR(AOM_ask_value_tag(childBomLine, bomAttr_lineObjectTag, &existItemRev));
				if (mainItemRev == existItemRev) {
					ITK_THROW_ERROR(BOM_line_cut(childBomLine)); // 剪切
				}
			}		

		} catch (int& error_code) {
			ifail = error_code;
		}

		if (window != NULLTAG) {
			ITK_THROW_ERROR(BOM_save_window(window)); // 保存窗口
			ITK_THROW_ERROR(BOM_close_window(window)); // 关闭窗口
		}
	CLEANUP:
		DOFREE(existUid);
		DOFREE(children);
		if (ifail != ITK_ok) {
			throw ifail;
		}
		return ifail;
	}


	/*
	* 移除包含替代料群组ID的记录
	*/
	static void filterContailMatGroupID(vector<SecondSourceInfo>& list) {
		for (auto it = list.begin(); it != list.end();) {
			SecondSourceInfo target = *it;
			string matGroupItemId = target.materialGroupItemId;
			if (!matGroupItemId.empty()) {
				it = list.erase(it);
			} else {
				it++;
			}

			if (it == list.end()) {
				break;
			}
		}
	}


	/*
	* 通过对象uid返回所在的索引
	*/
	static int getIndexByItemUid(vector<SecondSourceInfo>& list, SecondSourceInfo& bomBean) {
		int index = -1;
		for (int i = 0; i < list.size(); i++) {
			SecondSourceInfo target = list[i];
			if (target.itemRevUid.compare(bomBean.itemRevUid) == 0) {
				index = i;
				index++;
				break;
			}
		}
		return index;
	}

	/*
	* 获取自编流水码
	*/
	static string getSelfMatGroupCode(int index) {
		string str = to_string(index); // int转string
		int length = str.length(); // 获取字符串的长度
		int prefixCount = 6 - length; // 前缀长度
		string matCode = MGT;
		for (int i = 0; i < prefixCount; i++) {
			matCode += "0";
		}
		matCode = matCode + str;
		return matCode;
	}


	/*
	* 过滤掉不符合条件的记录
	*/
	static void filterSubList(vector<SecondSourceInfo>& subList) {
		for (auto it = subList.begin(); it != subList.end();) {
			SecondSourceInfo target = *it;
			string itemRevUid = target.itemRevUid;
			bool subExistBom = target.subExistBom;
			bool checkStates = target.checkStates;
			if (!subExistBom && !checkStates) { // 对于替代料本身没有存在BOMLine中，但是为未选中，则直接将此记录从集合中移除
				it = subList.erase(it);
			} else if (itemRevUid.empty() && !checkStates) { //来源于PNMS系统查询的记录，并且是未选中状态，则直接将此记录从集合中移除
				it = subList.erase(it);
			} else {
				it++;
			}

			if (it == subList.end()) {
				break;
			}
		}
	}

	/*
	* 移除替代料不含有子的记录
	*/
	static void filterSubChildEmpty(vector<SecondSourceInfo>& subList) {
		for (auto it = subList.begin(); it != subList.end();) {
			SecondSourceInfo target = *it;
			vector<SecondSourceInfo> childs = target.childs;
			if (childs.empty()) {
				it = subList.erase(it);
			} else {
				it++;
			}

			if (it == subList.end()) {
				break;
			}
		}
	}


	/*
	* 重新设置替代料Location和Qty属性
	*/
	static void resetSubsProp(vector<SecondSourceInfo>& subList, SecondSourceInfo& bomBean) {
		for (auto& subBean : subList) {
			subBean.location = bomBean.location;
			subBean.qty = bomBean.qty;
		}
	}

	/*
	* 校验替代料集合中是否有被选中的记录
	*/
	static bool checkSubListStates(vector<SecondSourceInfo>& subList) {
		bool flag = false;
		for (auto& subBean : subList) {
			bool checkStates = subBean.checkStates;
			if (checkStates) {
				flag = true;
				break;
			}
		}

		return flag;
	}
	/*
	* 设置BOM属性
	*/
	static void setBomProps(tag_t& bomLine, SecondSourceInfo& subBean) {
		ITK_THROW_ERROR(AOM_set_value_string(bomLine, d9_AltGroup, subBean.alternativeGroup.c_str()));
		if (subBean.hasMerge) { // 当替代料对应的主料发生位置和用量合并，才重新设置位置和数量属性
			ITK_THROW_ERROR(AOM_set_value_string(bomLine, quantity, subBean.qty.c_str()));
			ITK_THROW_ERROR(AOM_set_value_string(bomLine, d9_Location, subBean.location.c_str()));
		}
		
	}

#ifdef __cplusplus
}
#endif
