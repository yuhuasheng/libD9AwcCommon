#include "secondSourceCommonUtils.h"
using namespace MNTEBOM2ndSource;
#ifdef __cplusplus
extern "C" {
#endif	

	static std::tr1::regex pEndPattern("^.+?_P\\d+$"); // 匹配是否以_P+数字结尾

	static void filterMatGroupList(vector<MatGroupInfo>& matInfoList, string syncFrom);

	static int getGlobal2ndSource(vector<MatGroupInfo>& matInfoList, vector<tag_t>& list, vector<string>& matGroupList, string& item, string& groupName);

	static int getProject2ndSource(vector<MatGroupInfo>& matInfoList, vector<tag_t>& list, vector<string>& matGroupList, string& parentItem, string& item,
		string& level, string& projectID);

	static void checkExist(SecondSourceInfo& baseBean, vector<SecondSourceInfo>& list);

	static vector<string> getItemIdList(SecondSourceInfo& bomBean);

	static string getChildIds(vector<string>& list);

	BOMTYPE MNTEBOM2ndSource::getBOMType(tag_t bomLine) {
		tag_t
			window = NULLTAG,
			top_line = NULLTAG,
			topItemRev = NULLTAG;

		ITK_THROW_ERROR(BOM_line_ask_window(bomLine, &window)); // 通过BOMLine获取window
//		ITK_THROW_ERROR(AOM_ask_value_tag(bomLine, bomAttr_lineWindowTag, &window)); // 通过顶阶BOMLine获取window对象

		ITK_THROW_ERROR(BOM_ask_window_top_line(window, &top_line)); // 通过BOMWindow获取顶阶BOMLine
		ITK_THROW_ERROR(AOM_ask_value_tag(top_line, line_object, &topItemRev));
		string topId = getPropValue(top_line, item_id);
		if (startWithStr(topId, "8")) {
			return BOMTYPE::MNT_L10;
		}

		return BOMTYPE::MNT_L6;
	}

	bool MNTEBOM2ndSource::isExpandChilds(tag_t bom_line, string materialGroup) {
		if (materialGroup.compare("B8X80") == 0) {
			BOMTYPE bomType = getBOMType(bom_line);
			if (BOMTYPE::MNT_L10 == bomType) {
				return false;
			}
		}
		return true;
	}

	int MNTEBOM2ndSource::getTcProp(MNTEBOM2ndSource::SecondSourceInfo& info, tag_t& bomLine) {
		int
			ifail = ITK_ok;

		char
			* itemRevUid = NULL,
			* bomLineUid = NULL;

		tag_t
			itemRev = NULLTAG;

		ITK_THROW_ERROR(AOM_ask_value_tag(bomLine, line_object, &itemRev));

		ITK__convert_tag_to_uid(bomLine, &bomLineUid);
		ITK__convert_tag_to_uid(itemRev, &itemRevUid);

		info.findNum = getPropValue(bomLine, sequence_no).compare("") == 0 ? "0" : getPropValue(bomLine, sequence_no);
		info.item = getPropValue(itemRev, item_id);
		info.version = getPropValue(itemRev, current_revision_id);
		info.description = getPropValue(itemRev, d9_EnglishDescription);
		info.sapDescription = getPropValue(itemRev, d9_DescriptionSAP);
		info.mfg = getPropValue(itemRev, d9_ManufacturerID);
		info.mfgPn = getPropValue(itemRev, d9_ManufacturerPN);
		info.location = getPropValue(bomLine, d9_Location);
		info.alternativeGroup = getPropValue(bomLine, d9_AltGroup);
		info.materialGroup = getPropValue(itemRev, d9_MaterialGroup);
		info.materialType = getPropValue(itemRev, d9_MaterialType);
		info.procurementType = getPropValue(itemRev, d9_ProcurementMethods);
		info.qty = getPropValue(bomLine, quantity);
		info.unit = getPropValue(itemRev, d9_Un);
		info.status = getReleaseName(itemRev);
		info.referenceDimension = getPropValue(bomLine, d9_ReferenceDimension);
		info.sapRev = getPropValue(itemRev, d9_SAPRev);
		info.packageType = getPropValue(bomLine, d9_PackageType);
		info.side = getPropValue(bomLine, d9_Side);
		info.supplierZF = getPropValue(itemRev, d9_SupplierZF);
		info.bomLineUid = (string)bomLineUid;
		info.uid = (string)bomLineUid;
		info.itemRevUid = (string)itemRevUid;
	CLEANUP:
		DOFREE(bomLineUid);
		DOFREE(itemRevUid);
		return ifail;
	}

	int MNTEBOM2ndSource::checkVerCount(tag_t& itemRev, string& checkResult) {
		writeLog("Start handler checkVerCount\n");
		list<int> intList;
		list<string> strList;

		tag_t
			item = NULLTAG;
		int
			count = 0,
			ifail = ITK_ok;

		tag_t
			* rev_list = NULLTAG;

		checkResult = FIRSTVERNOTE;
		string currentVer = getPropValue(itemRev, current_revision_id);

		ITK_THROW_ERROR(ITEM_ask_item_of_rev(itemRev, &item)); // 对象版本获取对象

		ITK_THROW_ERROR(ITEM_list_all_revs(item, &count, &rev_list)); // 获取对象的所有版本

		for (size_t i = 0; i < count; i++) {
			string ver = getPropValue(rev_list[i], current_revision_id);
			std::tr1::regex regnumber("^[0-9]+$"); //匹配是否是数字
			std::tr1::regex regletter("^[a-zA-Z]+$"); //配置是否是字母

			if (regex_matchstr(ver, regnumber) == 1) { //判断版本是否为数字版	
				intList.push_back(std::stoi(ver));
			} else if (regex_matchstr(ver, regletter) == 1) { // 判断版本是否为字母版
				strList.push_back(ver);
			}
		}
		auto maxString = std::max_element(strList.begin(), strList.end(), compareStrings);

		if (intList.size() > 1) { // 代表最低有两个版本
			auto maxElement = std::max_element(intList.begin(), intList.end());
			auto minElement = std::min_element(intList.begin(), intList.end());
			if (std::stoi(currentVer) == (*maxElement)) { // 判断是否为最大版
				checkResult = NOTFIRSTVERNOTE;
			} else if (std::stoi(currentVer) != (*minElement)) { // 判断是否为最小版本
				checkResult = NOTFIRSTVERNOTE;
			}
		} else if (strList.size() > 1) {
			auto maxString = std::max_element(strList.begin(), strList.end(), compareStrings);
			auto minString = std::min_element(strList.begin(), strList.end(), compareStrings);
			if (tc_strcmp(currentVer.c_str(), (*maxString).c_str()) == 0) { // 判断是否为最大版本
				checkResult = NOTFIRSTVERNOTE;
			} else if (tc_strcmp(currentVer.c_str(), (*minString).c_str()) != 0) { // 判断是否为最小版本
				checkResult = NOTFIRSTVERNOTE;
			}
		}

		writeLog("End handler checkVerCount\n");

	CLEANUP:
		return ifail;
	}

	string MNTEBOM2ndSource::getAltGroupByFindNum(SecondSourceInfo& bean) {
		string altGroup = getAltGroupByRule(std::stoi(bean.findNum));
		if (altGroup.empty()) {
			string msg = "零组件ID为: " + bean.item + ", 版本号为: " + bean.version + " findNum为: " + bean.findNum + ", 获取替代料群组失败!";
			writeLog("%s\n", msg);
			throw exception(msg.c_str());
		}
		return altGroup;
	}

	string MNTEBOM2ndSource::getAltGroupByRule(int findNum) {
		string altGroup = "";
		if (findNum == NULL) {
			return "";
		} else if (findNum >= 10 && findNum < 1000) {
			altGroup = to_string(findNum);
			altGroup.pop_back();
		} else if (findNum >= 1000 && findNum <= 3599) {
			string str = to_string(findNum);
			int first = atoi(str.substr(0, 2).c_str());
			first += 55;

			string letter = "";
			letter.push_back(char(first));


			if (strcmp(letter.c_str(), "I") == 0 || strcmp(letter.c_str(), "O") == 0) {
				first++;
				letter.push_back(char(first));
			}

			str = str.substr(2, str.length() - 1);

			str.pop_back();
			altGroup = letter + str;
		}

		return altGroup;
	}

	int MNTEBOM2ndSource::getCurMatGroupItemId(SecondSourceInfo& info, string& matGroupItemId, string level, string projectId) {
		clock_t start, finish;
		double totalTime;
		start = clock();
		int
			ifail = ITK_ok,
			resultLength = 0;

		tag_t
			matGroupItemRev = NULLTAG;

		tag_t
			* queryResults = NULLTAG;

		string itemId = info.item;
		string parentItem = info.parentItem;
		/*if (itemId.empty() || parentItem.empty()) {
			return ITK_ok;
		}*/

		if (itemId.empty()) {
			return ITK_ok;
		}

		writeLog("find parts param: %s", itemId);

		char** qry_entries = (char**)MEM_alloc(sizeof(char**));
		char** qry_values = (char**)MEM_alloc(sizeof(char**));
		qry_entries[0] = (char*)MEM_alloc(sizeof(char*) * 200);
		qry_values[0] = (char*)MEM_alloc(sizeof(char*) * 200);

		tc_strcpy(qry_entries[0], Child_ID);
		tc_strcpy(qry_values[0], itemId.c_str());

		ITK_THROW_ERROR(query(D9_Find_MaterialGroup, qry_entries, qry_values, 1, &queryResults, &resultLength));
		writeLog(" -- >> materialGroup itemId : %s queryresult : %d \n", itemId, resultLength);

		for (int i = 0; i < resultLength; i++) {
			matGroupItemRev = queryResults[i];
			string itemId = getPropValue(matGroupItemRev, item_id);
			if (isContainsStr(toUpperCase(itemId), level + toUpperCase(projectId) + parentItem)) { // 判断是否包含
				matGroupItemId = itemId;
				break;
			} else if (isContainsStr(toUpperCase(itemId), toUpperCase(projectId))) { // 判断是否包含
				matGroupItemId = itemId;
				break;
			}
		}

	CLEANUP:
		DOFREE(qry_entries);
		DOFREE(qry_values);
		DOFREE(queryResults);
		finish = clock();
		totalTime = (double)(finish - start) / CLOCKS_PER_SEC;
		writeLog("getCurMatGroupItemId cost time \n");
		writeLog("totalTime %.5lf\n", totalTime);
		return ifail;
	}

	int MNTEBOM2ndSource::getCurMatGroupItemIdByMatch(vector<MatGroupInfo>& matInfoList, string& matGroupItemId, string& parentItem, string& item, string level, string projectId) {
		clock_t start, finish;
		double totalTime;
		start = clock();
		int
			ifail = ITK_ok;
		for (auto& info : matInfoList) {
			if (item.compare(info.childItemId) == 0) {
				if (item.compare("62011GL03-015-H") == 0) {
					cout << "123" << endl;
				}
				string matItemId = info.matGroupItemId;
				if (isContainsStr(toUpperCase(matItemId), level + toUpperCase(projectId) + parentItem)) { // 判断是否包含
					matGroupItemId = matItemId;
					break;
				} else if (isContainsStr(toUpperCase(matItemId), toUpperCase(projectId))) { // 判断是否包含
					matGroupItemId = matItemId;
					break;
				}
			}
		}
	CLEANUP:
		finish = clock();
		totalTime = (double)(finish - start) / CLOCKS_PER_SEC;
		writeLog("getCurMatGroupItemIdByMatch cost time \n");
		writeLog("totalTime %.5lf\n", totalTime);
		return ifail;
	}

	bool MNTEBOM2ndSource::checkJsonItemExist(json& data, json& jsonItem, string key) {
		for (auto& target : data) {
			if (target[key].get<std::string>().compare(jsonItem[key].get<std::string>()) == 0) {
				return true;
			}
		}
		return false;

		/*for (auto it = data.begin(); it != data.end();) {
			json target = *it;
			if (target[key].get<std::string>().compare(jsonItem[key].get<std::string>()) == 0) {
				return false;
			}
		}
		return true;*/
	}

	int MNTEBOM2ndSource::removeSameBean(vector<SecondSourceInfo>& list, string uid) {
		clock_t start, finish;
		double totalTime;
		start = clock();
		int
			ifail = ITK_ok;
		for (auto it = list.begin(); it != list.end();) {
			SecondSourceInfo target = *it;
			if (tc_strcmp(target.itemRevUid.c_str(), uid.c_str()) == 0) {
				it = list.erase(it); // 删除元素，返回值指向已删除元素的下一个位置
				if (it == list.end()) {
					break;
				}
			} else {
				it++;
			}
		}
		finish = clock();
		totalTime = (double)(finish - start) / CLOCKS_PER_SEC;
		writeLog("removeSameBean cost time \n");
		writeLog("totalTime %.5lf\n", totalTime);
		return ifail;
	}

	bool MNTEBOM2ndSource::compareItemUid(const SecondSourceInfo& firstInfo, const SecondSourceInfo& secondInfo) {
		return firstInfo.itemRevUid < secondInfo.itemRevUid;
	}

	int MNTEBOM2ndSource::distinctBeanByUid(vector<SecondSourceInfo>& list) {
		clock_t start, finish;
		double totalTime;
		start = clock();
		int
			ifail = ITK_ok;
		std::sort(list.begin(), list.end(), compareItemUid); // 首先进行排序
		list.erase(std::unique(list.begin(), list.end(), [](const SecondSourceInfo& firstInfo, const SecondSourceInfo& secondInfo) {
			return firstInfo.itemRevUid.compare(secondInfo.itemRevUid) == 0;
			}), list.end());

		finish = clock();
		totalTime = (double)(finish - start) / CLOCKS_PER_SEC;
		writeLog("distinctBeanByUid cost time \n");
		writeLog("totalTime %.5lf\n", totalTime);
		return ifail;
	}

	int MNTEBOM2ndSource::getTotal2ndSourceSubList(vector<MatGroupInfo>& matInfoList, vector<SecondSourceInfo>& totalSubList, tag_t& bomLine, SecondSourceInfo& bomBean, string syncFrom, string groupName, string level,
		string projectID, bool syncFlag, bool enable) {
		int
			ifail = ITK_ok;

		tag_t
			childItemRev = NULLTAG;

		logical
			isSub = false,
			hasSub = false;

		vector<SecondSourceInfo>
			sourceGroupList;

		vector<tag_t>
			searchList;

		vector<string>
			matGroupList;

		ITK_THROW_ERROR(BOM_line_ask_has_substitutes(bomLine, &hasSub)); // 判断是否含有替代料
		if (hasSub) {
			getExistSubBeanList(bomLine, totalSubList); // 获取已经存在BOMLine中的替代料的Bean
		}


		ITK_THROW_ERROR(AOM_ask_value_tag(bomLine, bomAttr_lineObjectTag, &childItemRev));
		//		getMatGroupItemRev(bomBean, searchList, matGroupList, syncFrom, groupName, level, projectID);
		if (syncFlag && !matInfoList.empty()) { // 判断是否需要同步2nd Source			
			getMatGroupItemRevByMatch(matInfoList, searchList, matGroupList, bomBean.parentItem, bomBean.item, syncFrom, groupName, level, projectID); // 当需要执行同步2nd Source,并且通过查询DB获取的替代料群组信息不为空,才执行此方法获取替代料群组
		}

		if (!searchList.empty()) {
			bool allCheck = true;
			if (!totalSubList.empty()) { // 代表BOM中已經存在替代料
				allCheck = false;
			}
			get2ndSourceGroup(searchList, sourceGroupList, allCheck);
			removeSameBean(sourceGroupList, bomBean.itemRevUid); // 移除和物料对象相同的替代群组中的料对象
			totalSubList.insert(totalSubList.end(), sourceGroupList.begin(), sourceGroupList.end()); // totalSubList追加sourceGroupList
		}

		if (!totalSubList.empty()) {
			bomBean.alternativeGroup = getAltGroupByFindNum(bomBean);
			bomBean.alternativeCode = PRI;
			bomBean.usageProb = "100";
			distinctBeanByUid(totalSubList); // 通过对象uid去除重复项
			if (!matGroupList.empty()) {
				bomBean.materialGroupItemId = matGroupList[0];
				for (auto& subBean : totalSubList) {
					subBean.materialGroupItemId = matGroupList[0]; // 设置同步后的替代料群组ID
					subBean.alternativeGroup = bomBean.alternativeGroup;
					subBean.enable = enable;
					subBean.parentItem = bomBean.parentItem;
				}
			}
		}
	CLEANUP:
		return ifail;
	}

	int MNTEBOM2ndSource::getMatGroupItemRevByMatch(vector<MatGroupInfo>& matInfoList, vector<tag_t>& list, vector<string>& matGroupList, string& parentItem,
		string& item, string& syncFrom, string& groupName, string& level, string& projectID) {
		int
			ifail = ITK_ok;

		filterMatGroupList(matInfoList, syncFrom); // 过滤掉不符合要求的记录
		if (syncFrom.compare(GLOBAL2NDSOURCE) == 0) {
			getGlobal2ndSource(matInfoList, list, matGroupList, item, groupName); // 获取全局同步2nd Source
		} else if (syncFrom.compare(PROJECT2NDSOURCE) == 0) {
			getProject2ndSource(matInfoList, list, matGroupList, parentItem, item, level, projectID); // 获取专案同步2nd Source
		}

	CLEANUP:
		return ifail;
	}

	/*
	* 过滤掉不符合要求的记录
	*/
	static void filterMatGroupList(vector<MatGroupInfo>& matInfoList, string syncFrom) {
		clock_t start, finish;
		double totalTime;
		start = clock();
		for (auto it = matInfoList.begin(); it != matInfoList.end();) {
			MatGroupInfo target = *it;
			string itemId = target.matGroupItemId;
			tag_t itemRev = target.matGroupItemRev;
			string bu = getPropValue(itemRev, d9_BU);
			vector<string> splits = split(itemId, "-");
			if (regex_matchstr(itemId, pEndPattern) == 1) {
				it = matInfoList.erase(it);
			} else if (syncFrom.compare(GLOBAL2NDSOURCE) == 0) { // 如果是全局同步,则将物料群组对象版本d9_BU属性为空的记录移除
				if (splits.size() > 1) { // 移除物料群组ID含有"-"的记录
					it = matInfoList.erase(it);
				} else {
					if (!isContainsStr(toLowCase(bu), enum_to_string(mnt)) && !isContainsStr(toLowCase(bu), enum_to_string(prt))) { // 移除d9_BU属性未填写MNT、PRT二者其中之一记录 
						it = matInfoList.erase(it);
					} else {
						it++;
					}
				}
			} else if (syncFrom.compare(PROJECT2NDSOURCE)) { // 如果选择的是专案同步
				if (splits.size() == 1) { // 移除物料群组ID不含有"-"的记录
					it = matInfoList.erase(it);
				} else {
					if (isContainsStr(toLowCase(bu), enum_to_string(mnt)) || isContainsStr(toLowCase(bu), enum_to_string(prt))) { // 移除d9_BU属性填写MNT或PRT的记录
						it = matInfoList.erase(it);
					} else {
						it++;
					}
				}
			} else {
				it++;
			}

			if (it == matInfoList.end()) {
				break;
			}
		}

		finish = clock();
		totalTime = (double)(finish - start) / CLOCKS_PER_SEC;
		writeLog("filterMatGroupList cost time \n");
		writeLog("totalTime %.5lf\n", totalTime);
	}


	/*
	* 获取全局同步2nd Source
	*/
	static int getGlobal2ndSource(vector<MatGroupInfo>& matInfoList, vector<tag_t>& list, vector<string>& matGroupList, string& item, string& groupName) {
		clock_t start, finish;
		double totalTime;
		start = clock();
		int
			ifail = ITK_ok;

		for (auto& info : matInfoList) {
			if (item.compare(info.childItemId) == 0) {
				string matItemId = info.matGroupItemId;
				tag_t itemRev = info.matGroupItemRev;
				string bu = getPropValue(itemRev, d9_BU);
				if (isContainsStr(toLowCase(groupName), enum_to_string(monitor))) {
					if (isContainsStr(toLowCase(bu), enum_to_string(mnt))) {
						list.push_back(itemRev);
						matGroupList.push_back(matItemId);
					}
				} else if (isContainsStr(toLowCase(groupName), enum_to_string(printer))) {
					if (isContainsStr(toLowCase(bu), enum_to_string(prt))) {
						list.push_back(itemRev);
						matGroupList.push_back(matItemId);
					}
				}
			}
		}

		finish = clock();
		totalTime = (double)(finish - start) / CLOCKS_PER_SEC;
		writeLog("getGlobal2ndSource cost time \n");
		writeLog("totalTime %.5lf\n", totalTime);
		return ifail;
	}


	/*
	* 获取专案同步2nd Source
	*/
	static int getProject2ndSource(vector<MatGroupInfo>& matInfoList, vector<tag_t>& list, vector<string>& matGroupList, string& parentItem, string& item,
		string& level, string& projectID) {
		clock_t start, finish;
		double totalTime;
		start = clock();
		int
			ifail = ITK_ok;

		for (auto& info : matInfoList) {
			if (item.compare(info.childItemId) == 0) {
				string matItemId = info.matGroupItemId;
				vector<string> splits = split(matItemId, "-");
				tag_t itemRev = info.matGroupItemRev;
				if (isContainsStr(toUpperCase(matItemId), "-" + toUpperCase(level) + "-" + toUpperCase(projectID) + "-")) { // 判断是否包含 "-" + "L6" + "-" + "專案ID" + "-"
					list.push_back(itemRev);
					matGroupList.push_back(splits[0]);
				} else if (endWithStr(toUpperCase(matItemId), "-" + toUpperCase(projectID))) { // 判断是否"-" + "專案ID" 结尾
					list.push_back(itemRev);
					matGroupList.push_back(splits[0]);
				}
			}
		}

		distinctValue(matGroupList); // 移除重复项

		finish = clock();
		totalTime = (double)(finish - start) / CLOCKS_PER_SEC;
		writeLog("getProject2ndSource cost time \n");
		writeLog("totalTime %.5lf\n", totalTime);
		return ifail;
	}

	int MNTEBOM2ndSource::get2ndSourceGroup(vector<tag_t>& searchList, vector<SecondSourceInfo>& list, bool allCheck) {
		clock_t start, finish;
		double totalTime;
		start = clock();
		int
			ifail = ITK_ok,
			child_count = 0;
		tag_t
			* children = NULLTAG;
		tag_t
			topItem = NULLTAG,
			topItemRev = NULLTAG,
			rev_rule = NULLTAG,
			window = NULLTAG,
			topLine = NULLTAG,
			targetObj = NULLTAG,
			childBOMLine = NULLTAG;

		for (int i = 0; i < searchList.size(); i++) {
			try {
				topItemRev = searchList[i];
				ITK_THROW_ERROR(ITEM_ask_item_of_rev(topItemRev, &topItem));
				ITK_THROW_ERROR(CFM_find(Latest_Working, &rev_rule));
				ITK_THROW_ERROR(BOM_create_window(&window));
				ITK_THROW_ERROR(BOM_set_window_config_rule(window, rev_rule));
				ITK_THROW_ERROR(BOM_set_window_pack_all(window, false));
				ITK_THROW_ERROR(BOM_set_window_top_line(window, topItem, topItemRev, NULL, &topLine));
				ITK_THROW_ERROR(BOM_line_ask_child_lines(topLine, &child_count, &children));
				for (int j = 0; j < child_count; j++) {
					childBOMLine = children[j];
					string indentTitle = getPropValue(childBOMLine, indented_title);
					writeLog("==>> 替代料群组标题: %s\n", indentTitle.c_str());

					SecondSourceInfo groupBean;
					getTcProp(groupBean, childBOMLine);

					groupBean.alternativeCode = ALT;
					groupBean.isSub = true;
					if (allCheck) { // 如果为true, 则代表全部设置为全选
						groupBean.checkStates = true; // 替代料默认全选
						groupBean.syncCheckFlag = true; // 代表需要将背景颜色设置为红色
					}

					groupBean.subExistBom = false; // 代表还没有将替代料添加到BOMLine中
					groupBean.location = "";
					list.push_back(groupBean);
				}
			} catch (int& error_code) {
				ifail = error_code;
				goto CLEANUP;
			}			
		}

	CLEANUP:
		DOFREE(children);
		if (window != NULLTAG) {
			ITK_THROW_ERROR(BOM_close_window(window));
		}
		if (ifail != ITK_ok) {
			throw ifail;
		}
		finish = clock();
		totalTime = (double)(finish - start) / CLOCKS_PER_SEC;
		writeLog("get2ndSourceGroup cost time \n");
		writeLog("totalTime %.5lf\n", totalTime);
		return ifail;
	}

	int MNTEBOM2ndSource::getExistSubBeanList(tag_t& bomLine, vector<SecondSourceInfo>& totalSubList) {
		clock_t start, finish;
		double totalTime;
		start = clock();
		int
			ifail = ITK_ok,
			sub_child_count = 0;

		tag_t
			* sub_children = NULLTAG;

		tag_t
			subBomLine = NULLTAG;

		ITK_THROW_ERROR(BOM_line_list_substitutes(bomLine, &sub_child_count, &sub_children)); // 获取替代料BOM集合
		for (int i = 0; i < sub_child_count; i++) {
			subBomLine = sub_children[i];
			SecondSourceInfo subBean;
			getTcProp(subBean, subBomLine); // 获取替代料属性
			subBean.alternativeCode = ALT;
			subBean.isSub = true;
			subBean.checkStates = true;
			subBean.location = "";
			totalSubList.push_back(subBean);
		}


	CLEANUP:
		DOFREE(sub_children);
		finish = clock();
		totalTime = (double)(finish - start) / CLOCKS_PER_SEC;
		writeLog("getExistSubBeanList cost time\n");
		writeLog("totalTime %.5lf\n", totalTime);
		return ifail;
	}

	void MNTEBOM2ndSource::setAltGroup(SecondSourceInfo& rootBean) {
		vector<SecondSourceInfo> childs = rootBean.childs;
		if (childs.empty()) {
			return;
		}
		for (auto& bomBean : childs) {
			if (bomBean.alternativeGroup.empty()) {
				vector<SecondSourceInfo> subList = bomBean.substitutesList;
				if (!subList.empty()) {
					string altGroup = getAltGroupByRule(stoi(bomBean.findNum));
					bomBean.alternativeGroup = altGroup;
					for (auto& subBean : subList) {
						subBean.alternativeGroup = altGroup;
					}
				}
			}
			vector<SecondSourceInfo> childs2 = bomBean.childs;
			if (!childs2.empty()) {
				setAltGroup(bomBean);
			}
		}
	}

	tag_t MNTEBOM2ndSource::checkTopLineRevise(tag_t& window, SecondSourceInfo& rootBean, map<string, tag_t>& bomMap) {
		tag_t
			rootItem = NULLTAG,
			rootItemRev = NULLTAG,
			rootBomLine = NULLTAG,
			newRootItem = NULLTAG,
			newRootItemRev = NULLTAG,
			newRootBomLine = NULLTAG,
			rev_rule = NULLTAG;
		char
			* objectType = NULL;
		string rootBomLineUid = rootBean.bomLineUid;
		ITK__convert_uid_to_tag(rootBomLineUid.c_str(), &rootBomLine); // 获取根BOMLine对象
		ITK_THROW_ERROR(AOM_ask_value_tag(rootBomLine, bomAttr_lineObjectTag, &rootItemRev)); // 通过BOMLine获取对象版本;
		ITK_THROW_ERROR(ITEM_ask_item_of_rev(rootItemRev, &rootItem)); // 通过版本获取对象
		string itemId = getPropValue(rootItemRev, item_id);
		string version = getPropValue(rootItemRev, current_revision_id);
		if (checkTopLineStatus(rootBean, rootItemRev)) { // 判断顶层BOMLine的子阶是否发布，假如发布，判断子阶是否发生变化
			string pattern = getVersionRule(rootItemRev); // 获取版本规则模式
			string newRevId = getVersionByRule(rootItem, pattern); // 获取版本号
			if (newRevId.empty()) {
				writeLog("itemId == %s, version == %s, 获取升版的版本号失败\n", itemId, version);
				return NULLTAG;
			}

			reviseRev(rootItemRev, newRevId, &newRootItemRev); // 进行升版
			
			ITK_THROW_ERROR(CFM_find(Latest_Working, &rev_rule));
			ITK_THROW_ERROR(BOM_set_window_config_rule(window, rev_rule));
			ITK_THROW_ERROR(BOM_set_window_pack_all(window, false)); // 解包
			ITK_THROW_ERROR(BOM_set_window_top_line(window, NULLTAG, newRootItemRev, NULL, &newRootBomLine));
			getBOMMap(newRootBomLine, bomMap);
		}

	CLEANUP:
		DOFREE(objectType);
		return newRootBomLine;
	}


	/*
	* 判断当前父阶是否已经发行
	*/
	int MNTEBOM2ndSource::checkParentBOMLineStatus(tag_t& window, tag_t& bomLine, map<string, tag_t>& bomMap) {
		int ifail = ITK_ok;
		tag_t
			superParentBomLine = NULLTAG,
			parentBomLine = NULLTAG,
			parentItem = NULLTAG,
			parentItemRev = NULLTAG,
			newParentItemRev = NULLTAG,
			newParentBomLine = NULLTAG,
			rev_rule = NULLTAG;
		char
			* objectType = NULL;
		ITK_THROW_ERROR(AOM_ask_value_tag(bomLine, bomAttr_lineParentTag, &parentBomLine)); // 获取父BOMLine;
		ITK_THROW_ERROR(AOM_ask_value_tag(parentBomLine, bomAttr_lineObjectTag, &parentItemRev)); // 通过BOMLine获取对象版本;
		ITK_THROW_ERROR(ITEM_ask_item_of_rev(parentItemRev, &parentItem)); // 通过对象版本获取对象
		string itemId = getPropValue(parentItemRev, item_id);
		string version = getPropValue(parentItemRev, current_revision_id);
		if (isRelease(parentItemRev)) {
			string pattern = getVersionRule(parentItemRev); // 获取版本规则模式
			string newRevId = getVersionByRule(parentItem, pattern); // 获取版本号
			if (newRevId.empty()) {
				writeLog("itemId == %s, version == %s, 获取升版的版本号失败\n", itemId, version);
				goto CLEANUP;
			}

			reviseRev(parentItemRev, newRevId, &newParentItemRev); // 进行升版
			ITK_THROW_ERROR(CFM_find(Latest_Working, &rev_rule));
			ITK_THROW_ERROR(BOM_set_window_config_rule(window, rev_rule)); // 设置加载最新版
			ITK_THROW_ERROR(BOM_set_window_pack_all(window, false)); // 解包

			ITK_THROW_ERROR(AOM_ask_value_tag(parentBomLine, bomAttr_lineParentTag, &superParentBomLine)); // 获取父BOMLine的父;
			if (superParentBomLine != NULL) {
				newParentBomLine = findBOMLine(superParentBomLine, newParentItemRev); // 返回在pLine子集合的newItemRev对应的BOMLine对象
			}
			getBOMMap(newParentBomLine, bomMap);
		}

	CLEANUP:
		DOFREE(objectType);
		return ifail;
	}

	bool MNTEBOM2ndSource::checkTopLineStatus(SecondSourceInfo& rootBean, tag_t& rootItemRev) {
		tag_t
			bomLine = NULLTAG;	

		if (!isRelease(rootItemRev)) {
			return false; // 代表此顶层BOMLine不是属于发布状态，无需进一步判断
		}
		vector<SecondSourceInfo> childs = rootBean.childs;
		if (!childs.empty()) {
			for (auto& mainBomInfo : childs) {
				vector<SecondSourceInfo> subList = mainBomInfo.substitutesList;
				if (subList.empty()) {
					continue;
				}

				string mainBomLineUid = mainBomInfo.bomLineUid;
				ITK__convert_uid_to_tag(mainBomLineUid.c_str(), &bomLine); // 获取BOMLine对象

				if (checkChange(bomLine, subList)) { // 判断替代料是否发生改变
					return true;
				}

				if (!subList.empty()) { // 此时的替代料集合全部是不存在TC中的记录
					for (auto& subBomInfo : subList) {
						bool checkStates = subBomInfo.checkStates;
						if (checkStates) {
							return true;
						}
					}
				}
			}
		}
		return false;
	}

	bool MNTEBOM2ndSource::checkChange(tag_t& bomLine, vector<SecondSourceInfo>& subList) {
		bool result = false;
		int
			sub_child_count = 0;
		tag_t
			* sub_children = NULLTAG;
		tag_t
			sourceItemRev = NULLTAG;
		char
			* sourceItemUid = NULL;

		for (auto it = subList.begin(); it != subList.end();) {
			SecondSourceInfo target = *it;
			string itemRevUid = target.itemRevUid;
			bool checkStates = target.checkStates;
			string altCode = target.alternativeCode;

			if (itemRevUid.empty()) { // 将从PNMS系统查询的记录排除
				continue;
			}
			ITK_THROW_ERROR(BOM_line_list_substitutes(bomLine, &sub_child_count, &sub_children)); // 获取替代料集合	
			vector<tag_t> subTagList = getTagList(sub_children, sub_child_count); // // 将对象指针转换为集合数组
			auto flag = find_if(subTagList.begin(), subTagList.end(), [&](const tag_t& subBomLine) {
				ITK_THROW_ERROR(AOM_ask_value_tag(subBomLine, bomAttr_lineObjectTag, &sourceItemRev)); // 通过BOMLine获取对象版本;
				ITK__convert_tag_to_uid(sourceItemRev, &sourceItemUid); // 通过对象版本获取uid
				return tc_strcmp(sourceItemUid, itemRevUid.c_str()) == 0;
			});

			bool found = flag != subTagList.end();
			if (found) {
				if (!checkStates) { // 存在于TC中,但是没有被选中
					result = true;
					break;
				} else {
					if (altCode.compare(PRI) == 0) { // 存在于TC中，但是将此替代料设置为了主料
						result = true;
						break;
					}
				}
				it = subList.erase(it);
				if (it == subList.end()) {
					break;
				}
			} else {
				it++; // 执行下一次迭代
			}
		}
	CLEANUP:
		DOFREE(sub_children);
		DOFREE(sourceItemUid);
		return result;
	}

	int MNTEBOM2ndSource::getBOMMap(tag_t& topLine, map<string, tag_t>& bomMap) {
		int
			ifail = ITK_ok,
			child_count = 0;

		tag_t
			* children = NULLTAG;

		tag_t
			childBomLine = NULLTAG;

		string
			parentItemId,
			childItemId;
		ITK_THROW_ERROR(BOM_line_ask_child_lines(topLine, &child_count, &children));
		if (child_count > 0) {
			parentItemId = getPropValue(topLine, bomAttr_itemId);
			bomMap.insert(pair<string, tag_t>("$" + parentItemId, topLine));
			for (int i = 0; i < child_count; i++) {
				childBomLine = children[i];
				childItemId = getPropValue(childBomLine, bomAttr_itemId);
				bomMap.insert(pair<string, tag_t>(parentItemId + "$" + childItemId, childBomLine));
				getBOMMap(childBomLine, bomMap);
			}
		}

	CLEANUP:
		DOFREE(children);
		return ifail;
	}

	
	tag_t MNTEBOM2ndSource::findBOMLine(tag_t& pLine, tag_t& newItemRev) {
		int			
			child_count = 0;	

		char
			* sourceItemUid = NULL,
			* matchItemUid = NULL;
		tag_t
			* children = NULLTAG;

		tag_t
			itemRev = NULLTAG,
			resultBomLine = NULLTAG,
			childBomLine = NULLTAG;		

		ITK_THROW_ERROR(BOM_line_ask_child_lines(pLine, &child_count, &children));
		for (int i = 0; i < child_count; i++) {
			childBomLine = children[i];
			ITK_THROW_ERROR(AOM_ask_value_tag(childBomLine, bomAttr_lineObjectTag, &itemRev));
			ITK__convert_tag_to_uid(itemRev, &sourceItemUid);
			ITK__convert_tag_to_uid(newItemRev, &matchItemUid);
			if (tc_strcmp(sourceItemUid, matchItemUid) == 0) {
				resultBomLine  = childBomLine;
				break;
			}
		}

	CLEANUP:
		DOFREE(children);
		DOFREE(sourceItemUid);
		DOFREE(matchItemUid);
		return resultBomLine;
	}


	/*
	* 判断是否需要合并
	*/
	void MNTEBOM2ndSource::checkMerge(SecondSourceInfo& rootBean) {
		vector<SecondSourceInfo> childs = rootBean.childs;
		if (childs.empty()) {
			return;
		}

		for (auto& childBean : childs) {
			if (childBean.isDelete) { // 如果当前主料是设置为需要删除，则无需进行判断是否需要合并
				continue;
			}

			if (!childBean.substitutesList.empty()) {
				checkExist(childBean, childs);
			}

			checkMerge(childBean);
		}
	}

	/*
	* 判断替代群组是否相同
	*/
	static void checkExist(SecondSourceInfo& baseBean, vector<SecondSourceInfo>& list) {
		vector<string> sourceList = getItemIdList(baseBean);
		sortVectorByName(sourceList); // 通过名称进行升序排序
		for (auto& bomBean : list) {
			vector<SecondSourceInfo> subList = bomBean.substitutesList;
			auto flag = find_if(subList.begin(), subList.end(), [&](SecondSourceInfo& subBean) {
				return baseBean.item.compare(subBean.item) == 0; // 判断item是否一致
				});

			bool found = flag != subList.end();
			if (found) {
				vector<string> destList = getItemIdList(bomBean);
				sortVectorByName(destList); // 通过名称进行升序排序
				if (sourceList == destList) {
					string baseQty = baseBean.qty;
					string destQty = bomBean.qty;
					string totalQty = add(baseQty, destQty);
					if (isContainsStr(baseBean.location, bomBean.location)) { // 判断是否包含此位置
						continue;
					}
					baseBean.location = baseBean.location + "," + bomBean.location;
					baseBean.qty = totalQty;
					baseBean.hasMerge = true; // 设置位置和数量发生合并的标识
					vector<SecondSourceInfo> subList = baseBean.substitutesList;
					for (auto& subBean : subList) {
						subBean.hasMerge = true; // 设置位置和数量发生合并的标识
					}
					bomBean.isDelete = true;
				}
			}
		}
	}

	/*
	* 获取Item集合
	*/
	static vector<string> getItemIdList(SecondSourceInfo& bomBean) {
		vector<string> list;
		list.push_back(bomBean.item);
		vector<SecondSourceInfo> subList = bomBean.substitutesList;
		for (auto& subBean : subList) {
			list.push_back(subBean.item);
		}
		return list;
	}

	int MNTEBOM2ndSource::getItemListByContainSub(tag_t& topLine, vector<string>& list) {
		int
			ifail = ITK_ok,
			child_count = 0,
			sub_child_count = 0;

		tag_t
			childBomLine = NULLTAG,
			subBomLine = NULLTAG;

		tag_t
			* children = NULLTAG,
			* sub_children = NULLTAG;

		ITK_THROW_ERROR(BOM_line_ask_child_lines(topLine, &child_count, &children));
		for (int i = 0; i < child_count; i++) {
			logical
				isSub = false,
				hasSub = false;
			childBomLine = children[i];
			ITK_THROW_ERROR(BOM_line_ask_is_substitute(childBomLine, &isSub)); // 判断是否为替代料
			if (isSub) {
				continue;
			}

			ITK_THROW_ERROR(BOM_line_ask_has_substitutes(childBomLine, &hasSub)); // 判断是否含有替代料
			if (hasSub) {
				//				string itemId = getPropValue(childBomLine, bomAttr_itemId);
				//				list.push_back(itemId);

				ITK_THROW_ERROR(BOM_line_list_substitutes(childBomLine, &sub_child_count, &sub_children)); {
					for (int j = 0; j < sub_child_count; j++) {
						subBomLine = sub_children[j];
						string subItemId = getPropValue(subBomLine, bomAttr_itemId);
						list.push_back(subItemId);
					}
				}
			}

			getItemListByContainSub(childBomLine, list);
		}

	CLEANUP:
		DOFREE(children);
		return ifail;
	}

	int MNTEBOM2ndSource::getLeafNoSubItemList(tag_t& topLine, vector<string>& list) {
		clock_t start, finish;
		double totalTime;
		start = clock();
		int
			ifail = ITK_ok,
			child_count = 0,
			sub_child_count = 0;

		tag_t
			childBomLine = NULLTAG;

		tag_t
			* children = NULLTAG;

		ITK_THROW_ERROR(BOM_line_ask_child_lines(topLine, &child_count, &children));
		if (child_count == 0) { // 代表当前是根节点
			string itemId = getPropValue(topLine, bomAttr_itemId);
			list.push_back(itemId);
		} else {
			for (int i = 0; i < child_count; i++) {
				logical
					isSub = false;
				childBomLine = children[i];
				ITK_THROW_ERROR(BOM_line_ask_is_substitute(childBomLine, &isSub)); // 判断是否为替代料
				if (isSub) {
					continue;
				}

				getLeafNoSubItemList(childBomLine, list);
			}
		}

	CLEANUP:
		DOFREE(children);
		finish = clock();
		totalTime = (double)(finish - start) / CLOCKS_PER_SEC;
		writeLog("getLeafItemList cost time \n");
		writeLog("totalTime %.5lf\n", totalTime);
		return ifail;
	}

	int MNTEBOM2ndSource::getMatGroupInfoList(vector<string>& list, vector<MatGroupInfo>& matInfoList) {
		clock_t start, finish;
		double totalTime;
		start = clock();
		int
			ifail = ITK_ok;
		Environment* env = NULL;
		Connection* conn = NULL;
		Statement* pStmt = NULL;
		ResultSet* pRs = NULL;
		tag_t matGroupItemRev = NULLTAG;
		string findSql = "";
		getConnection(D9_DB_XPLM, env, conn); // 连接数据库
		vector<vector<string>> groupList = groupVector(list, 950); // 以950个长度分割
		for (auto& vec : groupList) {
			string inPut = getChildIds(vec);
			findSql = "SELECT * FROM GET_MATERIAL_GROUP_BY_CHILDID where child_id IN (" + inPut + ")";
			writeLog("findSql == %s\n", findSql.c_str());
			pStmt = conn->createStatement();
			pRs = pStmt->executeQuery(findSql);

			while (pRs->next()) {
				string uid = pRs->getString(1);
				if (uid.empty()) {
					continue;
				}
				string parentId = pRs->getString(2);
				string prenteRev = pRs->getString(3);
				string childId = pRs->getString(4);
				MatGroupInfo matInfo;
				ITK__convert_uid_to_tag(uid.c_str(), &matGroupItemRev);
				matInfo.matGroupItemRev = matGroupItemRev;
				matInfo.matGroupItemId = parentId;
				matInfo.matGroupVersion = prenteRev;
				matInfo.childItemId = childId;
				matInfoList.push_back(matInfo);
			}

			findSql = "";
			pRs = NULL;
			pStmt = NULL;
		}

	CLEANUP:
		if (pRs != NULL) {
			pStmt->closeResultSet(pRs);
		}
		if (pStmt != NULL) {
			conn->terminateStatement(pStmt);
		}
		if (conn != NULL) {
			env->terminateConnection(conn);
		}
		if (env != NULL) {
			Environment::terminateEnvironment(env);
		}
		finish = clock();
		totalTime = (double)(finish - start) / CLOCKS_PER_SEC;
		writeLog("getMatGroupInfoList cost time \n");
		writeLog("totalTime %.5lf\n", totalTime);
		return ifail;
	}

	/*
	*将包含ChildId的集合转成以“,”分割的字符串
	*/
	static string getChildIds(vector<string>& list) {
		string strData = "";
		for (auto& str : list) {
			strData += "'" + str + "',";
		}
		strData = strData.substr(0, strData.size() - 1);
		return strData;
	}


	/**
	 * 重新设置替代料属性
	 * @param substitutesList
	 * @param bomInfo
	 */
	void resetSubsProp(vector<SecondSourceInfo> substitutesList, SecondSourceInfo bomInfo) {
		for (auto& subInfo : substitutesList) {
			subInfo.location = bomInfo.location;
			subInfo.qty = bomInfo.qty;
		}
	}


#ifdef __cplusplus
}
#endif
