#include "secondSourceCommonUtils.h"
#include "getSync2ndSourceParams.h"
using namespace MNTEBOM2ndSource;
#ifdef __cplusplus
extern "C" {
#endif

	static string
		curProjectID,
		syncFrom,
		level;

	int getMatGroupProjsByLevel(json& data);
	static void filterList(vector<tag_t>& tagList);
	bool compareContainProjectId(const tag_t& firstTag, const tag_t& secondTag);
	void distinctTagByProjectId(vector<tag_t>& tagList);

	/*
	* 获取替代料群组指派的专案信息
	*/
	int getMatGroupProjsByLevel(json& data) {
		int
			ifail = ITK_ok,
			resultLength = 0;
		char
			* projectID = NULL,
			* projectName = NULL;
		tag_t
			itemRev = NULLTAG,
			project = NULLTAG;
		tag_t
			* queryResults = NULLTAG;
		string
			projectIdStr,
			projectNameStr;

		char** qry_entries = (char**)MEM_alloc(sizeof(char**));
		char** qry_values = (char**)MEM_alloc(sizeof(char**));
		qry_entries[0] = (char*)MEM_alloc(sizeof(char*) * 200);
		qry_values[0] = (char*)MEM_alloc(sizeof(char*) * 200);

		string queryValue = "*" + level + "-P" + "*";
		//string queryValue = "*";
		tc_strcpy(qry_entries[0], ID);
		tc_strcpy(qry_values[0], queryValue.c_str());

		ITK_THROW_ERROR(query(D9_Find_MaterialGroupByID, qry_entries, qry_values, 1, &queryResults, &resultLength));
		writeLog(" -- >> Assign project D9_MaterialGroupRevision find : %s queryresult : %d \n", queryValue, resultLength);
		vector<tag_t> tagList = getTagList(queryResults, resultLength); // 将对象指针转换为集合数组
		if (tagList.empty()) {
			goto CLEANUP;
		}

		filterList(tagList); // 过滤集合元素ID以-分割后长度不为4的记录		
		distinctTagByProjectId(tagList); // 通过拥有相同的专案ID进行去重
		for (auto itemRev : tagList) {
			string itemId = getPropValue(itemRev, item_id);
			vector<string> splits = split(itemId, "-");
			string findStr = getElementStartWith(splits, "P"); // 返回以P开头的字符串
			if (findStr.empty()) {
				continue;
			}

			vector<string> splits2 = split(findStr, ",");
			for (auto s : splits2) {
//				queryProjectById(s, project); // 通过专案ID获取专案对象
				ITK_THROW_ERROR(PROJ_find(s.c_str(), &project)); // 通过专案ID获取专案对象
				if (project == NULLTAG) {
					continue;
				}
				ITK_THROW_ERROR(PROJ_ask_id2(project, &projectID));
				writeLog("projectID == %s\n", projectID);
				ITK_THROW_ERROR(PROJ_ask_name2(project, &projectName));
				writeLog("projectName == %s\n", projectName);
				projectIdStr = (string)projectID;
				projectNameStr = (string)projectName;
				string_replace(projectNameStr, "{", "\\{");
				string_replace(projectNameStr, "\}", "\\}");
				if (projectIdStr.compare(curProjectID) == 0) { // 如果和当前的专案一致,进行下一次迭代				
					continue;
				}

				json projJson;
				projJson["projectID"] = projectIdStr;
				projJson["projectName"] = projectNameStr;
				if (!checkJsonItemExist(data, projJson, "projectID")) { // 判断Json Item是否已经存在
					data.push_back(projJson);
				}
			}
		}
	CLEANUP:
		DOFREE(projectID);
		DOFREE(projectName);
		DOFREE(qry_entries);
		DOFREE(qry_values);
		DOFREE(queryResults);
		return ifail;
	}


	/*
	* 过滤集合元素ID以-分割后长度不为4的记录
	*/
	static void filterList(vector<tag_t>& tagList) {
		for (auto it = tagList.begin(); it != tagList.end();) {
			tag_t target = *it;
			string itemId = getPropValue(target, item_id);
			vector<string> splits = split(itemId, "-");
			if (splits.size() != 4) {
				it = tagList.erase(it);
				if (it == tagList.end()) {
					break;
				}
			} else {
				it++;
			}
		}
	}


	/*
	* 通过ItemID里面包含专案ID来进行排序
	*/
	bool compareContainProjectId(const tag_t& firstTag, const tag_t& secondTag) {
		string firstItemId = getPropValue(firstTag, item_id);
		string secondItemId = getPropValue(secondTag, item_id);
		vector<string> firstSplits = split(firstItemId, "-");
		vector<string> secondTagSplits = split(secondItemId, "-");
		return firstSplits[2] < secondTagSplits[2];
	}

	/*
	* 通过拥有相同的专案ID进行去重
	*/
	void distinctTagByProjectId(vector<tag_t>& tagList) {
		std::sort(tagList.begin(), tagList.end(), compareContainProjectId); // 首先进行排序
		tagList.erase(std::unique(tagList.begin(), tagList.end(), [](const tag_t& firstTag, const tag_t& secondTag) {
			string firstItemId = getPropValue(firstTag, item_id);
			string secondItemId = getPropValue(secondTag, item_id);
			vector<string> firstSplits = split(firstItemId, "-");
			vector<string> secondTagSplits = split(secondItemId, "-");
			return firstSplits[2].compare(secondTagSplits[2]) == 0;
			}), tagList.end());
	}

	extern __declspec(dllexport) string get2ndSourceParams(string params) {
		int
			ifail = ITK_ok;
		tag_t
			sessionTag = NULLTAG,
			userTag = NULLTAG,
			project = NULLTAG;

		json
			paramsData,
			data = json::array(),
			result;

		string
			jsonFileName,
			jsonFileNamePath;

		createLogFile("2ndSourceService_get2ndSourceParams");

		try {
			paramsData = json::parse(string2wstring(params));
			curProjectID = paramsData["curProjectID"].get<std::string>();
			level = paramsData["level"].get<std::string>();

			writeLog("level == %s\n", level.c_str());
			writeLog("curProjectID == %s\n", curProjectID.c_str());

			jsonFileName = level + "2ndSourceService_get2ndSourceParams";
			jsonFileNamePath = createJsonFile(jsonFileName.c_str());

			getMatGroupProjsByLevel(data); // 获取替代料群组指派的专案信息

			result["code"] = "0000";
			result["msg"] = getExternCode("查询成功");
			result["data"] = data;
		} catch (int& error_code) {
			ifail = error_code;
			throw exception(getErrorMsg(ifail).c_str());
		} catch (const std::exception& e) {
			writeLog("error == %s\n", e.what());
			result["code"] = "4002";
			result["msg"] = getExternCode(("查询失败,错误信息如下: " + (string)e.what()).c_str());
		}
		
	CLEANUP:
		appendJsonData(jsonFileName.c_str(), result.dump().c_str()); // 将日志追加到json中
		return result.dump();
	}

#ifdef __cplusplus
}
#endif
