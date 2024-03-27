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
		leafMainItemList; // Ҷ�ӽڵ����ϼ���

	static vector<MatGroupInfo>
		matInfoList; // �����Ⱥ����Ϣ����

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

			ITK__convert_uid_to_tag(uid.c_str(), &topLine); // ��ȡ����BOMLine
			jsonFileName = level + ("2ndSourceService_getSync2ndSourceStruct");
			jsonFileNamePath = createJsonFile(jsonFileName.c_str());

			ITK_THROW_ERROR(BOM_line_ask_window(topLine, &window)); // ͨ��BOMLine��ȡwindow
//			ITK_THROW_ERROR(AOM_ask_value_tag(topLine, bomAttr_lineWindowTag, &window)); // ͨ������BOMLine��ȡwindow����
			ITK_THROW_ERROR(BOM_set_window_pack_all(window, false)); // ���

			leafMainItemList.clear();
			matInfoList.clear();

			getLeafNoSubItemList(topLine, leafMainItemList); // ��ȡҶ�ӽڵ���Ϻż���
			distinctValue(leafMainItemList); // ȥ��
			getMatGroupInfoList(leafMainItemList, matInfoList); // ��ѯDB��ȡ�����Ⱥ����Ϣ
			recurseSync2ndSourceBOM(rootBean, topLine, true, true);
			rootBean.enable = false;

			data = rootBean;
			result["code"] = "0000";
			result["msg"] = getExternCode("�ɹ�");
			result["data"] = data;
		} catch (int& error_code) {
			ifail = error_code;
			throw exception(getErrorMsg(ifail).c_str());
		} catch (const std::exception& e) {
			writeLog("error == %s\n", e.what());
			result["code"] = "4002";
			result["msg"] = getExternCode(("BOMͬ��2nd Sourceʧ��,������Ϣ����: " + (string)e.what()).c_str());
		}

	CLEANUP:
		appendJsonData(jsonFileNamePath.c_str(), data.dump().c_str()); // ���ṹ��ת��Ϊjson���󱣴浽json��
		return result.dump();
	}

	/*
	* �ݹ����ͬ��2nd Source BOMLine
	* syncFlag �����Ƿ���Ҫͬ��2nd Source��ʶ, true������Ҫͬ��
	* enable �ж��Ƿ�����Ҽ���ѡ�и�ѡ��, true�������
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

				ITK_THROW_ERROR(BOM_line_ask_is_substitute(childBomLine, &isSub)); // �ж��Ƿ�Ϊ�����
				if (isSub) {
					continue;
				}
				if (!sourceBomMap.empty()) {
					if (isBom(childItemRev)) {
						auto it = sourceBomMap.find(childItemRevUid); // ���Դ����BOM���ϲ�Ϊ��, ���Ҵ����ϴ���Դ����BOM��
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
					getTotal2ndSourceSubList(matInfoList, subList, childBomLine, childBean, syncFrom, groupName, level, projectID, syncFlag, enable); // ��ȡִ��ͬ��2nd Source�������ϼ���
				}				
				
				if (!subList.empty()) {
					if (verCheckResult.empty()) {
						tag_t
							parentItemRev = NULLTAG;
						ITK_THROW_ERROR(AOM_ask_value_tag(topLine, bomAttr_lineObjectTag, &parentItemRev)); // ͨ��BOMLine��ȡ����汾
						checkVerCount(parentItemRev, verCheckResult);
						writeLog("%s\n", verCheckResult);
					}
					childBean.verNote = getExternCode(verCheckResult.c_str());
					childBean.alternativeGroup = getAltGroupByFindNum(childBean);
				}

				getSyncSubBOMStruct(childBomLine, subList, syncFlag, enable); // ��ȡͬ��2nd Source�����BOM���ӽ�

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
	* ��ȡͬ��2nd Source�����BOM���ӽ�
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

		ITK_THROW_ERROR(BOM_line_ask_has_substitutes(bomLine, &hasSub)); // �ж��Ƿ��������
		if (hasSub) {
			ITK_THROW_ERROR(BOM_line_ask_child_lines(bomLine, &sub_child_count, &sub_children)); // ��ȡ�����BOM����
			for (int i = 0; i < sub_child_count; i++) {
				subBomLine = sub_children[i];
				SecondSourceInfo subChildBean;
				recurseSync2ndSourceBOM(subChildBean, subBomLine, syncFlag, enable);
				vector<SecondSourceInfo> subChilds = subChildBean.childs;
				addSubBOMChildList(subList, subChilds, subChildBean); // �����BOM�������
			}
		}

	CLEANUP:
		DOFREE(sub_children);
		return ifail;
	}

	/*
	* �����BOM�������
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
