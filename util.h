#ifndef UTIL
#define UTIL

#pragma comment(lib, "oraocci19.lib")
#pragma comment(lib, "oraocci19d.lib")

#include <Windows.h>
//#include <tc/iman.h>                -----------20221222
#include <tc/emh.h>
#include <pom/pom/pom_errors.h>
//#include <tc/tc.h>                -----------20221222
#include <pom/pom/pom.h>

#include <stdio.h>
#include <string.h>
#include <string>
#include <fclasses/tc_date.h>
#include <time.h>
#include <base_utils/Mem.h>
#include <tc/preferences.h>
#include <vector>
#include <tc/tc_macros.h> //ITKCALL 的头文件
#include <tccore/aom_prop.h>
#include <tccore/grm.h>
//#include <tccore/iman_grmtype.h>                -----------20221222
//#include "ADOSql.h"

#include <io.h>
//#include <direct.h>
#include <tccore/aom.h>
//#include "common_itk_util.h"

#include <vector>
#include <map>
//#include <itk/mem.h>                -----------20221222
#include <stdio.h>
#include <stdlib.h>
#include <map>
#include <string>
#include <iostream>
#include "version_pattern.h"
//#include <tc/tc.h>                 -----------20221222
#include <epm/cr.h>
#include <epm/signoff.h>
#include <epm/epm.h>
#include <tccore/workspaceobject.h>
#include <tccore/aom.h>
#include <tccore/aom_prop.h>
#include <tccore/grm.h>
#include <tccore/project.h>
#include <ae/dataset.h>
#include <ae/datasettype.h>
#include <fclasses/tc_date.h>
#include <user_exits/epm_toolkit_utils.h>
#include <epm/epm.h>
#include <tc/tc_arguments.h>
#include <ict/ict_userservice.h>
#include <io.h>
#include <direct.h>
#include <codecvt>  
#include <locale> 
#include <tc/emh.h>
#include <tccore/item.h>
#include <bom/bom.h>
#include <sa/am.h>
#include <sa/person.h>
#include <ug_va_copy.h>
#include <epm/epm.h>
#include <epm/epm_task_template_itk.h>
#include <schmgt/schmgt_bridge_itk.h>
#include <ae/dataset_msg.h>
#include <cfm/cfm.h>
#include <epm/epm_toolkit_tc_utils.h>
#include <regex>
#include <ics/ics.h>
#include <ics/ics2.h>
#include <lov/lov.h>
#include <res\res_itk.h>
#include <property/prop.h>

#include <tc/envelope.h>

#include <ae/dataset.h>

#include <time.h>
#include <regex>
#include <occi.h>
#include <errno.h>
#include <fstream>
#include <dispatcher/dispatcher_itk.h>
#include <curl/curl.h>
#include <cassert>
#include <nls/nls.h>
#include "tcprop.h"
#include "search.h"
#include "preference.h"
#include <property/nr.h>
//#include <sql.h>


#define ITK_THROW_ERROR( X ) {                                              \
	writeLog ( "ITK CALL: %s LINE: %d\n",						    		\
	#X,																        \
	__LINE__ );																\
   int ifail      =   ITK_ok;                                               \
   if( ( ifail = ( X ) ) != ITK_ok )                                        \
   {                                                                        \
      int      n_ifails     =   0;                                          \
      const int *severities   =   NULL;                                     \
      const int *ifails      =   NULL;                                      \
      const char **texts     =   NULL;                                      \
      EMH_ask_errors( &n_ifails, &severities, &ifails, &texts );            \
      if( n_ifails > 0 )                                                    \
      {                                                                     \
         if( texts[n_ifails-1] )                                            \
         {                                                                  \
            writeLog("ERROR: %d ERROR LEVEL:%d ERROR MSG: %s.\n",   		\
                           ifails[n_ifails-1],                              \
						   severities[n_ifails-1],							\
                           texts[n_ifails-1] );                             \
            writeLog("ERROR: FILE: %s LINE: %d\n",                 		\
                           __FILE__,                                        \
                           __LINE__ );                                      \
         }                                                                  \
      }                                                                     \
      throw ifail;                                                          \
   }                                                                        \
}

#define DOFREE(obj)								\
{												\
	if(obj != NULL)										\
	{											\
		MEM_free(obj);							\
		obj = NULL;								\
	}											\
}


#define TCFREE(obj)								\
	{												\
	if(obj)										\
	{											\
	MEM_free(obj);							\
	obj = NULL;								\
	}											\
}


extern "C" int POM_AM__set_application_bypass(logical bypass);
using namespace std;
using namespace oracle::occi;
using namespace ITEM_ATTR;
using namespace BOM_ATTR;
using namespace PROJECT;

class Messagebox {
public:
	static const int infomation = EMH_severity_information;
	static const int warning = EMH_severity_warning;
	static const int error = EMH_severity_error;
	static const int userError = EMH_severity_user_error;
	static int post(const char* infomation, int TYPE)
	{

		EMH_store_error_s1(TYPE, EMH_USER_error_base, infomation);
		return EMH_USER_error_base;
	}
};

#define _CRT_SECURE_NO_WARNINGS
#define BUFFER_SIZE 256
#define MAX_PATH_LENGTH 2000
#define MAX_PRINTLINE_LENGTH 2000
#define BUFSIZE				512
#define ERRORCODE_MESSAGE (EMH_USER_error_base + 110)
#define PATH_SEPARATOR "\\"
bool regex_matchstr(string search_string, std::tr1::regex reg);
vector<string> getReviseBeforeItemRevList(tag_t* rev_list, int count);
void current_time(date_t* date_tag);
void formatDate(char* drawingDate, const char* formdate, date_t date);
bool getPreferenceByName(string prefername, vector<string>& preference_vec);
int getHashMapPreference(vector<string> preference_vec, map<string, string>& mp);
vector<string> split(string str, string pattern);
//bool getPreferenceInfo(vector<string> &preference_vec,pre_infosql &preference_info);
int query(char* qry_name, char** attr_names, char** values, int attr_cnt, tag_t** results, int* results_cnt);
int export_dataset_file(tag_t dataset, char* ref_name, char* ext, char** filename, char** original_name, char* dirPath);
int execmd(char* cmd, char* result);
int import_dataset_file(tag_t dataset, char* ref_name, char* ext, char* fullfilename, char* original_name);
int create_dataset(char* ds_type, char* ref_name, char* ds_name, char* fullfilename, char* relation_name, tag_t parent_rev, tag_t* dataset);
int FindTargetTool(tag_t ds_type, const char* ref_name, tag_t* target_tool);
void str_replace(char* str1, char* str2, char* str3);
bool isNumeric(const  char* str);
bool isDigNumeric(const char* str);
char* U2G(const char* utf8);
char* G2U(const char* gb2312);
char* G2B5(char* gb2312);
string getTemplateId(vector<string> vec, char* templateName);
logical checkItemType(vector<string> tagTypeVec, tag_t rev_tag);
char* getProjectInfo(tag_t item_tag);
bool getProjectInfo(tag_t item_tag, vector<string>& preference_vec);
int getProjectInfo(tag_t item_tag, vector<tag_t>& vec);
int GTCTYPE_is_type_of(tag_t object_tag, const char* typeName, logical* isType);
int checkTagAttr(vector<string>  tagAttrVec, tag_t  rev_tag, char** error_msg);
int checkStrAttr(vector<string>  tagAttrVec, tag_t  rev_tag, char** error_msg);
string getMaterialGroup(char* groupName, vector<string> preference_vec);
int startWith(char* str, char* p);
int endWith(char* str, char* p);
int containStr(char* str, char* p);
int strReplace(char strRes[], char from[], char to[]);
bool startWithStr(const std::string& str, const std::string prefix);
bool endWithStr(const std::string& str, const std::string suffix);
bool isContainsStr(const std::string& str, const std::string contains_str);
void string_replace(string& s1, const string& s2, const string& s3);
logical checkStructure(tag_t itemRev);
logical hasChildBOMLine(tag_t tItemRev);
static inline bool is_base64(unsigned char c);
std::string base64_encode(char const* bytes_to_encode, int in_len); // base64 编码
std::string base64_decode(std::string& encoded_string); // base64 解码
logical checkProName(tag_t primaryTag, char* proName);
void createFolder(std::string dir);
void deleteFolder(std::string dir);
std::string getTempFolder(std::string dir);
bool createFile(std::string fileName);
void appendContent(const char* fileName, vector<string> content);
bool is_special_dir(const char* path);
bool is_dir(int attrib);
void show_error(const char* file_name);
void get_file_path(const char* path, const char* file_name, char* file_path);
void delete_file(const char* path);
void removeDir(const char* dirPath);
void getFiles(string path, vector<string>& files);
boolean rename(string oldName, string newName);
void getConnection(string prefername, Environment*& env, Connection*& conn);
int validateProp(tag_t object_tag, const char* propertyName, logical* verdict);
bool isBom(tag_t object_tag);
size_t ReceiveData(void* contents, size_t size, size_t nmemb, void* stream);
CURLcode httpPost(const std::string& url, const std::string& data, std::string& response, int timeout);
CURLcode httpPostFormData(const std::string& url, const std::string& data, std::string& response, int timeout);
CURLcode httpGet(const std::string& url, std::string& response, int timeout);
bool isRelease(tag_t object_tag);
string getReleaseName(tag_t object_tag);
bool checkSignRecord(tag_t object_tag);
int getHandlerArgs(TC_argument_list_t* arguments, map<string, string>& mp);
unsigned char ToHex(unsigned char x);
unsigned char FromHex(unsigned char x);
std::string UrlEncode(const std::string& str);
std::string UrlDecode(const std::string& str);
boolean checkExistItem(tag_t folder_tag, tag_t item_tag, char* propName);
int updateProp(tag_t item, const char* propName, const char* propValue);
int batchUpdataProp(tag_t item, map<string, string> propMap);
string& clearHeadTailSpace(string& str);
string& clearAllSpace(string& str);
string multiply(string num1, string num2);
void createLogFile(char* FunctionName);
void writeLog(const char* format, ...);
string getTcCode(const char* utf8);
string getExternCode(const char* str);
string getPropValue(tag_t obj, const char* propName);
string getCurUser();
string getCurGroup();
char* createJsonFile(const char* FunctionName);
void appendJsonData(const char* fileName, const char* data);
wstring string2wstring(string str);
string wstring2string(wstring wstr);
std::string to_utf8(std::wstring& wide_string);
std::wstring from_utf8(std::string& string);
bool compareStrings(const std::string& str1, const std::string& str2);
std::string toUpperCase(const std::string& str);
std::string toLowCase(const std::string& str);
void unpackBomLine(tag_t bomLine);
bool is_element_in_vector(vector<string>& v, string str);
int element_index(vector<string>& v, string str);
string getElementStartWith(vector<string>& v, string str);
string getElementEndWith(vector<string>& v, string str);
vector<tag_t> getTagList(tag_t* list, int num);
string getCurProjectId(tag_t item);
vector<tag_t> queryItems(string item);
vector<tag_t> findItemsByAtt(vector<string> attNames, vector <string> attValue);
string getVersionRule(tag_t itemRev);
string getVersionByRule(tag_t item, string pattern);
int reviseRev(tag_t basedRev, string newRevId, tag_t* newRev);
int createItem(string itemId, string itemName, string itemType, string pattern, tag_t* newItem, tag_t* newItemRev);
void sortVectorByName(vector<string>& list);
int getDecimalPlaces(std::string num);
string add(string x, string y);
std::vector<std::vector<string>> groupVector(const std::vector<string>& vec, int groupSize);
void distinctValue(vector<string>& list);
string getErrorMsg(int ifail);
bool isNewRevision(tag_t itemRev);
void createBomView(tag_t& item, tag_t& item_rev);
#endif