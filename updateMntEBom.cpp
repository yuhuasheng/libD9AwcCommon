#include "updateMntEBom.h"
#include "secondSourceCommonUtils.h"
#include <iomanip>

extern "C" int POM_AM__set_application_bypass(logical bypass);
#ifdef __cplusplus         
extern "C" {
#endif
	namespace UPDATEL6EBOM {

		tag_t doReviseForMntEbom(tag_t item, tag_t rev_tag) {
			tag_t new_rev_tag = NULLTAG;
			string ruleRev = getVersionRule(rev_tag);
			string newRevid = getVersionByRule(item, ruleRev);
			reviseRev(rev_tag, newRevid, &new_rev_tag);
			return new_rev_tag;
		}



		tag_t getTopBomline(string itemId, bool isDoRevise, bool isCreateBomView) {
			tag_t
				item_tag = NULLTAG,
				rev_tag = NULLTAG,
				//rev_rule = NULLTAG,
				window = NULLTAG,
				topBOMLine = NULLTAG;
			ITK_THROW_ERROR(ITEM_find_item(itemId.c_str(), &item_tag));
			ITK_THROW_ERROR(ITEM_ask_latest_rev(item_tag, &rev_tag));
			if (isRelease(rev_tag) && isDoRevise) {
				rev_tag = doReviseForMntEbom(item_tag, rev_tag);
			}
			if (isCreateBomView) {
				createBomView(item_tag, rev_tag);
			}
			//ITK_THROW_ERROR(CFM_find("Latest Working", &rev_rule));
			ITK_THROW_ERROR(BOM_create_window(&window));
			//ITK_THROW_ERROR(BOM_set_window_config_rule(window, rev_rule));
			//ITK_THROW_ERROR(BOM_set_window_pack_all(window, false));
			ITK_THROW_ERROR(BOM_set_window_top_line(window, item_tag, rev_tag, NULL, &topBOMLine));
			return topBOMLine;
		}



		void getParentItemIds(vector<json> jsonV, vector<string>& parentItems) {
			for (json bomJ : jsonV) {
				string parentPn = bomJ["parentItem"];
				if (count(parentItems.begin(), parentItems.end(), parentPn) == 0) {
					parentItems.push_back(parentPn);
				}
			}
		}

		void setBOMLineAttr(json jBom, tag_t  bomLine, bool isSub) {
			string attrVal = "";
			char* bomAttr = NULL;
			//ITKCALL(AOM_lock(bomLine));
			//ITKCALL(AOM_load(bomLine));
			tag_t uom = NULL;
			if (jBom.contains("packageType")) {
				attrVal = jBom["packageType"];
				ITKCALL(AOM_set_value_string(bomLine, "bl_occ_d9_PackageType", attrVal.c_str()));
			}
			if (jBom.contains("side")) {
				attrVal = jBom["side"];
				ITKCALL(AOM_set_value_string(bomLine, "bl_occ_d9_Side", attrVal.c_str()));
			}
			if (!isSub) {
				if (jBom.contains("location")) {
					attrVal = jBom["location"];
					ITKCALL(AOM_set_value_string(bomLine, "bl_occ_d9_Location", attrVal.c_str()));
				}
				if (jBom.contains("qty")) {
					attrVal = jBom["qty"];
					size_t index = attrVal.find(".", 0);
					if (index != string::npos) {
						string decimalStr = attrVal.substr(index + 1, attrVal.length());
						int  decimalInt = atoi(decimalStr.c_str());
						if (decimalInt > 0) {
							ITKCALL(UOM_find_by_symbol("Other", &uom));
							ITKCALL(AOM_set_value_tag(bomLine, "bl_uom", uom));
						}
					}
					ITKCALL(AOM_set_value_string(bomLine, "bl_quantity", attrVal.c_str()));
				}
				if (jBom.contains("findNum")) {
					attrVal = jBom["findNum"];
					if (attrVal.length() > 0) {
						ITKCALL(AOM_set_value_string(bomLine, "bl_sequence_no", attrVal.c_str()));
					}
				}
				if (attrVal.length() == 0) {
					ITKCALL(AOM_ask_value_string(bomLine, "bl_sequence_no", &bomAttr));
					attrVal = bomAttr;
				}
				vector<json> subs = jBom["secondSource"];
				if (subs.size() > 0) {
					attrVal = MNTEBOM2ndSource::getAltGroupByRule(atoi(attrVal.c_str()));
					ITKCALL(AOM_set_value_string(bomLine, "bl_occ_d9_AltGroup", attrVal.c_str()));
				}

				if (jBom.contains("referenceDimension")) {
					attrVal = jBom["referenceDimension"];
					ITKCALL(AOM_set_value_string(bomLine, "bl_occ_d9_ReferenceDimension", attrVal.c_str()));
				}
			}
			//ITKCALL(AOM_save_with_extensions(bomLine));
			//ITKCALL(AOM_unlock(bomLine));
		}

		void setItemAttr(tag_t itemRev, json jItem) {
			string attrVal = "";
			ITKCALL(AOM_lock(itemRev));
			ITKCALL(AOM_load(itemRev));
			if (jItem.contains("description")) {
				attrVal = jItem["description"];
				ITKCALL(AOM_set_value_string(itemRev, "d9_EnglishDescription", attrVal.c_str()));
			}
			if (jItem.contains("sapDescription")) {
				attrVal = jItem["sapDescription"];
				ITKCALL(AOM_set_value_string(itemRev, "d9_DescriptionSAP", attrVal.c_str()));
			}
			if (jItem.contains("mfg")) {
				attrVal = jItem["mfg"];
				ITKCALL(AOM_set_value_string(itemRev, "d9_ManufacturerID", attrVal.c_str()));
			}
			if (jItem.contains("mfgPn")) {
				attrVal = jItem["mfgPn"];
				ITKCALL(AOM_set_value_string(itemRev, "d9_ManufacturerPN", attrVal.c_str()));
			}
			if (jItem.contains("materialGroup")) {
				attrVal = jItem["materialGroup"];
				ITKCALL(AOM_set_value_string(itemRev, "d9_MaterialGroup", attrVal.c_str()));
			}
			if (jItem.contains("materialType")) {
				attrVal = jItem["materialType"];
				ITKCALL(AOM_set_value_string(itemRev, "d9_MaterialType", attrVal.c_str()));
			}
			if (jItem.contains("unit")) {
				attrVal = jItem["unit"];
				ITKCALL(AOM_set_value_string(itemRev, "d9_Un", attrVal.c_str()));
			}
			if (jItem.contains("sapRev")) {
				attrVal = jItem["sapRev"];
				ITKCALL(AOM_set_value_string(itemRev, "d9_SAPRev", attrVal.c_str()));
			}
			if (jItem.contains("description")) {
				attrVal = jItem["description"];
				ITKCALL(AOM_set_value_string(itemRev, "d9_EnglishDescription", attrVal.c_str()));
			}
			if (jItem.contains("supplierZF")) {
				attrVal = jItem["supplierZF"];
				ITKCALL(AOM_set_value_string(itemRev, "d9_SupplierZF", attrVal.c_str()));
			}
			ITKCALL(AOM_save_with_extensions(itemRev));
			ITKCALL(AOM_unlock(itemRev));
		}

		void addBomLine(vector<json> addBOMS, map<string, tag_t>& topBomlineMap) {
			tag_t
				subItemTag = NULLTAG,
				subItemRevTag = NULLTAG,
				itemRevTag = NULLTAG,
				itemTag = NULLTAG,
				topBomLine = NULLTAG,
				newBomLine = NULLTAG,
				newSubBomLine = NULLTAG;
			string
				fromSys = "",
				itemRevUid,
				parentPn;
			for (json jBom : addBOMS) {
				parentPn = jBom["parentItem"];
				topBomLine = topBomlineMap[parentPn];
				if (topBomLine == NULLTAG) {
					topBomLine = getTopBomline(parentPn, true, true);
					topBomlineMap[parentPn] = topBomLine;

				}
				fromSys = jBom["sourceSystem"];
				if (fromSys.compare("pnms") == 0) {
					//create
					ITK_THROW_ERROR(createItem(jBom["item"], jBom["description"], "EDAComPart", LetterPattern, &itemTag, &itemRevTag));
					// 设置新建的 item 属性
					setItemAttr(itemRevTag, jBom);
				}
				else {
					itemRevUid = jBom["itemRevUid"];
					ITK__convert_uid_to_tag(itemRevUid.c_str(), &itemRevTag);
					ITEM_ask_item_of_rev(itemRevTag, &itemTag);
				}
				ITK_THROW_ERROR(BOM_line_add(topBomLine, itemTag, itemRevTag, NULLTAG, &newBomLine));
				setBOMLineAttr(jBom, newBomLine, false);
				vector<json> subs = jBom["secondSource"];
				if (subs.size() > 0) {
					for (json jSub : subs) {
						itemRevUid = jSub["itemRevUid"];
						ITK__convert_uid_to_tag(itemRevUid.c_str(), &subItemRevTag);
						ITEM_ask_item_of_rev(subItemRevTag, &subItemTag);
						BOM_line_add_substitute(newBomLine, subItemTag, subItemRevTag, NULLTAG, &newSubBomLine);
						// 设置替代bom attr ; 
						setBOMLineAttr(jSub, newSubBomLine, true);
					}
				}
			}
		}

		tag_t findChildBOMLineByMntEBOM(tag_t topBomLine, string josnItemId, string jsonFindNum) {
			char* itemId = NULL,
				* findNum = NULL;
			int count = 0;
			tag_t* children = NULLTAG,
				bomLine = NULLTAG;
			bool compareItemId = false,
				compareFindNum = false;
			ITK_THROW_ERROR(BOM_line_ask_all_child_lines(topBomLine, &count, &children));
			for (int i = 0; i < count; i++) {
				ITK_THROW_ERROR(AOM_ask_value_string(children[i], bomAttr_itemId, &itemId));
				ITK_THROW_ERROR(AOM_ask_value_string(children[i], bomAttr_occSeqNo, &findNum));
				compareItemId = josnItemId.compare(itemId) == 0;
				compareFindNum = jsonFindNum.compare(findNum) == 0;
				if (compareItemId && compareFindNum) {
					bomLine = children[i];
					goto CLEARUP;
				}
			}
		CLEARUP:
			DOFREE(itemId);
			DOFREE(findNum);
			DOFREE(children);
			return bomLine;
		}

		void delBomLine(vector<json> delBOMS, map<string, tag_t>& topBomlineMap) {
			tag_t topBomLine = NULLTAG,
				childrenBomLine = NULLTAG;
			int count = 0;
			string parentPn;
			char* itemId = NULL;
			for (json jBom : delBOMS) {
				parentPn = jBom["parentItem"];
				topBomLine = topBomlineMap[parentPn];
				if (topBomLine == NULLTAG) {
					topBomLine = getTopBomline(parentPn, true, false);
					topBomlineMap[parentPn] = topBomLine;
				}
				childrenBomLine = findChildBOMLineByMntEBOM(topBomLine, jBom["item"], jBom["findId"]);
				ITK_THROW_ERROR(BOM_line_cut(childrenBomLine));
			}
		}

		bool compareLocation(string locationNew, string locationOld) {
			auto convectArrayFunc = [](string locationStr) ->vector<string> {
				vector<string> locationVector = split(locationStr, ",");
				sort(locationVector.begin(), locationVector.end());
				return locationVector;
			};
			vector<string> locationVectorNew = convectArrayFunc(locationNew);
			vector<string> locationVectorOld = convectArrayFunc(locationOld);
			return locationVectorNew != locationVectorOld;
		}

		void  changeBOMLine(vector<json> changeBOMs, map<string, tag_t>& topBomlineMap) {

			char* findNum = NULL,
				* location = NULL,
				* referenceDimension = NULL,
				* qty = NULL;

			string itemRevUid,
				parentPn,
				jsonQty,
				jsonFindNum,
				jsonLocation,
				jsonReferenceDimension;

			tag_t itemRev = NULLTAG,
				topBomLine = NULLTAG,
				childrenBomLine = NULLTAG,
				bomWindow = NULLTAG,
				parentItem = NULLTAG,
				parentItemRev = NULLTAG;

			bool isSecondSource = false,
				mapFlag = false,
				locationChange = false,
				qtyChange = false,
				findNumChange = false,
				referenceDimensionChange = false;


			for (json jBom : changeBOMs) {
				isSecondSource = jBom["isSecondSource"];
				mapFlag = false;
				itemRevUid = jBom["itemRevUid"];
				jsonQty = jBom["qty"];
				jsonFindNum = jBom["findNum"];
				jsonLocation = jBom["location"];
				jsonReferenceDimension = jBom["referenceDimension"];
				ITK__convert_uid_to_tag(itemRevUid.c_str(), &itemRev);
				setItemAttr(itemRev, jBom);
				// bom change 
				if (isSecondSource) {
					continue;
				}
				parentPn = jBom["parentItem"];
				topBomLine = topBomlineMap[parentPn];
				if (topBomLine == NULLTAG) {
					topBomLine = getTopBomline(parentPn, false, false);
					mapFlag = true;
				}
				childrenBomLine = findChildBOMLineByMntEBOM(topBomLine, jBom["item"], jBom["findId"]);
				ITKCALL(AOM_ask_value_string(childrenBomLine, BOM_ATTR::sequence_no, &findNum));
				ITKCALL(AOM_ask_value_string(childrenBomLine, BOM_ATTR::d9_Location, &location));
				ITKCALL(AOM_ask_value_string(childrenBomLine, BOM_ATTR::d9_ReferenceDimension, &referenceDimension));
				ITKCALL(AOM_ask_value_string(childrenBomLine, BOM_ATTR::quantity, &qty));
				if (jsonQty.length() == 0) {
					jsonQty = "0";
				}
				if (tc_strlen(qty) == 0) {
					tc_strcpy(qty, "0");
				}
				locationChange = compareLocation(jsonLocation, location);
				qtyChange = (stod(jsonQty) != stod(qty));
				findNumChange = jsonFindNum.compare(findNum) != 0;
				referenceDimensionChange = jsonReferenceDimension.compare(referenceDimension) != 0;
				if (locationChange || qtyChange || findNumChange || referenceDimensionChange) {
					if (mapFlag) {
						ITKCALL(AOM_ask_value_tag(topBomLine, bomAttr_lineItemTag, &parentItem));
						ITKCALL(AOM_ask_value_tag(topBomLine, bomAttr_lineItemRevTag, &parentItemRev));
						if (isRelease(parentItemRev)) {
							parentItemRev = doReviseForMntEbom(parentItem, parentItemRev);
							ITKCALL(BOM_line_ask_window(topBomLine, &bomWindow));
							ITK_THROW_ERROR(BOM_set_window_top_line(bomWindow, parentItem, parentItemRev, NULL, &topBomLine));
							//BOM_refresh_window(bomWindow);
							//AOM_refresh(topBomLine, false);
							childrenBomLine = findChildBOMLineByMntEBOM(topBomLine, jBom["item"], jBom["findId"]);
						}
					}
					topBomlineMap[parentPn] = topBomLine;
					// modify bom :
					if (findNumChange)
						ITKCALL(AOM_set_value_string(childrenBomLine, BOM_ATTR::sequence_no, jsonFindNum.c_str()));
					if (locationChange)
						ITKCALL(AOM_set_value_string(childrenBomLine, BOM_ATTR::d9_Location, jsonLocation.c_str()));
					if (referenceDimensionChange)
						ITKCALL(AOM_set_value_string(childrenBomLine, BOM_ATTR::d9_ReferenceDimension, jsonReferenceDimension.c_str()));
					if (qtyChange)
						ITKCALL(AOM_set_value_string(childrenBomLine, BOM_ATTR::quantity, jsonQty.c_str()));

				}
			}
		}

		tag_t getPreviousItemRev(tag_t itemRev) {
			int revCount = 0;
			tag_t item = NULLTAG,
				presiousRev = NULLTAG,
				* itemRevList = NULLTAG;
			ITKCALL(ITEM_ask_item_of_rev(itemRev, &item));
			ITKCALL(ITEM_list_all_revs(item, &revCount, &itemRevList));
			if (revCount > 0) {
				for (int i = 0; i < item; i++) {
					if (itemRevList[i] == itemRev) {
						if ((i - 1) >= 0) {
							presiousRev = itemRevList[i - 1];
							goto CLEARUP;
						}
					}
				}
			}
		CLEARUP:
			DOFREE(itemRevList);
			return presiousRev;
		}


		void addMNTECNRelation(tag_t primary_object, tag_t secondary_object, const char* relationTypeName) {
			tag_t
				relationType = NULLTAG,
				problemType = NULLTAG,
				presiousRev = NULLTAG,
				relationTag = NULLTAG;
			ITKCALL(GRM_find_relation_type(relationTypeName, &relationType));
			ITKCALL(GRM_find_relation(primary_object, secondary_object, relationType, &relationTag));
			if (relationTag == NULLTAG) {
				ITKCALL(AOM_lock(primary_object));
				ITKCALL(GRM_create_relation(primary_object, secondary_object, relationType, NULL, &relationTag));
				ITKCALL(GRM_save_relation(relationTag));
				ITKCALL(AOM_save_with_extensions(primary_object));
				ITKCALL(AOM_unlock(primary_object))
			}
		}

		string startUpdateBom(string  rootItemUid, vector<json> addBOMS, vector<json> delBOMS, vector<json> changeBOMS) {

			int ifail = ITK_ok;
			int markpoint_number = 0;
			vector<string> parentItems;
			map<string, tag_t>  topBomlineMap;
			tag_t
				previousItemRev = NULLTAG,
				dcnItemRev = NULLTAG,
				rootItemRev = NULLTAG,
				topBOMLine = NULLTAG,
				bomWindow = NULLTAG,
				parentItemRevTag = NULLTAG;
			string result = "";
			try {
				getParentItemIds(addBOMS, parentItems);
				getParentItemIds(delBOMS, parentItems);
				ITKCALL(POM_place_markpoint(&markpoint_number)); //设置markpoint
				// do things
				if (changeBOMS.size() > 0) {
					changeBOMLine(changeBOMS, topBomlineMap);
				}
				if (delBOMS.size() > 0) {
					delBomLine(delBOMS, topBomlineMap);
				}
				if (addBOMS.size() > 0) {
					addBomLine(addBOMS, topBomlineMap);
				}
				if (topBomlineMap.size() > 0) {
					ITK__convert_uid_to_tag(rootItemUid.c_str(), &rootItemRev);
					dcnItemRev = MNTEBOM::getNotReleasedDCNItemRev(rootItemRev);
					map<string, tag_t>::iterator bomMapIt;
					for (bomMapIt = topBomlineMap.begin(); bomMapIt != topBomlineMap.end(); bomMapIt++) {
						topBOMLine = bomMapIt->second;
						if (topBOMLine != NULLTAG) {
							ITK_THROW_ERROR(BOM_line_ask_window(topBOMLine, &bomWindow));
							ITK_THROW_ERROR(BOM_save_window(bomWindow));
							ITKCALL(AOM_ask_value_tag(topBOMLine, bomAttr_lineItemRevTag, &parentItemRevTag));
							ITKCALL(BOM_close_window(bomWindow));
							//add ECN
							if (dcnItemRev != NULLTAG) {
								addMNTECNRelation(dcnItemRev, parentItemRevTag, "CMHasSolutionItem");
								previousItemRev = getPreviousItemRev(parentItemRevTag);
								if (previousItemRev != NULLTAG) {
									addMNTECNRelation(dcnItemRev, previousItemRev, "CMHasProblemItem");
								}
							}
						}
					}
				}

				ITKCALL(POM_forget_markpoint(markpoint_number)); //释放markpoint
			}
			catch (int& error_code) {
				try {
					logical state_has_changed = true;//默认TC中数据有变化
					ITKCALL(POM_roll_to_markpoint(markpoint_number, &state_has_changed));

					if (!state_has_changed) {
						TC_write_syslog("回滚markpoint[%d]成功\n", markpoint_number);
					}
					else {
						TC_write_syslog("回滚markpoint[%d]失败\n", markpoint_number);
					}
				}
				catch (int& rollbackerror_code) {
					ifail = rollbackerror_code;
					TC_write_syslog("POM_roll_to_markpoint markpoint[%d] fail\n", markpoint_number);
				}
				char* error = NULL;
				ITKCALL(EMH_ask_error_text(error_code, &error));
				result = error;
				DOFREE(error);
			}
			return result;
		}
	}

	extern __declspec(dllexport) string updateMntEBom(string changeJsonData) {
		json changeJson = json::parse(changeJsonData);
		vector<json> addBOMS = changeJson["add"];
		vector<json> delBOMS = changeJson["del"];
		vector<json> changeBOMS = changeJson["change"];
		return  UPDATEL6EBOM::startUpdateBom(changeJson["rootItemRev"], addBOMS, delBOMS, changeBOMS);
	}

#ifdef __cplusplus
}
#endif

