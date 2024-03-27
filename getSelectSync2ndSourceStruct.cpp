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
		leafMainItemList; // Ҷ�ӽڵ����ϼ���

	static vector<MatGroupInfo>
		matInfoList; // �����Ⱥ����Ϣ����

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

			getSelectLeafMainItemList(selectDataList, leafMainItemList); // ��ȡ��ǰѡ�еĸ��ڵ����Ϻż���
			distinctValue(leafMainItemList); // ȥ��
			getMatGroupInfoList(leafMainItemList, matInfoList); // ��ѯDB��ȡ�����Ⱥ����Ϣ
			for (auto info : selectDataList) {
				string parentUid = info.parentUid;
				string bomUid = info.bomUid;

				writeLog("parentUid == %s\n", parentUid.c_str());
				writeLog("bomUid == %s\n", bomUid.c_str());

				ITK__convert_uid_to_tag(parentUid.c_str(), &parentBomLine); // ��ȡ��BOMLine
				ITK__convert_uid_to_tag(bomUid.c_str(), &bomLine); // ��ȡ��BOMLine

				SecondSourceInfo bomBean;
				getSelectTotalSync2ndSourceInfo(bomBean, parentBomLine, bomLine, true, true);

				json beanJson = bomBean;
				if (!checkJsonItemExist(data, beanJson, "itemRevUid")) { // �ж�Json Item�Ƿ��Ѿ�����
					data.push_back(beanJson);
				}
			}

			result["code"] = "0000";
			result["msg"] = getExternCode("�ɹ�");
			result["data"] = data;
		} catch (int& error_code) {
			ifail = error_code;
			throw exception(getErrorMsg(ifail).c_str());
		} catch (const std::exception& e) {
			writeLog("error == %s\n", e.what());
			result["code"] = "4002";
			result["msg"] = getExternCode(("BOMѡ����ͬ��2nd Sourceʧ��,������Ϣ����: " + (string)e.what()).c_str());
		}

	CLEANUP:
		appendJsonData(jsonFileNamePath.c_str(), data.dump().c_str()); // ���ṹ��ת��Ϊjson���󱣴浽json��
		return result.dump();
	}	

	/*
	* ��ȡѡ����ͬ��2nd Source���е�����ϼ���
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

		ITK_THROW_ERROR(BOM_line_ask_is_substitute(bomLine, &isSub)); // �ж��Ƿ�Ϊ�����
		if (!isSub) {
			getTcProp(bomBean, bomLine); //��ȡ����
			ITK_THROW_ERROR(AOM_ask_value_tag(parentBomLine, bomAttr_lineObjectTag, &parentItemRev)); // ͨ��BOMLine��ȡ����汾			
			bomBean.enable = enable;
			bomBean.parentItem = getPropValue(parentItemRev, item_id);

			vector<SecondSourceInfo> subList;
			getTotal2ndSourceSubList(matInfoList, subList, bomLine, bomBean, syncFrom, groupName, level, projectID, syncFlag, enable); // ��ȡִ��ͬ��2nd Source�������ϼ���
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
	* ��ȡ��ǰѡ�еĸ��ڵ����Ϻż���
	*/
	int getSelectLeafMainItemList(vector<SelectSync2ndSourceInfo>& selectDataList, vector<string>& list) {
		int
			ifail = ITK_ok;

		tag_t
			bomLine = NULLTAG;
		for (auto& info : selectDataList) {
			string bomUid = info.bomUid;
			ITK__convert_uid_to_tag(bomUid.c_str(), &bomLine); // ��ȡ��BOMLine
			string itemId = getPropValue(bomLine, bomAttr_itemId);
			list.push_back(itemId);
		}

		return ifail;
	}


#ifdef __cplusplus
}
#endif
