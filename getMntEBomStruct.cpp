#include "getMntEBomStruct.h"


#ifdef __cplusplus         
extern "C" {
#endif

	const char* bom_attr[] = { BOM_ATTR::d9_Location,BOM_ATTR::d9_PackageType,BOM_ATTR::d9_ReferenceDimension,BOM_ATTR::d9_Side,BOM_ATTR::quantity,BOM_ATTR::ref_designator,BOM_ATTR::sequence_no };
	const char* item_attr[] = { ITEM_ATTR::item_id , ITEM_ATTR::current_revision_id , ITEM_ATTR::d9_DescriptionSAP,ITEM_ATTR::d9_EnglishDescription,ITEM_ATTR::d9_ManufacturerID,ITEM_ATTR::d9_ManufacturerPN,ITEM_ATTR::d9_MaterialGroup,ITEM_ATTR::d9_MaterialType,ITEM_ATTR::d9_ProcurementMethods,ITEM_ATTR::d9_SAPRev,ITEM_ATTR::d9_SupplierZF,ITEM_ATTR::d9_TempPN,ITEM_ATTR::d9_Un };

	BOMTYPE eBomType = BOMTYPE::MNT_L6;
	mutex mutex_;

	namespace MNTEBOM {


		EBOMLineBean getBOMStruct(tag_t bomLine, bool flag);
		json setBOMAttrJson(tag_t bomLine);
		string getMntEBomStructByBOMWindow(string param);
		string getMntEBomStructByItemRev(string param);
		BOMTYPE getEBOMType(tag_t toBOMLine);
		EBOMLineBean getBOMStructFuture(tag_t bomLine);
		void  getOtherBOMAttr(tag_t itemRev, EBOMLineBean& bomBean);


		string getMntEBomStructByBOMWindow(string bomWindowUid) {
			json bomJson;
			tag_t
				itemRev = NULLTAG,
				bomWindow = NULLTAG,
				topBOMLine = NULLTAG;
			ITK__convert_uid_to_tag(bomWindowUid.c_str(), &bomWindow);
			if (bomWindow != NULLTAG) {
				ITKCALL(BOM_set_window_pack_all(bomWindow, false));
				ITKCALL(BOM_refresh_window(bomWindow));
				ITKCALL(BOM_ask_window_top_line(bomWindow, &topBOMLine));
				if (topBOMLine != NULLTAG) {
					ITKCALL(AOM_ask_value_tag(topBOMLine, bomAttr_lineItemRevTag, &itemRev));
					eBomType = getEBOMType(topBOMLine);
					unpackBomLine(topBOMLine);
					EBOMLineBean bomBean = getBOMStruct(topBOMLine, false);
					getOtherBOMAttr(itemRev, bomBean);
					//EBOMLineBean bomBean = getBOMStructFuture(topBOMLine);
					bomJson = bomBean;
				}
				else {
					TC_write_syslog("error ,get topBOMLine is null , bomwindowIid  == %s\n", bomWindowUid);
				}
			}
			string result = bomJson.dump();

			return result;
		}

		string getMntEBomStructByItemRev(string itemRevUid) {
			json bomJson;
			tag_t
				item_tag = NULLTAG,
				rev_tag = NULLTAG,
				rev_rule = NULLTAG,
				window = NULLTAG,
				topBOMLine = NULLTAG;
			char* itemId;
			ITK__convert_uid_to_tag(itemRevUid.c_str(), &rev_tag);
			cout << " get bom :: " + itemRevUid << endl;
			if (rev_tag == NULLTAG) {
				TC_write_syslog("error , get ebom is null , uid=%s ", itemRevUid);
				return "";
			}
			ITKCALL(ITEM_ask_item_of_rev(rev_tag, &item_tag));
			ITKCALL(CFM_find("Latest Working", &rev_rule));
			ITKCALL(BOM_create_window(&window));
			ITKCALL(BOM_set_window_config_rule(window, rev_rule));
			ITKCALL(BOM_set_window_pack_all(window, false));
			ITKCALL(BOM_set_window_top_line(window, item_tag, rev_tag, NULL, &topBOMLine));
			if (topBOMLine != NULL) {
				eBomType = getEBOMType(topBOMLine);
				unpackBomLine(topBOMLine);
				EBOMLineBean bomBean = getBOMStruct(topBOMLine, false);
				getOtherBOMAttr(rev_tag, bomBean);
				//bomBean.isBOMViewWFTask = attrBean.isBOMViewWFTask;
				//bomBean.isCanDcn = attrBean.isCanDcn;
				//bomBean.isNewVersion = attrBean.isNewVersion;
				//bomBean.showDifference = attrBean.showDifference;
				//EBOMLineBean bomBean = getBOMStructFuture(topBOMLine);
				bomJson = bomBean;
			}
			else {
				TC_write_syslog("error ,get topBOMLine is null , itemRevUid  == %s\n", itemRevUid);
			}
			if (window != NULL) {
				ITKCALL(BOM_close_window(window));
			}
			string result = bomJson.dump().c_str();
			bomJson.clear();

			return result;
		}

		bool isExpandChilds_(string materialGroup) {
			return eBomType == BOMTYPE::MNT_L10 && materialGroup.find("B8X80") == 0;
		}

		EBOMLineBean  convertBOMBean(tag_t bomLine) {
			//mutex_.lock();
			tag_t itemRev = NULL, * statusTags = NULL;
			int statusLen = 0;
			char* attrVal;
			EBOMLineBean bomBean;
			ITKCALL(AOM_ask_value_string(bomLine, BOM_ATTR::sequence_no, &attrVal));
			bomBean.findNum = attrVal;
			//ITKCALL(AOM_ask_value_string(bomLine, BOM_ATTR::fnd0objectId, &attrVal));
			bomBean.findId = attrVal;
			ITKCALL(AOM_ask_value_string(bomLine, BOM_ATTR::d9_AltGroup, &attrVal));
			bomBean.alternativeGroup = attrVal;
			ITKCALL(AOM_ask_value_string(bomLine, BOM_ATTR::d9_Location, &attrVal));
			bomBean.location = attrVal;
			ITKCALL(AOM_ask_value_string(bomLine, BOM_ATTR::d9_PackageType, &attrVal));
			bomBean.packageType = attrVal;
			ITKCALL(AOM_ask_value_string(bomLine, BOM_ATTR::d9_ReferenceDimension, &attrVal));
			if (bomBean.location.length() == 0 && tc_strlen(attrVal) > 0) {
				bomBean.location = attrVal;
			}
			ITKCALL(AOM_ask_value_string(bomLine, BOM_ATTR::d9_Side, &attrVal));
			bomBean.side = attrVal;
			ITKCALL(AOM_ask_value_string(bomLine, BOM_ATTR::quantity, &attrVal));
			bomBean.qty = attrVal;
			ITK__convert_tag_to_uid(bomLine, &attrVal);
			bomBean.uid = attrVal;
			ITKCALL(AOM_ask_value_tag(bomLine, bomAttr_lineItemRevTag, &itemRev));
			ITKCALL(AOM_ask_value_string(itemRev, ITEM_ATTR::current_revision_id, &attrVal));
			bomBean.version = attrVal;
			ITKCALL(AOM_ask_value_string(itemRev, ITEM_ATTR::d9_DescriptionSAP, &attrVal));
			bomBean.sapDescription = attrVal;
			ITKCALL(AOM_ask_value_string(itemRev, ITEM_ATTR::d9_EnglishDescription, &attrVal));
			bomBean.description = attrVal;
			ITKCALL(AOM_ask_value_string(itemRev, ITEM_ATTR::d9_ManufacturerID, &attrVal));
			bomBean.mfg = attrVal;
			ITKCALL(AOM_ask_value_string(itemRev, ITEM_ATTR::d9_ManufacturerPN, &attrVal));
			bomBean.mfgPn = attrVal;
			ITKCALL(AOM_ask_value_string(itemRev, ITEM_ATTR::d9_MaterialGroup, &attrVal));
			bomBean.materialGroup = attrVal;
			ITKCALL(AOM_ask_value_string(itemRev, ITEM_ATTR::d9_MaterialType, &attrVal));
			bomBean.materialType = attrVal;
			ITKCALL(AOM_ask_value_string(itemRev, ITEM_ATTR::d9_ProcurementMethods, &attrVal));
			bomBean.procurementType = attrVal;
			ITKCALL(AOM_ask_value_string(itemRev, ITEM_ATTR::d9_SAPRev, &attrVal));
			bomBean.sapRev = attrVal;
			ITKCALL(AOM_ask_value_string(itemRev, ITEM_ATTR::d9_SupplierZF, &attrVal));
			bomBean.supplierZF = attrVal;
			ITKCALL(AOM_ask_value_string(itemRev, ITEM_ATTR::d9_TempPN, &attrVal));
			bomBean.tempPN = attrVal;
			ITKCALL(AOM_ask_value_string(itemRev, ITEM_ATTR::d9_Un, &attrVal));
			bomBean.unit = attrVal;
			ITKCALL(AOM_ask_value_string(itemRev, ITEM_ATTR::item_id, &attrVal));
			bomBean.item = attrVal;
			ITKCALL(AOM_ask_value_tags(itemRev, ITEM_ATTR::release_status_list, &statusLen, &statusTags));
			if (statusLen > 0) {
				ITKCALL(AOM_ask_value_string(statusTags[0], "name", &attrVal));
				bomBean.status = attrVal;
			}
			ITK__convert_tag_to_uid(itemRev, &attrVal);
			bomBean.itemRevUid = attrVal;
			//mutex_.unlock();
			DOFREE(attrVal);
			return bomBean;
		}



		vector<EBOMLineBean> getSubBOMBean(tag_t mainBOMLine, EBOMLineBean mainBOMBean) {
			tag_t* subLines;
			int subSize = 0;
			logical  hasSub = false;
			vector<EBOMLineBean> subs;
			char* itemId = "";
			ITKCALL(BOM_line_ask_has_substitutes(mainBOMLine, &hasSub));
			if (hasSub) {
				ITKCALL(BOM_line_list_substitutes(mainBOMLine, &subSize, &subLines));
				for (int i = 0; i < subSize; i++) {
					tag_t subBomline = subLines[i];
					EBOMLineBean subBean;
					if (BOMTYPE::MNT_L10 == eBomType) {
						subBean = getBOMStruct(subBomline, true);
					}
					else {
						subBean = convertBOMBean(subBomline);
					}
					subBean.isSecondSource = true;
					subBean.mainSource = mainBOMBean.item;
					subBean.parentItem = mainBOMBean.parentItem;
					subBean.parentRevUid = mainBOMBean.parentRevUid;
					subBean.alternativeCode = "ALT";
					subs.push_back(subBean);
				}
				return subs;
			}
			return subs;
		}


		json setBOMAttrJson(tag_t bomLine) {
			tag_t itemRev = NULL, * statusTags = NULL;
			int statusLen = 0;
			int attrSize = end(bom_attr) - begin(bom_attr);
			int itemAttrSize = end(item_attr) - begin(item_attr);
			char* attrVal, * bomLineUid, * itemRevUid, * statusName;
			json bomJson;
			for (int i = 0; i < attrSize; i++) {
				ITKCALL(AOM_ask_value_string(bomLine, bom_attr[i], &attrVal));
				if (attrVal != NULL) {
					bomJson[bom_attr[i]] = attrVal;
				}
			}
			ITK__convert_tag_to_uid(bomLine, &bomLineUid);
			bomJson["uid"] = bomLineUid;
			// item :
			AOM_ask_value_tag(bomLine, bomAttr_lineItemRevTag, &itemRev);
			ITK__convert_tag_to_uid(itemRev, &itemRevUid);
			for (int i = 0; i < itemAttrSize; i++) {
				ITKCALL(AOM_ask_value_string(itemRev, item_attr[i], &attrVal));
				if (attrVal != NULL) {
					bomJson[item_attr[i]] = attrVal;
				}
			}
			//ITEM_ATTR::release_status_list 
			AOM_ask_value_tags(itemRev, ITEM_ATTR::release_status_list, &statusLen, &statusTags);
			if (statusLen > 0) {
				AOM_ask_value_string(statusTags[0], "name", &statusName);
				bomJson[ITEM_ATTR::release_status_list] = statusName;
			}
			bomJson[MNTEBOM_ATTR::itemRevUid] = itemRevUid;
			return bomJson;
		}

		json getSubs(tag_t mainBOMLine, json mainBOMJson) {
			tag_t* subLines;
			int subSize = 0;
			logical  hasSub = false;
			BOM_line_ask_has_substitutes(mainBOMLine, &hasSub);
			if (hasSub) {
				json arrayJson = json::array();
				BOM_line_list_substitutes(mainBOMLine, &subSize, &subLines);
				for (int i = 0; i < subSize; i++) {
					json subBomJson = setBOMAttrJson(subLines[i]);
					subBomJson[MNTEBOM_ATTR::IsSecondSource] = true;
					subBomJson[MNTEBOM_ATTR::MainSource] = mainBOMJson[ITEM_ATTR::item_id];
					subBomJson[MNTEBOM_ATTR::ParentItem] = mainBOMJson[MNTEBOM_ATTR::ParentItem];
					subBomJson[MNTEBOM_ATTR::ParentRevUid] = mainBOMJson[MNTEBOM_ATTR::ParentRevUid];
					subBomJson[MNTEBOM_ATTR::alternativeCode] = "ALT";
					arrayJson.push_back(subBomJson);
				}
				return arrayJson;
			}
			return NULL;
		}

		EBOMLineBean getBOMStruct(tag_t bomLine, bool flag) {
			EBOMLineBean bomBean;
			if (bomLine != NULL) {
				tag_t* childrens;
				tag_t childBOMLine;
				int count = 0;
				logical isSub = false;
				ITKCALL(BOM_line_ask_is_substitute(bomLine, &isSub));
				if (!isSub || flag) {
					bomBean = convertBOMBean(bomLine);
					if (isExpandChilds_(bomBean.materialGroup)) {
						return bomBean;
					}
					ITKCALL(BOM_line_ask_all_child_lines(bomLine, &count, &childrens));
					if (count > 0) {
						vector<EBOMLineBean> childBeans;
						for (int i = 0; i < count; i++) {
							childBOMLine = childrens[i];
							EBOMLineBean childBean = getBOMStruct(childBOMLine, false);
							if (childBean.item.length() > 0) {
								childBean.parentItem = bomBean.item;
								childBean.parentRevUid = bomBean.itemRevUid;
								vector<EBOMLineBean> subVector = getSubBOMBean(childBOMLine, childBean);
								if (subVector.size() > 0) {
									childBean.alternativeCode = "PRI";
									childBean.secondSource = subVector;
								}
								childBeans.push_back(childBean);
							}
						}
						bomBean.childs = childBeans;
					}
				}
			}
			return bomBean;
		}



		EBOMLineBean getBOMStructFuture(tag_t bomLine) {
			cout << "thread :: " << this_thread::get_id() << endl;
			EBOMLineBean  bomBean;
			if (bomLine != NULL) {
				tag_t* childrens;
				tag_t childBOMLine;
				int count = 0;
				logical isSub = false, hasChild = false;
				ITKCALL(BOM_line_ask_is_substitute(bomLine, &isSub));
				if (!isSub) {
					bomBean = convertBOMBean(bomLine);
					if (isExpandChilds_(bomBean.materialGroup)) {
						return bomBean;
					}
					ITKCALL(BOM_line_ask_all_child_lines(bomLine, &count, &childrens));
					if (count > 0) {
						vector<EBOMLineBean> childBeans;
						vector<future<EBOMLineBean>> futures;
						for (int i = 0; i < count; i++) {
							childBOMLine = childrens[i];
							ITKCALL(AOM_ask_value_logical(childBOMLine, bomAttr_lineHasChildren, &hasChild));
							if (hasChild) {
								future<EBOMLineBean> fuTask = async(getBOMStructFuture, childBOMLine);
								futures.push_back(move(fuTask));
							}
							else {
								EBOMLineBean childBean = getBOMStructFuture(childBOMLine);
								if (childBean.item.length() > 0) {
									childBeans.push_back(childBean);
								}
							}
						}
						if (futures.size() > 0) {
							vector<future<EBOMLineBean>>::iterator itFu;
							for (itFu = futures.begin(); itFu != futures.end(); ++itFu) {
								EBOMLineBean bomBean = (*itFu).get();
								if (bomBean.item.length() > 0) {
									childBeans.push_back(bomBean);
								}
							}

						}
						if (childBeans.size() > 0) {
							for (int i = 0; i < childBeans.size(); i++) {
								EBOMLineBean childBean = childBeans[i];
								childBean.parentItem = bomBean.item;
								childBean.parentRevUid = bomBean.itemRevUid;
								vector<EBOMLineBean> subVector = getSubBOMBean(childBOMLine, childBean);
								if (subVector.size() > 0) {
									childBean.alternativeCode = "PRI";
									childBean.secondSource = subVector;
								}
								childBeans[i] = childBean;
							}
						}

						bomBean.childs = childBeans;
					}
				}
			}
			return  bomBean;
		}


		BOMTYPE getEBOMType(tag_t toBOMLine) {
			char* itemId = "";
			ITKCALL(AOM_ask_value_string(toBOMLine, bomAttr_itemId, &itemId));
			if (tc_strlen(itemId) > 0) {
				if (itemId[0] == '8') {
					return  BOMTYPE::MNT_L10;
				}
			}
			return  BOMTYPE::MNT_L6;
		}

		json  getBOMStruct_json(tag_t bomLine) {
			if (bomLine != NULL) {
				tag_t* childrens;
				tag_t childBOMLine;
				int count = 0;
				logical isSub = false;
				ITKCALL(BOM_line_ask_is_substitute(bomLine, &isSub));
				if (!isSub) {
					json bomJson = setBOMAttrJson(bomLine);
					ITKCALL(BOM_line_ask_all_child_lines(bomLine, &count, &childrens));
					if (count > 0) {
						json arrayJson = json::array();
						for (int i = 0; i < count; i++) {
							childBOMLine = childrens[i];
							json childJson = getBOMStruct_json(childBOMLine);
							if (childJson != NULL) {
								childJson[MNTEBOM_ATTR::ParentItem] = bomJson[ITEM_ATTR::item_id];
								childJson[MNTEBOM_ATTR::ParentRevUid] = bomJson[MNTEBOM_ATTR::itemRevUid];
								json subJson = getSubs(childBOMLine, childJson);
								if (subJson != NULL) {
									childJson[MNTEBOM_ATTR::alternativeCode] = "PRI";
									childJson[MNTEBOM_ATTR::secondSource] = subJson;
								}
								arrayJson.push_back(childJson);
								cout << "bom " << bomJson[ITEM_ATTR::item_id] << " -- " << childJson[ITEM_ATTR::item_id] << endl;
							}
						}
						bomJson["childs"] = arrayJson;
					}
					return bomJson;
				}
			}
			return NULL;
		}

		bool isSignOffNode(tag_t itemRev, vector<string> wfNodes) {
			char
				* taskName = NULL,
				* wfName = NULL;
			int len;
			tag_t
				* tasks = NULLTAG,
				task = NULLTAG,
				parentTask = NULLTAG,
				tempTask = NULLTAG,
				wf;

			ITKCALL(AOM_ask_value_tags(itemRev, "fnd0MyWorkflowTasks", &len, &tasks));
			if (len > 0) {
				//parent_name
				task = tasks[0];
				ITKCALL(EPM_ask_parent_task(tasks[0], &parentTask));
				ITKCALL(EPM_ask_parent_task(parentTask, &tempTask));
				if (tempTask != NULLTAG) {
					task = parentTask;
				}
				ITKCALL(AOM_ask_name(task, &taskName));
				ITKCALL(AOM_ask_value_string(task, "parent_name", &wfName));

				if (wfNodes.size() > 0) {
					string splitStr = "=";
					string wfNodeStrs = "";
					vector<string> wfStrs;
					for (int i = 0; i < wfNodes.size(); i++) {
						wfNodeStrs = wfNodes[i];
						if (wfNodeStrs.find(wfName) == 0) {
							string taskNameStr = wfNodeStrs.substr(wfNodeStrs.find(splitStr) + splitStr.length());
							wfStrs = split(taskNameStr, splitStr);
							if (wfStrs.size() > 0 && count(wfStrs.begin(), wfStrs.end(), taskName)) {
								return true;
							}
						}
					}
				}
			}
			return false;
		}

		tag_t getNotReleasedDCNItemRev(tag_t itemRev) {

			int
				status_num = 0,
				n_referencers = 0,
				* levels = NULL;

			tag_t
				dcnItemRev = NULLTAG,
				* status_list = NULLTAG,
				* referencers = NULLTAG,
				relationType = NULLTAG;
			char
				** relations = NULL,
				* relationTypeName = NULL;

			ITKCALL(WSOM_where_referenced2(itemRev, 1, &n_referencers, &levels, &referencers, &relations));
			if (n_referencers > 0) {
				for (int i = 0; i < n_referencers; i++) {
					ITKCALL(TCTYPE_ask_object_type(referencers[i], &relationType));
					ITKCALL(TCTYPE_ask_name2(relationType, &relationTypeName));
					ITKCALL(AOM_ask_value_tags(referencers[i], ITEM_ATTR::release_status_list, &status_num, &status_list));
					if (tc_strcmp(relationTypeName, "D9_MNT_DCNRevision") == 0 && status_num == 0) {
						dcnItemRev = referencers[i];
						goto CLEARUP;
					}
				}
			}
		CLEARUP:
			DOFREE(status_list);
			DOFREE(referencers);
			DOFREE(relationTypeName);
			DOFREE(relations);
			DOFREE(levels);
			return dcnItemRev;

		}

		void  getOtherBOMAttr(tag_t itemRev, EBOMLineBean& bomBean) {
			//EBOMLineBean bomAttr;
			int
				status_num = 0,
				wfLen = 0,
				n_referencers = 0,
				* levels = NULL;
			tag_t
				* status_list = NULLTAG,
				* wfs = NULLTAG,
				* referencers = NULLTAG,
				user = NULLTAG,
				userGroup = NULLTAG,
				dcnItemRev = NULLTAG,
				item = NULLTAG,
				lastItemRev = NULLTAG,
				relationType = NULLTAG;

			char
				* itemTypeName = NULL,
				* userName = NULL,
				* userGroupName = NULL,
				** relations = NULL,
				* relationTypeName;
			bool
				isCanDcn = true,
				canEditPIEE = false,
				isNewVersion = false,
				isShowDifference = false,
				isBOMViewWFTask = false;
			//isCanDcn
			//ITKCALL(WSOM_where_referenced2(itemRev, 1, &n_referencers, &levels, &referencers, &relations));
			//if (n_referencers > 0) {
			//	for (int i = 0; i < n_referencers; i++) {
			//		dcnItemRev = referencers[i];
			//		ITKCALL(TCTYPE_ask_object_type(dcnItemRev, &relationType));
			//		ITKCALL(TCTYPE_ask_name2(relationType, &relationTypeName));
			//		ITKCALL(AOM_ask_value_tags(dcnItemRev, ITEM_ATTR::release_status_list, &status_num, &status_list));
			//		if (tc_strcmp(relationTypeName, "D9_MNT_DCNRevision") == 0 && status_num == 0) {
			//			isCanDcn = false;
			//			break;
			//		}
			//	}
			//}
			dcnItemRev = getNotReleasedDCNItemRev(itemRev);
			isCanDcn = dcnItemRev == NULLTAG;
			bomBean.isCanDcn = isCanDcn;
			// canEditPIEE  userBu
			ITKCALL(AOM_ask_value_tags(itemRev, "fnd0AllWorkflows", &wfLen, &wfs));
			ITKCALL(POM_get_user(&userName, &user));
			ITKCALL(SA_ask_user_login_group(user, &userGroup));
			ITKCALL(SA_ask_group_full_name(userGroup, &userGroupName));
			bomBean.userBu = "MNT";
			if (tc_strstr(userGroupName, "Printer")) {
				bomBean.userBu = "PRT";
			}
			bomBean.canEditPIEE = (wfLen == 0 && (tc_strcmp(userGroupName, "PSU.R&D.Monitor.D_Group") == 0 || tc_strcmp(userGroupName, "EE.R&D.Monitor.D_Group") == 0));
			// isNewVersion
			ITKCALL(ITEM_ask_item_of_rev(itemRev, &item));
			ITEM_ask_latest_rev(item, &lastItemRev);
			ITEM_ask_type2(itemRev, &itemTypeName);
			bomBean.isNewVersion = (lastItemRev == itemRev || tc_strcmp("D9_EE_PCBARevision", itemTypeName) == 0);
			//isBOMViewWFTask
			vector<string>* bomWfNode = new vector<string>();
			getPreferenceByName("D9_MNT_EBOM_WFNode", *bomWfNode);
			bomBean.isBOMViewWFTask = isSignOffNode(itemRev, *bomWfNode);
			//isShowDifference
			vector<string>  showDifferenceWf;
			showDifferenceWf.push_back("FXN31_MNT BOM CoWork Process=2-EE Review");
			bomBean.showDifference = isSignOffNode(itemRev, showDifferenceWf);
		}
	}
	__declspec(dllexport) string getMntEBomStruct(string param) {
		json input = json::parse(param);
		string result = "";
		if (input.contains("bomWindowUid")) {
			result = MNTEBOM::getMntEBomStructByBOMWindow(input["bomWindowUid"]);
		}
		else if (input.contains("itemRevUid")) {
			result = MNTEBOM::getMntEBomStructByItemRev(input["itemRevUid"]);
		}
		return result;
	}
#ifdef __cplusplus
}
#endif