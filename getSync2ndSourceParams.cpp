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
	* ��ȡ�����Ⱥ��ָ�ɵ�ר����Ϣ
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
		vector<tag_t> tagList = getTagList(queryResults, resultLength); // ������ָ��ת��Ϊ��������
		if (tagList.empty()) {
			goto CLEANUP;
		}

		filterList(tagList); // ���˼���Ԫ��ID��-�ָ�󳤶Ȳ�Ϊ4�ļ�¼		
		distinctTagByProjectId(tagList); // ͨ��ӵ����ͬ��ר��ID����ȥ��
		for (auto itemRev : tagList) {
			string itemId = getPropValue(itemRev, item_id);
			vector<string> splits = split(itemId, "-");
			string findStr = getElementStartWith(splits, "P"); // ������P��ͷ���ַ���
			if (findStr.empty()) {
				continue;
			}

			vector<string> splits2 = split(findStr, ",");
			for (auto s : splits2) {
//				queryProjectById(s, project); // ͨ��ר��ID��ȡר������
				ITK_THROW_ERROR(PROJ_find(s.c_str(), &project)); // ͨ��ר��ID��ȡר������
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
				if (projectIdStr.compare(curProjectID) == 0) { // ����͵�ǰ��ר��һ��,������һ�ε���				
					continue;
				}

				json projJson;
				projJson["projectID"] = projectIdStr;
				projJson["projectName"] = projectNameStr;
				if (!checkJsonItemExist(data, projJson, "projectID")) { // �ж�Json Item�Ƿ��Ѿ�����
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
	* ���˼���Ԫ��ID��-�ָ�󳤶Ȳ�Ϊ4�ļ�¼
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
	* ͨ��ItemID�������ר��ID����������
	*/
	bool compareContainProjectId(const tag_t& firstTag, const tag_t& secondTag) {
		string firstItemId = getPropValue(firstTag, item_id);
		string secondItemId = getPropValue(secondTag, item_id);
		vector<string> firstSplits = split(firstItemId, "-");
		vector<string> secondTagSplits = split(secondItemId, "-");
		return firstSplits[2] < secondTagSplits[2];
	}

	/*
	* ͨ��ӵ����ͬ��ר��ID����ȥ��
	*/
	void distinctTagByProjectId(vector<tag_t>& tagList) {
		std::sort(tagList.begin(), tagList.end(), compareContainProjectId); // ���Ƚ�������
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

			getMatGroupProjsByLevel(data); // ��ȡ�����Ⱥ��ָ�ɵ�ר����Ϣ

			result["code"] = "0000";
			result["msg"] = getExternCode("��ѯ�ɹ�");
			result["data"] = data;
		} catch (int& error_code) {
			ifail = error_code;
			throw exception(getErrorMsg(ifail).c_str());
		} catch (const std::exception& e) {
			writeLog("error == %s\n", e.what());
			result["code"] = "4002";
			result["msg"] = getExternCode(("��ѯʧ��,������Ϣ����: " + (string)e.what()).c_str());
		}
		
	CLEANUP:
		appendJsonData(jsonFileName.c_str(), result.dump().c_str()); // ����־׷�ӵ�json��
		return result.dump();
	}

#ifdef __cplusplus
}
#endif
